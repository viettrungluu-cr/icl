// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/functions.h"

//FIXME clean up includes

#include <stddef.h>

#include <string>
#include <utility>

#include "icl/function_impls.h"
#include "icl/err.h"
#include "icl/parse_tree.h"
#include "icl/scope.h"
#include "icl/token.h"
#include "icl/value.h"
/*
#include "base/environment.h"
#include "base/strings/string_util.h"
#include "tools/gn/config.h"
#include "tools/gn/config_values_generator.h"
#include "tools/gn/input_file.h"
#include "tools/gn/parse_node_value_adapter.h"
#include "tools/gn/pool.h"
#include "tools/gn/scheduler.h"
#include "tools/gn/template.h"
#include "tools/gn/value_extractors.h"
#include "tools/gn/variables.h"
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
      "statement or a target declaration.");
  err->AppendRange(function->function().range());
  return false;
}

}  // namespace

//FIXME
/*
bool EnsureNotProcessingImport(const ParseNode* node,
                               const Scope* scope,
                               Err* err) {
  if (scope->IsProcessingImport()) {
    *err = Err(node, "Not valid from an import.",
        "Imports are for defining defaults, variables, and rules. The\n"
        "appropriate place for this kind of thing is really in a normal\n"
        "BUILD file.");
    return false;
  }
  return true;
}
*/

//FIXME
/*
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

  // Set the target name variable to the current target, and mark it used
  // because we don't want to issue an error if the script ignores it.
  const StringPiece target_name(variables::kTargetName);
  block_scope->SetValue(target_name, Value(function, args[0].string_value()),
                        function);
  block_scope->MarkUsed(target_name);
  return true;
}
*/

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

// Setup the function map via a static initializer. We use this because it
// avoids race conditions without having to do some global setup function or
// locking-heavy singleton checks at runtime. In practice, we always need this
// before we can do anything interesting, so it's OK to wait for the
// initializer.
struct FunctionInfoInitializer {
  FunctionInfoMap map = {function_impls::AssertFn(),
                         function_impls::DefinedFn(),
                         function_impls::ForEachFn(),
                         function_impls::PrintFn()};
//FIXME
/*
    INSERT_FUNCTION(Config)
    INSERT_FUNCTION(DeclareArgs)
    INSERT_FUNCTION(ForwardVariablesFrom)
    INSERT_FUNCTION(GetEnv)
    INSERT_FUNCTION(GetPathInfo)
    INSERT_FUNCTION(Import)
    INSERT_FUNCTION(ProcessFileTemplate)
    INSERT_FUNCTION(ReadFile)
    INSERT_FUNCTION(RebasePath)
    INSERT_FUNCTION(SplitList)
    INSERT_FUNCTION(Template)
    INSERT_FUNCTION(WriteFile)
*/
};
const FunctionInfoInitializer function_info;

const FunctionInfoMap& GetFunctions() {
  return function_info.map;
}

Value RunFunction(Scope* scope,
                  const FunctionCallNode* function,
                  const ListNode* args_list,
                  BlockNode* block,
                  Err* err) {
  const Token& name = function->function();

  const FunctionInfoMap& function_map = GetFunctions();
  FunctionInfoMap::const_iterator found_function =
      function_map.find(name.value());
  if (found_function == function_map.end()) {
    // No built-in function matching this, check for a template.
//FIXME
/*
    std::string template_name = function->function().value().as_string();
    const Template* templ = scope->GetTemplate(template_name);
    if (templ) {
      Value args = args_list->Execute(scope, err);
      if (err->has_error())
        return Value();
      return templ->Invoke(scope, function, template_name, args.list_value(),
                           block, err);
    }
*/

    *err = Err(name, "Unknown function.");
    return Value();
  }

  if (found_function->second.self_evaluating_args_runner) {
    // Self evaluating args functions are special weird built-ins like foreach.
    // Rather than force them all to check that they have a block or no block
    // and risk bugs for new additions, check a whitelist here.
//FIXME remove this special-casing
    if (found_function->second.self_evaluating_args_runner !=
            &function_impls::RunForEach) {
      if (!VerifyNoBlockForFunctionCall(function, block, err))
        return Value();
    }
    return found_function->second.self_evaluating_args_runner(
        scope, function, args_list, err);
  }

  // All other function types take a pre-executed set of args.
  Value args = args_list->Execute(scope, err);
  if (err->has_error())
    return Value();

  if (found_function->second.generic_block_runner) {
    if (!block) {
      FillNeedsBlockError(function, err);
      return Value();
    }
    return found_function->second.generic_block_runner(
        scope, function, args.list_value(), block, err);
  }

  if (found_function->second.executed_block_runner) {
    if (!block) {
      FillNeedsBlockError(function, err);
      return Value();
    }

    Scope block_scope(scope);
    block->Execute(&block_scope, err);
    if (err->has_error())
      return Value();

    Value result = found_function->second.executed_block_runner(
        function, args.list_value(), &block_scope, err);
    if (err->has_error())
      return Value();

    if (!block_scope.CheckForUnusedVars(err))
      return Value();
    return result;
  }

  // Otherwise it's a no-block function.
  if (!VerifyNoBlockForFunctionCall(function, block, err))
    return Value();
  return found_function->second.no_block_runner(scope, function,
                                                args.list_value(), err);
}

}  // namespace icl
