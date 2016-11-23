// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_FUNCTIONS_H_
#define ICL_FUNCTIONS_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "icl/string_piece.h"

namespace icl {

class Err;
class BlockNode;
class FunctionCallNode;
class ListNode;
class ParseNode;
class Scope;
class Token;
class Value;

class Function {
 public:
  enum class Type {
    // Self-evaluating args functions evaluate their arguments themselves rather
    // than taking a pre-evaluated list of arguments. (They may or may not take
    // a block, which is up to them to execute.) These are typically used for
    // built-in functions.
    SELF_EVALUATING_ARGS_BLOCK,
    SELF_EVALUATING_ARGS_NO_BLOCK,
    // Generic block functions take an evaluated list of arguments and a block
    // which is up to them to execute.
    GENERIC_BLOCK,
    // Executed block functions take an evaluated list of arguments and a block
    // that is already executed (passed as a |Scope| for the block).
    EXECUTED_BLOCK,
    // Generic no-block functions just take an evaluated list of arguments and
    // no block.
    GENERIC_NO_BLOCK,
  };

  virtual ~Function() = default;

  Value Run(Scope* scope,
            const FunctionCallNode* function,
            const ListNode* args_list,
            BlockNode* block,  // Optional.
            Err* err);

  // The following methods should be implemented/overridden by subclasses. They
  // are left public (instead of being protected) to make testing easier.

  virtual Type GetType() const = 0;

  // Exactly one of the following should be overridden, depending on the |Type|
  // (as returned by |GetType()|).
  virtual Value SelfEvaluatingArgsBlockFn(Scope* scope,
                                          const FunctionCallNode* function,
                                          const ListNode* args_list,
                                          Err* err) const;
  virtual Value SelfEvaluatingArgsNoBlockFn(Scope* scope,
                                            const FunctionCallNode* function,
                                            const ListNode* args_list,
                                            Err* err) const;
  virtual Value GenericBlockFn(Scope* scope,
                               const FunctionCallNode* function,
                               const std::vector<Value>& args,
                               BlockNode* block,
                               Err* err) const;
  virtual Value ExecutedBlockFn(const FunctionCallNode* function,
                                const std::vector<Value>& args,
                                Scope* block_scope,
                                Err* err) const;
  virtual Value GenericNoBlockFn(Scope* scope,
                                 const FunctionCallNode* function,
                                 const std::vector<Value>& args,
                                 Err* err) const;

 protected:
  Function() = default;

  Function(const Function&) = delete;
  Function& operator=(const Function&) = delete;
};

//////////////////////////////////////////

// This type of function invocation has no block and evaluates its arguments
// itself rather than taking a pre-executed list. This allows us to implement
// certain built-in functions.
using SelfEvaluatingArgsFunction = Value (*)(Scope* scope,
                                             const FunctionCallNode* function,
                                             const ListNode* args_list,
                                             Err* err);

// This type of function invocation takes a block node that it will execute.
using GenericBlockFunction = Value (*)(Scope* scope,
                                       const FunctionCallNode* function,
                                       const std::vector<Value>& args,
                                       BlockNode* block,
                                       Err* err);

// This type of function takes a block, but does not need to control execution
// of it. The dispatch function will pre-execute the block and pass the
// resulting block_scope to the function.
using ExecutedBlockFunction = Value (*)(const FunctionCallNode* function,
                                        const std::vector<Value>& args,
                                        Scope* block_scope,
                                        Err* err);

// This type of function does not take a block. It just has arguments.
using NoBlockFunction = Value (*)(Scope* scope,
                                  const FunctionCallNode* function,
                                  const std::vector<Value>& args,
                                  Err* err);

// One function record. Only one of the given runner types will be non-null
// which indicates the type of function it is.
class FunctionInfo : public Function {
 public:
  // Allow implicit conversion, because we're bad people.
  FunctionInfo(SelfEvaluatingArgsFunction seaf)
      : self_evaluating_args_runner(seaf) {}
  FunctionInfo(GenericBlockFunction gbf) : generic_block_runner(gbf) {}
  FunctionInfo(ExecutedBlockFunction ebf) : executed_block_runner(ebf) {}
  FunctionInfo(NoBlockFunction nbf) : no_block_runner(nbf) {}
  ~FunctionInfo() override = default;

  Type GetType() const override;
  Value SelfEvaluatingArgsBlockFn(Scope* scope,
                                  const FunctionCallNode* function,
                                  const ListNode* args_list,
                                  Err* err) const override;
  Value SelfEvaluatingArgsNoBlockFn(Scope* scope,
                                    const FunctionCallNode* function,
                                    const ListNode* args_list,
                                    Err* err) const override;
  Value GenericBlockFn(Scope* scope,
                       const FunctionCallNode* function,
                       const std::vector<Value>& args,
                       BlockNode* block,
                       Err* err) const override;
  Value ExecutedBlockFn(const FunctionCallNode* function,
                        const std::vector<Value>& args,
                        Scope* block_scope,
                        Err* err) const override;
  Value GenericNoBlockFn(Scope* scope,
                         const FunctionCallNode* function,
                         const std::vector<Value>& args,
                         Err* err) const override;

 private:
  SelfEvaluatingArgsFunction self_evaluating_args_runner = nullptr;
  GenericBlockFunction generic_block_runner = nullptr;
  ExecutedBlockFunction executed_block_runner = nullptr;
  NoBlockFunction no_block_runner = nullptr;
};

using FunctionInfoMap = std::map<StringPiece, std::unique_ptr<Function>>;
using FunctionInfoMapEntry = FunctionInfoMap::value_type;

// Runs the given function.
Value RunFunction(Scope* scope,
                  const FunctionCallNode* function,
                  const ListNode* args_list,
                  BlockNode* block,  // Optional.
                  Err* err);

// Helper functions -----------------------------------------------------------

// Verifies that the current scope is not processing an import. If it is, it
// will set the error, blame the given parse node for it, and return false.
bool EnsureNotProcessingImport(const ParseNode* node,
                               const Scope* scope,
                               Err* err);

// Sets up the |block_scope| for executing a target (or something like it).
// The |scope| is the containing scope. It should have been already set as the
// parent for the |block_scope| when the |block_scope| was created.
//
// This will set up the target defaults and set the |target_name| variable in
// the block scope to the current target name, which is assumed to be the first
// argument to the function.
//
// On success, returns true. On failure, sets the error and returns false.
bool FillTargetBlockScope(const Scope* scope,
                          const FunctionCallNode* function,
                          const std::string& target_type,
                          const BlockNode* block,
                          const std::vector<Value>& args,
                          Scope* block_scope,
                          Err* err);

// Sets the given error to a message explaining that the function call requires
// a block.
void FillNeedsBlockError(const FunctionCallNode* function, Err* err);

// Validates that the given function call has one string argument. This is
// the most common function signature, so it saves space to have this helper.
// Returns false and sets the error on failure.
bool EnsureSingleStringArg(const FunctionCallNode* function,
                           const std::vector<Value>& args,
                           Err* err);

// Some types of blocks can't be nested inside other ones. For such cases,
// instantiate this object upon entering the block and Enter() will fail if
// there is already another non-nestable block on the stack.
class NonNestableBlock {
 public:
  // type_description is a string that will be used in error messages
  // describing the type of the block, for example, "template" or "config".
  NonNestableBlock(Scope* scope,
                   const FunctionCallNode* function,
                   const char* type_description);
  ~NonNestableBlock();

  bool Enter(Err* err);

 private:
  // Used as a void* key for the Scope to track our property. The actual value
  // is never used.
  static const int kKey;

  Scope* scope_;
  const FunctionCallNode* function_;
  const char* type_description_;

  // Set to true when the key is added to the scope so we don't try to
  // delete nonexistant keys which will cause assertions.
  bool key_added_;
};

}  // namespace icl

#endif  // ICL_FUNCTIONS_H_
