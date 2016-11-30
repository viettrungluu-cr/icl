// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/function.h"

//FIXME clean up includes

#include <stddef.h>

#include <string>
#include <utility>

#include "icl/delegate.h"
#include "icl/err.h"
#include "icl/parse_tree.h"
#include "icl/scope.h"
#include "icl/template.h"
#include "icl/token.h"
#include "icl/value.h"
#include "icl/variables.h"
/*
#include "tools/gn/parse_node_value_adapter.h"
#include "tools/gn/value_extractors.h"
*/

namespace icl {

namespace {

// Some functions take a {} following them, and some don't. For the ones that
// don't, this is used to verify that the given block node is null and will
// set the error accordingly if it's not. Returns true if the block is null.
bool VerifyNoBlockForFunctionCall(const FunctionCallNode* function,
                                  const BlockNode* block,
                                  Err* err) {
  if (!block)
    return true;

  *err = Err(block, "Unexpected '{'.",
      "This function call doesn't take a {} block following it, and you\n"
      "can't have a {} block that's not connected to something like an if\n"
      "statement or an item declaration.");
  err->AppendRange(function->function().range());
  return false;
}

}  // namespace

Value Function::Run(Scope* scope,
                    const FunctionCallNode* function,
                    const ListNode* args_list,
                    BlockNode* block,  // Optional.
                    Err* err) {
  auto type = GetType();

  if (type == Type::SELF_EVALUATING_ARGS_BLOCK)
    return SelfEvaluatingArgsBlockFn(scope, function, args_list, err);

  if (type == Type::SELF_EVALUATING_ARGS_NO_BLOCK) {
    if (!VerifyNoBlockForFunctionCall(function, block, err))
      return Value();
    return SelfEvaluatingArgsNoBlockFn(scope, function, args_list, err);
  }

  // All other function types take a pre-executed set of args.
  Value args = args_list->Execute(scope, err);
  if (err->has_error())
    return Value();

  if (type == Type::GENERIC_BLOCK) {
    if (!block) {
      FillNeedsBlockError(function, err);
      return Value();
    }
    return GenericBlockFn(scope, function, args.list_value(), block, err);
  }

  if (type == Type::EXECUTED_BLOCK) {
    if (!block) {
      FillNeedsBlockError(function, err);
      return Value();
    }

    Scope block_scope(scope);
    block->Execute(&block_scope, err);
    if (err->has_error())
      return Value();

    Value result =
        ExecutedBlockFn(function, args.list_value(), &block_scope, err);
    if (err->has_error())
      return Value();

    if (!block_scope.CheckForUnusedVars(err))
      return Value();
    return result;
  }

  // Otherwise it's a no-block function.
  assert(type == Type::GENERIC_NO_BLOCK);
  if (!VerifyNoBlockForFunctionCall(function, block, err))
    return Value();
  return GenericNoBlockFn(scope, function, args.list_value(), err);
}

Value Function::SelfEvaluatingArgsBlockFn(Scope* scope,
                                          const FunctionCallNode* function,
                                          const ListNode* args_list,
                                          Err* err) const {
  assert(false);
  return Value();
}

Value Function::SelfEvaluatingArgsNoBlockFn(Scope* scope,
                                            const FunctionCallNode* function,
                                            const ListNode* args_list,
                                            Err* err) const {
  assert(false);
  return Value();
}

Value Function::GenericBlockFn(Scope* scope,
                               const FunctionCallNode* function,
                               const std::vector<Value>& args,
                               BlockNode* block,
                               Err* err) const {
  assert(false);
  return Value();
}

Value Function::ExecutedBlockFn(const FunctionCallNode* function,
                                const std::vector<Value>& args,
                                Scope* block_scope,
                                Err* err) const {
  assert(false);
  return Value();
}

Value Function::GenericNoBlockFn(Scope* scope,
                                 const FunctionCallNode* function,
                                 const std::vector<Value>& args,
                                 Err* err) const {
  assert(false);
  return Value();
}

Value RunFunction(Scope* scope,
                  const FunctionCallNode* function,
                  const ListNode* args_list,
                  BlockNode* block,
                  Err* err) {
  const Token& name = function->function();

  const FunctionMap& function_map = scope->delegate()->GetFunctions();
  FunctionMap::const_iterator found_function = function_map.find(name.value());
  if (found_function == function_map.end()) {
    // No built-in function matching this, check for a template.
    std::string template_name = function->function().value().as_string();
    const Template* templ = scope->GetTemplate(template_name);
    if (templ) {
      Value args = args_list->Execute(scope, err);
      if (err->has_error())
        return Value();
      return templ->Invoke(scope, function, template_name, args.list_value(),
                           block, err);
    }

    *err = Err(name, "Unknown function.");
    return Value();
  }

  return found_function->second->Run(scope, function, args_list, block, err);
}

// Helper functions ------------------------------------------------------------

bool EnsureNotProcessingImport(const ParseNode* node,
                               const Scope* scope,
                               Err* err) {
  if (scope->IsProcessingImport()) {
    *err = Err(node, "Not valid from an import.");
    return false;
  }
  return true;
}

bool FillTargetBlockScope(const Scope* scope,
                          const FunctionCallNode* function,
                          const std::string& target_type,
                          const BlockNode* block,
                          const std::vector<Value>& args,
                          Scope* block_scope,
                          Err* err) {
  if (!block) {
    FillNeedsBlockError(function, err);
    return false;
  }

  // Copy the target defaults, if any, into the scope we're going to execute
  // the block in.
  const Scope* default_scope = scope->GetTargetDefaults(target_type);
  if (default_scope) {
    Scope::MergeOptions merge_options;
    merge_options.skip_private_vars = true;
    if (!default_scope->NonRecursiveMergeTo(block_scope, merge_options,
                                            function, "target defaults", err))
      return false;
  }

  // The name is the single argument to the target function.
  if (!EnsureSingleStringArg(function, args, err))
    return false;

  // Set the item name variable to the current item, and mark it used because we
  // don't want to issue an error if the script ignores it.
  const StringPiece item_name(variables::kItemName);
  block_scope->SetValue(item_name, Value(function, args[0].string_value()),
                        function);
  block_scope->MarkUsed(item_name);
  return true;
}

void FillNeedsBlockError(const FunctionCallNode* function, Err* err) {
  *err = Err(function->function(), "This function call requires a block.",
      "The block's \"{\" must be on the same line as the function "
      "call's \")\".");
}

bool EnsureSingleStringArg(const FunctionCallNode* function,
                           const std::vector<Value>& args,
                           Err* err) {
  if (args.size() != 1) {
    *err = Err(function->function(), "Incorrect arguments.",
               "This function requires a single string argument.");
    return false;
  }
  return args[0].VerifyTypeIs(Value::STRING, err);
}

// static
const int NonNestableBlock::kKey = 0;

NonNestableBlock::NonNestableBlock(
    Scope* scope,
    const FunctionCallNode* function,
    const char* type_description)
    : scope_(scope),
      function_(function),
      type_description_(type_description),
      key_added_(false) {
}

NonNestableBlock::~NonNestableBlock() {
  if (key_added_)
    scope_->SetProperty(&kKey, nullptr);
}

bool NonNestableBlock::Enter(Err* err) {
  void* scope_value = scope_->GetProperty(&kKey, nullptr);
  if (scope_value) {
    // Existing block.
    const NonNestableBlock* existing =
        reinterpret_cast<const NonNestableBlock*>(scope_value);
    *err = Err(function_, "Can't nest these things.",
        std::string("You are trying to nest a ") + type_description_ +
        " inside a " + existing->type_description_ + ".");
    err->AppendSubErr(Err(existing->function_, "The enclosing block."));
    return false;
  }

  scope_->SetProperty(&kKey, this);
  key_added_ = true;
  return true;
}

}  // namespace icl
