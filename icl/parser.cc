// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/parser.h"

#include <assert.h>

#include <utility>

#include "icl/function.h"
#include "icl/operators.h"
#include "icl/token.h"

namespace icl {

enum Precedence {
  PRECEDENCE_ASSIGNMENT = 1,  // Lowest precedence.
  PRECEDENCE_OR = 2,
  PRECEDENCE_AND = 3,
  PRECEDENCE_EQUALITY = 4,
  PRECEDENCE_RELATION = 5,
  PRECEDENCE_SUM = 6,
  PRECEDENCE_PREFIX = 7,
  PRECEDENCE_CALL = 8,
  PRECEDENCE_DOT = 9,         // Highest precedence.
};

// The top-level for blocks/ifs is recursive descent, the expression parser is
// a Pratt parser. The basic idea there is to have the precedences (and
// associativities) encoded relative to each other and only parse up until you
// hit something of that precedence. There's a dispatch table in expressions_
// at the top of parser.cc that describes how each token dispatches if it's
// seen as either a prefix or infix operator, and if it's infix, what its
// precedence is.
//
// Refs:
// - http://javascript.crockford.com/tdop/tdop.html
// - http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/

// Indexed by Token::Type.
ParserHelper Parser::expressions_[] = {
    {nullptr, nullptr, -1},                                   // INVALID
    {&Parser::Literal, nullptr, -1},                          // INTEGER
    {&Parser::Literal, nullptr, -1},                          // STRING
    {&Parser::Literal, nullptr, -1},                          // TRUE_TOKEN
    {&Parser::Literal, nullptr, -1},                          // FALSE_TOKEN
    {nullptr, &Parser::Assignment, PRECEDENCE_ASSIGNMENT},    // EQUAL
    {nullptr, &Parser::BinaryOperator, PRECEDENCE_SUM},       // PLUS
//FIXME why not both?
    {&Parser::Neg, &Parser::BinaryOperator, PRECEDENCE_SUM},  // MINUS
    {nullptr, &Parser::Assignment, PRECEDENCE_ASSIGNMENT},    // PLUS_EQUALS
    {nullptr, &Parser::Assignment, PRECEDENCE_ASSIGNMENT},    // MINUS_EQUALS
    {nullptr, &Parser::BinaryOperator, PRECEDENCE_EQUALITY},  // EQUAL_EQUAL
    {nullptr, &Parser::BinaryOperator, PRECEDENCE_EQUALITY},  // NOT_EQUAL
    {nullptr, &Parser::BinaryOperator, PRECEDENCE_RELATION},  // LESS_EQUAL
    {nullptr, &Parser::BinaryOperator, PRECEDENCE_RELATION},  // GREATER_EQUAL
    {nullptr, &Parser::BinaryOperator, PRECEDENCE_RELATION},  // LESS_THAN
    {nullptr, &Parser::BinaryOperator, PRECEDENCE_RELATION},  // GREATER_THAN
    {nullptr, &Parser::BinaryOperator, PRECEDENCE_AND},       // BOOLEAN_AND
    {nullptr, &Parser::BinaryOperator, PRECEDENCE_OR},        // BOOLEAN_OR
    {&Parser::Not, nullptr, -1},                              // BANG
    {nullptr, &Parser::DotOperator, PRECEDENCE_DOT},          // DOT
    {&Parser::Group, nullptr, -1},                            // LEFT_PAREN
    {nullptr, nullptr, -1},                                   // RIGHT_PAREN
    {&Parser::List, &Parser::Subscript, PRECEDENCE_CALL},     // LEFT_BRACKET
    {nullptr, nullptr, -1},                                   // RIGHT_BRACKET
    {&Parser::Block, nullptr, -1},                            // LEFT_BRACE
    {nullptr, nullptr, -1},                                   // RIGHT_BRACE
    {nullptr, nullptr, -1},                                   // IF
    {nullptr, nullptr, -1},                                   // ELSE
    {&Parser::Name, &Parser::IdentifierOrCall, PRECEDENCE_CALL},  // IDENTIFIER
    {nullptr, nullptr, -1},                                       // COMMA
    {nullptr, nullptr, -1},                // UNCLASSIFIED_COMMENT
    {nullptr, nullptr, -1},                // LINE_COMMENT
    {nullptr, nullptr, -1},                // SUFFIX_COMMENT
    {&Parser::BlockComment, nullptr, -1},  // BLOCK_COMMENT
};

Parser::Parser(const std::vector<Token>& tokens, Err* err)
    : invalid_token_(Location(), Token::INVALID, StringPiece()),
      err_(err),
      cur_(0) {
  for (const auto& token : tokens) {
    switch (token.type()) {
      case Token::LINE_COMMENT:
        line_comment_tokens_.push_back(token);
        break;
      case Token::SUFFIX_COMMENT:
        suffix_comment_tokens_.push_back(token);
        break;
      default:
        // Note that BLOCK_COMMENTs (top-level standalone comments) are passed
        // through the real parser.
        tokens_.push_back(token);
        break;
    }
  }
}

Parser::~Parser() = default;

// static
std::unique_ptr<ParseNode> Parser::Parse(const std::vector<Token>& tokens,
                                         Err* err) {
  Parser p(tokens, err);
  return p.ParseFile();
}

// static
std::unique_ptr<ParseNode> Parser::ParseExpression(
    const std::vector<Token>& tokens,
    Err* err) {
  Parser p(tokens, err);
  std::unique_ptr<ParseNode> expr = p.ParseExpression();
  if (!p.at_end() && !err->has_error()) {
    *err = Err(p.cur_token(), "Trailing garbage");
    return nullptr;
  }
  return expr;
}

// static
std::unique_ptr<ParseNode> Parser::ParseValue(const std::vector<Token>& tokens,
                                              Err* err) {
  for (const Token& token : tokens) {
    switch (token.type()) {
      case Token::INTEGER:
      case Token::STRING:
      case Token::TRUE_TOKEN:
      case Token::FALSE_TOKEN:
      case Token::LEFT_BRACKET:
      case Token::RIGHT_BRACKET:
      case Token::COMMA:
        continue;
      default:
        *err = Err(token, "Invalid token in literal value");
        return nullptr;
    }
  }

  return ParseExpression(tokens, err);
}

bool Parser::IsAssignment(const ParseNode* node) const {
  return node && node->AsBinaryOp() &&
         (node->AsBinaryOp()->op().type() == Token::EQUAL ||
          node->AsBinaryOp()->op().type() == Token::PLUS_EQUALS ||
          node->AsBinaryOp()->op().type() == Token::MINUS_EQUALS);
}

bool Parser::IsStatementBreak(Token::Type token_type) const {
  switch (token_type) {
    case Token::IDENTIFIER:
    case Token::LEFT_BRACE:
    case Token::RIGHT_BRACE:
    case Token::IF:
    case Token::ELSE:
      return true;
    default:
      return false;
  }
}

bool Parser::LookAhead(Token::Type type) {
  if (at_end())
    return false;
  return cur_token().type() == type;
}

bool Parser::Match(Token::Type type) {
  if (!LookAhead(type))
    return false;
  Consume();
  return true;
}

const Token& Parser::Consume(Token::Type type, const char* error_message) {
  Token::Type types[1] = { type };
  return Consume(types, 1, error_message);
}

const Token& Parser::Consume(Token::Type* types,
                             size_t num_types,
                             const char* error_message) {
  if (has_error()) {
    // Don't overwrite current error, but make progress through tokens so that
    // a loop that's expecting a particular token will still terminate.
    if (!at_end())
      cur_++;
    return invalid_token_;
  }
  if (at_end()) {
    const char kEOFMsg[] = "I hit EOF instead.";
    if (tokens_.empty())
      *err_ = Err(Location(), error_message, kEOFMsg);
    else
      *err_ = Err(tokens_[tokens_.size() - 1], error_message, kEOFMsg);
    return invalid_token_;
  }

  for (size_t i = 0; i < num_types; ++i) {
    if (cur_token().type() == types[i])
      return Consume();
  }
  *err_ = Err(cur_token(), error_message);
  return invalid_token_;
}

const Token& Parser::Consume() {
  return tokens_[cur_++];
}

std::unique_ptr<ParseNode> Parser::ParseExpression() {
  return ParseExpression(0);
}

std::unique_ptr<ParseNode> Parser::ParseExpression(int precedence) {
  if (at_end())
    return std::unique_ptr<ParseNode>();

  const Token& token = Consume();
  PrefixFunc prefix = expressions_[token.type()].prefix;

  if (prefix == nullptr) {
    *err_ = Err(token,
                std::string("Unexpected token '") + token.value().as_string() +
                    std::string("'"));
    return std::unique_ptr<ParseNode>();
  }

  std::unique_ptr<ParseNode> left = (this->*prefix)(token);
  if (has_error())
    return left;

  while (!at_end() && !IsStatementBreak(cur_token().type()) &&
         precedence <= expressions_[cur_token().type()].precedence) {
    const Token& next_token = Consume();
    InfixFunc infix = expressions_[next_token.type()].infix;
    if (infix == nullptr) {
      *err_ = Err(next_token, std::string("Unexpected token '") +
                                  next_token.value().as_string() +
                                  std::string("'"));
      return std::unique_ptr<ParseNode>();
    }
    left = (this->*infix)(std::move(left), next_token);
    if (has_error())
      return std::unique_ptr<ParseNode>();
  }

  return left;
}

std::unique_ptr<ParseNode> Parser::Block(const Token& token) {
  // This entrypoint into ParseBlock means it's part of an expression and we
  // always want the result.
  return ParseBlock(token, BlockNode::RETURNS_SCOPE);
}

std::unique_ptr<ParseNode> Parser::Literal(const Token& token) {
  // TODO(C++14): use std::make_unique.
  return std::unique_ptr<LiteralNode>(new LiteralNode(token));
}

std::unique_ptr<ParseNode> Parser::Name(const Token& token) {
  return IdentifierOrCall(std::unique_ptr<ParseNode>(), token);
}

std::unique_ptr<ParseNode> Parser::BlockComment(const Token& token) {
  std::unique_ptr<BlockCommentNode> comment(new BlockCommentNode());
  comment->set_comment(token);
  return std::move(comment);
}

std::unique_ptr<ParseNode> Parser::Group(const Token& token) {
  std::unique_ptr<ParseNode> expr = ParseExpression();
  if (has_error())
    return std::unique_ptr<ParseNode>();
  Consume(Token::RIGHT_PAREN, "Expected ')'");
  return expr;
}

std::unique_ptr<ParseNode> Parser::Not(const Token& token) {
  std::unique_ptr<ParseNode> expr = ParseExpression(PRECEDENCE_PREFIX + 1);
  if (has_error())
    return std::unique_ptr<ParseNode>();
  if (!expr) {
    if (!has_error())
      *err_ = Err(token, "Expected right-hand side for '!'.");
    return std::unique_ptr<ParseNode>();
  }
  std::unique_ptr<UnaryOpNode> unary_op(new UnaryOpNode);
  unary_op->set_op(token);
  unary_op->set_operand(std::move(expr));
  return std::move(unary_op);
}

std::unique_ptr<ParseNode> Parser::Neg(const Token& token) {
  std::unique_ptr<ParseNode> expr = ParseExpression(PRECEDENCE_PREFIX + 1);
  if (has_error())
    return std::unique_ptr<ParseNode>();
  if (!expr) {
    if (!has_error())
      *err_ = Err(token, "Expected right-hand side for '!'.");
    return std::unique_ptr<ParseNode>();
  }
  std::unique_ptr<UnaryOpNode> unary_op(new UnaryOpNode);
  unary_op->set_op(token);
  unary_op->set_operand(std::move(expr));
  return std::move(unary_op);
}

std::unique_ptr<ParseNode> Parser::List(const Token& node) {
  std::unique_ptr<ParseNode> list(ParseList(node, Token::RIGHT_BRACKET, true));
  if (!has_error() && !at_end())
    Consume(Token::RIGHT_BRACKET, "Expected ']'");
  return list;
}

std::unique_ptr<ParseNode> Parser::BinaryOperator(
    std::unique_ptr<ParseNode> left,
    const Token& token) {
  std::unique_ptr<ParseNode> right =
      ParseExpression(expressions_[token.type()].precedence + 1);
  if (!right) {
    if (!has_error()) {
      *err_ = Err(token, "Expected right-hand side for '" +
                             token.value().as_string() + "'");
    }
    return std::unique_ptr<ParseNode>();
  }
  std::unique_ptr<BinaryOpNode> binary_op(new BinaryOpNode);
  binary_op->set_op(token);
  binary_op->set_left(std::move(left));
  binary_op->set_right(std::move(right));
  return std::move(binary_op);
}

std::unique_ptr<ParseNode> Parser::IdentifierOrCall(
    std::unique_ptr<ParseNode> left,
    const Token& token) {
  std::unique_ptr<ListNode> list(new ListNode);
  list->set_begin_token(token);
  // TODO(C++14): Use std::make_unique.
  list->set_end(std::unique_ptr<EndNode>(new EndNode(token)));
  std::unique_ptr<BlockNode> block;
  bool has_arg = false;
  if (LookAhead(Token::LEFT_PAREN)) {
    const Token& start_token = Consume();
    // Parsing a function call.
    has_arg = true;
    if (Match(Token::RIGHT_PAREN)) {
      // Nothing, just an empty call.
    } else {
      list = ParseList(start_token, Token::RIGHT_PAREN, false);
      if (has_error())
        return std::unique_ptr<ParseNode>();
      Consume(Token::RIGHT_PAREN, "Expected ')' after call");
    }
    // Optionally with a scope.
    if (LookAhead(Token::LEFT_BRACE)) {
      block = ParseBlock(Consume(), BlockNode::DISCARDS_RESULT);
      if (has_error())
        return std::unique_ptr<ParseNode>();
    }
  }

  if (!left && !has_arg) {
    // Not a function call, just a standalone identifier.
    return std::unique_ptr<ParseNode>(new IdentifierNode(token));
  }
  std::unique_ptr<FunctionCallNode> func_call(new FunctionCallNode);
  func_call->set_function(token);
  func_call->set_args(std::move(list));
  if (block)
    func_call->set_block(std::move(block));
  return std::move(func_call);
}

std::unique_ptr<ParseNode> Parser::Assignment(std::unique_ptr<ParseNode> left,
                                              const Token& token) {
  if (left->AsIdentifier() == nullptr && left->AsAccessor() == nullptr) {
    *err_ = Err(left.get(),
        "The left-hand side of an assignment must be an identifier, "
        "scope access, or array access.");
    return std::unique_ptr<ParseNode>();
  }
  std::unique_ptr<ParseNode> value = ParseExpression(PRECEDENCE_ASSIGNMENT);
  if (!value) {
    if (!has_error())
      *err_ = Err(token, "Expected right-hand side for assignment.");
    return std::unique_ptr<ParseNode>();
  }
  std::unique_ptr<BinaryOpNode> assign(new BinaryOpNode);
  assign->set_op(token);
  assign->set_left(std::move(left));
  assign->set_right(std::move(value));
  return std::move(assign);
}

std::unique_ptr<ParseNode> Parser::Subscript(std::unique_ptr<ParseNode> left,
                                             const Token& token) {
  // TODO: Maybe support more complex expressions like a[0][0]. This would
  // require work on the evaluator too.
  if (left->AsIdentifier() == nullptr) {
    *err_ = Err(left.get(), "May only subscript identifiers.",
        "The thing on the left hand side of the [] must be an identifier\n"
        "and not an expression. If you need this, you'll have to assign the\n"
        "value to a temporary before subscripting. Sorry.");
    return std::unique_ptr<ParseNode>();
  }
  std::unique_ptr<ParseNode> value = ParseExpression();
  Consume(Token::RIGHT_BRACKET, "Expecting ']' after subscript.");
  std::unique_ptr<AccessorNode> accessor(new AccessorNode);
  accessor->set_base(left->AsIdentifier()->value());
  accessor->set_index(std::move(value));
  return std::move(accessor);
}

std::unique_ptr<ParseNode> Parser::DotOperator(std::unique_ptr<ParseNode> left,
                                               const Token& token) {
  if (left->AsIdentifier() == nullptr) {
    *err_ = Err(left.get(), "May only use \".\" for identifiers.",
        "The thing on the left hand side of the dot must be an identifier\n"
        "and not an expression. If you need this, you'll have to assign the\n"
        "value to a temporary first. Sorry.");
    return std::unique_ptr<ParseNode>();
  }

  std::unique_ptr<ParseNode> right = ParseExpression(PRECEDENCE_DOT);
  if (!right || !right->AsIdentifier()) {
    *err_ = Err(token, "Expected identifier for right-hand-side of \".\"",
        "Good: a.cookies\nBad: a.42\nLooks good but still bad: a.cookies()");
    return std::unique_ptr<ParseNode>();
  }

  std::unique_ptr<AccessorNode> accessor(new AccessorNode);
  accessor->set_base(left->AsIdentifier()->value());
  accessor->set_member(std::unique_ptr<IdentifierNode>(
      static_cast<IdentifierNode*>(right.release())));
  return std::move(accessor);
}

// Does not Consume the start or end token.
std::unique_ptr<ListNode> Parser::ParseList(const Token& start_token,
                                            Token::Type stop_before,
                                            bool allow_trailing_comma) {
  std::unique_ptr<ListNode> list(new ListNode);
  list->set_begin_token(start_token);
  bool just_got_comma = false;
  bool first_time = true;
  while (!LookAhead(stop_before)) {
    if (!first_time) {
      if (!just_got_comma) {
        // Require commas separate things in lists.
        *err_ = Err(cur_token(), "Expected comma between items.");
        return std::unique_ptr<ListNode>();
      }
    }
    first_time = false;

    // Why _OR? We're parsing things that are higher precedence than the ,
    // that separates the items of the list. , should appear lower than
    // boolean expressions (the lowest of which is OR), but above assignments.
    list->append_item(ParseExpression(PRECEDENCE_OR));
    if (has_error())
      return std::unique_ptr<ListNode>();
    if (at_end()) {
      *err_ =
          Err(tokens_[tokens_.size() - 1], "Unexpected end of file in list.");
      return std::unique_ptr<ListNode>();
    }
    if (list->contents().back()->AsBlockComment()) {
      // If there was a comment inside the list, we don't need a comma to the
      // next item, so pretend we got one, if we're expecting one.
      just_got_comma = allow_trailing_comma;
    } else {
      just_got_comma = Match(Token::COMMA);
    }
  }
  if (just_got_comma && !allow_trailing_comma) {
    *err_ = Err(cur_token(), "Trailing comma");
    return std::unique_ptr<ListNode>();
  }
  // TODO(C++14): Use std::make_unique.
  list->set_end(std::unique_ptr<EndNode>(new EndNode(cur_token())));
  return list;
}

std::unique_ptr<ParseNode> Parser::ParseFile() {
  std::unique_ptr<BlockNode> file(new BlockNode(BlockNode::DISCARDS_RESULT));
  for (;;) {
    if (at_end())
      break;
    std::unique_ptr<ParseNode> statement = ParseStatement();
    if (!statement)
      break;
    file->append_statement(std::move(statement));
  }
  if (!at_end() && !has_error())
    *err_ = Err(cur_token(), "Unexpected here, should be newline.");
  if (has_error())
    return std::unique_ptr<ParseNode>();

  // TODO(scottmg): If this is measurably expensive, it could be done only
  // when necessary (when reformatting, or during tests). Comments are
  // separate from the parse tree at this point, so downstream code can remain
  // ignorant of them.
  AssignComments(file.get());

  return std::move(file);
}

std::unique_ptr<ParseNode> Parser::ParseStatement() {
  if (LookAhead(Token::IF)) {
    return ParseCondition();
  } else if (LookAhead(Token::BLOCK_COMMENT)) {
    return BlockComment(Consume());
  } else {
    // TODO(scottmg): Is this too strict? Just drop all the testing if we want
    // to allow "pointless" expressions and return ParseExpression() directly.
    std::unique_ptr<ParseNode> stmt = ParseExpression();
    if (stmt) {
      if (stmt->AsFunctionCall() || IsAssignment(stmt.get()))
        return stmt;
    }
    if (!has_error()) {
      const Token& token = cur_or_last_token();
      *err_ = Err(token, "Expecting assignment or function call.");
    }
    return std::unique_ptr<ParseNode>();
  }
}

std::unique_ptr<BlockNode> Parser::ParseBlock(
    const Token& begin_brace,
    BlockNode::ResultMode result_mode) {
  if (has_error())
    return std::unique_ptr<BlockNode>();
  std::unique_ptr<BlockNode> block(new BlockNode(result_mode));
  block->set_begin_token(begin_brace);

  for (;;) {
    if (LookAhead(Token::RIGHT_BRACE)) {
      // TODO(C++14): Use std::make_unique.
      block->set_end(std::unique_ptr<EndNode>(new EndNode(Consume())));
      break;
    }

    std::unique_ptr<ParseNode> statement = ParseStatement();
    if (!statement)
      return std::unique_ptr<BlockNode>();
    block->append_statement(std::move(statement));
  }
  return block;
}

std::unique_ptr<ParseNode> Parser::ParseCondition() {
  std::unique_ptr<ConditionNode> condition(new ConditionNode);
  condition->set_if_token(Consume(Token::IF, "Expected 'if'"));
  Consume(Token::LEFT_PAREN, "Expected '(' after 'if'.");
  condition->set_condition(ParseExpression());
  if (IsAssignment(condition->condition()))
    *err_ = Err(condition->condition(), "Assignment not allowed in 'if'.");
  Consume(Token::RIGHT_PAREN, "Expected ')' after condition of 'if'.");
  condition->set_if_true(ParseBlock(
      Consume(Token::LEFT_BRACE, "Expected '{' to start 'if' block."),
      BlockNode::DISCARDS_RESULT));
  if (Match(Token::ELSE)) {
    if (LookAhead(Token::LEFT_BRACE)) {
      condition->set_if_false(ParseBlock(Consume(),
                                         BlockNode::DISCARDS_RESULT));
    } else if (LookAhead(Token::IF)) {
      condition->set_if_false(ParseStatement());
    } else {
      *err_ = Err(cur_or_last_token(), "Expected '{' or 'if' after 'else'.");
      return std::unique_ptr<ParseNode>();
    }
  }
  if (has_error())
    return std::unique_ptr<ParseNode>();
  return std::move(condition);
}

void Parser::TraverseOrder(const ParseNode* root,
                           std::vector<const ParseNode*>* pre,
                           std::vector<const ParseNode*>* post) {
  if (root) {
    pre->push_back(root);

    if (const AccessorNode* accessor = root->AsAccessor()) {
      TraverseOrder(accessor->index(), pre, post);
      TraverseOrder(accessor->member(), pre, post);
    } else if (const BinaryOpNode* binop = root->AsBinaryOp()) {
      TraverseOrder(binop->left(), pre, post);
      TraverseOrder(binop->right(), pre, post);
    } else if (const BlockNode* block = root->AsBlock()) {
      for (const auto& statement : block->statements())
        TraverseOrder(statement.get(), pre, post);
      TraverseOrder(block->End(), pre, post);
    } else if (const ConditionNode* condition = root->AsConditionNode()) {
      TraverseOrder(condition->condition(), pre, post);
      TraverseOrder(condition->if_true(), pre, post);
      TraverseOrder(condition->if_false(), pre, post);
    } else if (const FunctionCallNode* func_call = root->AsFunctionCall()) {
      TraverseOrder(func_call->args(), pre, post);
      TraverseOrder(func_call->block(), pre, post);
    } else if (root->AsIdentifier()) {
      // Nothing.
    } else if (const ListNode* list = root->AsList()) {
      for (const auto& node : list->contents())
        TraverseOrder(node.get(), pre, post);
      TraverseOrder(list->End(), pre, post);
    } else if (root->AsLiteral()) {
      // Nothing.
    } else if (const UnaryOpNode* unaryop = root->AsUnaryOp()) {
      TraverseOrder(unaryop->operand(), pre, post);
    } else if (root->AsBlockComment()) {
      // Nothing.
    } else if (root->AsEnd()) {
      // Nothing.
    } else {
      assert(false);
    }

    post->push_back(root);
  }
}

void Parser::AssignComments(ParseNode* file) {
  // Start by generating a pre- and post- order traversal of the tree so we
  // can determine what's before and after comments.
  std::vector<const ParseNode*> pre;
  std::vector<const ParseNode*> post;
  TraverseOrder(file, &pre, &post);

  // Assign line comments to syntax immediately following.
  int cur_comment = 0;
  for (auto* node : pre) {
    if (node->GetRange().is_null()) {
      assert(node == file);
      continue;
    }
    const Location& start = node->GetRange().begin();
    while (cur_comment < static_cast<int>(line_comment_tokens_.size())) {
      if (start.byte() >= line_comment_tokens_[cur_comment].location().byte()) {
        const_cast<ParseNode*>(node)->comments_mutable()->append_before(
            line_comment_tokens_[cur_comment]);
        ++cur_comment;
      } else {
        break;
      }
    }
  }

  // Remaining line comments go at end of file.
  for (; cur_comment < static_cast<int>(line_comment_tokens_.size());
       ++cur_comment)
    file->comments_mutable()->append_after(line_comment_tokens_[cur_comment]);

  // Assign suffix to syntax immediately before.
  cur_comment = static_cast<int>(suffix_comment_tokens_.size() - 1);
  for (std::vector<const ParseNode*>::const_reverse_iterator i = post.rbegin();
       i != post.rend();
       ++i) {
    // Don't assign suffix comments to the function, list, or block, but instead
    // to the last thing inside.
    if ((*i)->AsFunctionCall() || (*i)->AsList() || (*i)->AsBlock())
      continue;

    Location start = (*i)->GetRange().begin();
    Location end = (*i)->GetRange().end();

    // Don't assign suffix comments to something that starts on an earlier
    // line, so that in:
    //
    // sources = [ "a",
    //     "b" ] # comment
    //
    // it's attached to "b", not sources = [ ... ].
    if (start.line_number() != end.line_number())
      continue;

    while (cur_comment >= 0) {
      if (end.byte() <= suffix_comment_tokens_[cur_comment].location().byte()) {
        const_cast<ParseNode*>(*i)->comments_mutable()->append_suffix(
            suffix_comment_tokens_[cur_comment]);
        --cur_comment;
      } else {
        break;
      }
    }

    // Suffix comments were assigned in reverse, so if there were multiple on
    // the same node, they need to be reversed.
    if ((*i)->comments() && !(*i)->comments()->suffix().empty())
      const_cast<ParseNode*>(*i)->comments_mutable()->ReverseSuffix();
  }
}

}  // namespace icl
