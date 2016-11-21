// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_FUNCTIONS_H_
#define ICL_FUNCTIONS_H_

#include <map>
#include <string>
#include <vector>

#include "icl/string_piece.h"

namespace icl {

class Err;
class BlockNode;
class FunctionCallNode;
class Label;
class ListNode;
class ParseNode;
class Scope;
class Token;
class Value;

// -----------------------------------------------------------------------------

namespace functions {

// This type of function invocation has no block and evaluates its arguments
// itself rather than taking a pre-executed list. This allows us to implement
// certain built-in functions.
typedef Value (*SelfEvaluatingArgsFunction)(Scope* scope,
                                            const FunctionCallNode* function,
                                            const ListNode* args_list,
                                            Err* err);

// This type of function invocation takes a block node that it will execute.
typedef Value (*GenericBlockFunction)(Scope* scope,
                                      const FunctionCallNode* function,
                                      const std::vector<Value>& args,
                                      BlockNode* block,
                                      Err* err);

// This type of function takes a block, but does not need to control execution
// of it. The dispatch function will pre-execute the block and pass the
// resulting block_scope to the function.
typedef Value(*ExecutedBlockFunction)(const FunctionCallNode* function,
                                      const std::vector<Value>& args,
                                      Scope* block_scope,
                                      Err* err);

// This type of function does not take a block. It just has arguments.
typedef Value (*NoBlockFunction)(Scope* scope,
                                 const FunctionCallNode* function,
                                 const std::vector<Value>& args,
                                 Err* err);

extern const char kAssert[];
extern const char kAssert_HelpShort[];
extern const char kAssert_Help[];
Value RunAssert(Scope* scope,
                const FunctionCallNode* function,
                const std::vector<Value>& args,
                Err* err);

//FIXME
/*
extern const char kConfig[];
extern const char kConfig_HelpShort[];
extern const char kConfig_Help[];
Value RunConfig(const FunctionCallNode* function,
                const std::vector<Value>& args,
                Scope* block_scope,
                Err* err);

extern const char kDeclareArgs[];
extern const char kDeclareArgs_HelpShort[];
extern const char kDeclareArgs_Help[];
Value RunDeclareArgs(Scope* scope,
                     const FunctionCallNode* function,
                     const std::vector<Value>& args,
                     BlockNode* block,
                     Err* err);
*/

extern const char kDefined[];
extern const char kDefined_HelpShort[];
extern const char kDefined_Help[];
Value RunDefined(Scope* scope,
                 const FunctionCallNode* function,
                 const ListNode* args_list,
                 Err* err);

extern const char kForEach[];
extern const char kForEach_HelpShort[];
extern const char kForEach_Help[];
Value RunForEach(Scope* scope,
                 const FunctionCallNode* function,
                 const ListNode* args_list,
                 Err* err);

//FIXME delete probably
/*
extern const char kForwardVariablesFrom[];
extern const char kForwardVariablesFrom_HelpShort[];
extern const char kForwardVariablesFrom_Help[];
Value RunForwardVariablesFrom(Scope* scope,
                              const FunctionCallNode* function,
                              const ListNode* args_list,
                              Err* err);

extern const char kGetEnv[];
extern const char kGetEnv_HelpShort[];
extern const char kGetEnv_Help[];
Value RunGetEnv(Scope* scope,
                const FunctionCallNode* function,
                const std::vector<Value>& args,
                Err* err);

extern const char kGetPathInfo[];
extern const char kGetPathInfo_HelpShort[];
extern const char kGetPathInfo_Help[];
Value RunGetPathInfo(Scope* scope,
                     const FunctionCallNode* function,
                     const std::vector<Value>& args,
                     Err* err);

extern const char kImport[];
extern const char kImport_HelpShort[];
extern const char kImport_Help[];
Value RunImport(Scope* scope,
                const FunctionCallNode* function,
                const std::vector<Value>& args,
                Err* err);
*/

extern const char kPrint[];
extern const char kPrint_HelpShort[];
extern const char kPrint_Help[];
Value RunPrint(Scope* scope,
               const FunctionCallNode* function,
               const std::vector<Value>& args,
               Err* err);

//FIXME
/*
extern const char kProcessFileTemplate[];
extern const char kProcessFileTemplate_HelpShort[];
extern const char kProcessFileTemplate_Help[];
Value RunProcessFileTemplate(Scope* scope,
                             const FunctionCallNode* function,
                             const std::vector<Value>& args,
                             Err* err);

extern const char kReadFile[];
extern const char kReadFile_HelpShort[];
extern const char kReadFile_Help[];
Value RunReadFile(Scope* scope,
                  const FunctionCallNode* function,
                  const std::vector<Value>& args,
                  Err* err);

extern const char kRebasePath[];
extern const char kRebasePath_HelpShort[];
extern const char kRebasePath_Help[];
Value RunRebasePath(Scope* scope,
                    const FunctionCallNode* function,
                    const std::vector<Value>& args,
                    Err* err);

extern const char kSplitList[];
extern const char kSplitList_HelpShort[];
extern const char kSplitList_Help[];
Value RunSplitList(Scope* scope,
                   const FunctionCallNode* function,
                   const ListNode* args_list,
                   Err* err);

extern const char kTemplate[];
extern const char kTemplate_HelpShort[];
extern const char kTemplate_Help[];
Value RunTemplate(Scope* scope,
                  const FunctionCallNode* function,
                  const std::vector<Value>& args,
                  BlockNode* block,
                  Err* err);

extern const char kWriteFile[];
extern const char kWriteFile_HelpShort[];
extern const char kWriteFile_Help[];
Value RunWriteFile(Scope* scope,
                   const FunctionCallNode* function,
                   const std::vector<Value>& args,
                   Err* err);
*/

// -----------------------------------------------------------------------------

// One function record. Only one of the given runner types will be non-null
// which indicates the type of function it is.
struct FunctionInfo {
  FunctionInfo();
  FunctionInfo(SelfEvaluatingArgsFunction seaf,
               const char* in_help_short,
               const char* in_help);
  FunctionInfo(GenericBlockFunction gbf,
               const char* in_help_short,
               const char* in_help);
  FunctionInfo(ExecutedBlockFunction ebf,
               const char* in_help_short,
               const char* in_help);
  FunctionInfo(NoBlockFunction nbf,
               const char* in_help_short,
               const char* in_help);

  SelfEvaluatingArgsFunction self_evaluating_args_runner;
  GenericBlockFunction generic_block_runner;
  ExecutedBlockFunction executed_block_runner;
  NoBlockFunction no_block_runner;

  const char* help_short;
  const char* help;
};

typedef std::map<StringPiece, FunctionInfo> FunctionInfoMap;

// Returns the mapping of all built-in functions.
const FunctionInfoMap& GetFunctions();

// Runs the given function.
Value RunFunction(Scope* scope,
                  const FunctionCallNode* function,
                  const ListNode* args_list,
                  BlockNode* block,  // Optional.
                  Err* err);

}  // namespace functions

// Helper functions -----------------------------------------------------------

//FIXME delete maybe
/*
// Verifies that the current scope is not processing an import. If it is, it
// will set the error, blame the given parse node for it, and return false.
bool EnsureNotProcessingImport(const ParseNode* node,
                               const Scope* scope,
                               Err* err);

// Like EnsureNotProcessingImport but checks for running the build config.
bool EnsureNotProcessingBuildConfig(const ParseNode* node,
                                    const Scope* scope,
                                    Err* err);
*/

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

// Some tyesp of blocks can't be nested inside other ones. For such cases,
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
