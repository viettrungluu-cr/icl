// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/err.h"
#include "icl/function_impls.h"  // Where |RunForEach()| is declared.
#include "icl/parse_node_value_adapter.h"
#include "icl/parse_tree.h"
#include "icl/scope.h"

namespace icl {
namespace function_impls {

static Value RunForEach(Scope* scope,
                        const FunctionCallNode* function,
                        const ListNode* args_list,
                        Err* err) {
  const auto& args_vector = args_list->contents();
  if (args_vector.size() != 2) {
    *err = Err(function, "Wrong number of arguments to foreach().",
               "Expecting exactly two.");
    return Value();
  }

  // Extract the loop variable.
  const IdentifierNode* identifier = args_vector[0]->AsIdentifier();
  if (!identifier) {
    *err =
        Err(args_vector[0].get(), "Expected an identifier for the loop var.");
    return Value();
  }
  StringPiece loop_var(identifier->value().value());

  // Extract the list to iterate over.
  ParseNodeValueAdapter list_adapter;
  if (!list_adapter.InitForType(scope, args_vector[1].get(), Value::LIST, err))
    return Value();
  const std::vector<Value>& list = list_adapter.get().list_value();

  // Block to execute.
  const BlockNode* block = function->block();
  if (!block) {
    *err = Err(function, "Expected { after foreach.");
    return Value();
  }

  // If the loop variable was previously defined in this scope, save it so we
  // can put it back after the loop is done.
  const Value* old_loop_value_ptr = scope->GetValue(loop_var);
  Value old_loop_value;
  if (old_loop_value_ptr)
    old_loop_value = *old_loop_value_ptr;

  for (const auto& cur : list) {
    scope->SetValue(loop_var, cur, function);
    block->Execute(scope, err);
    if (err->has_error())
      return Value();
  }

  // Put back loop var.
  if (old_loop_value_ptr) {
    // Put back old value. Use the copy we made, rather than use the pointer,
    // which will probably point to the new value now in the scope.
    scope->SetValue(loop_var, std::move(old_loop_value),
                    old_loop_value.origin());
  } else {
    // Loop variable was undefined before loop, delete it.
    scope->RemoveIdentifier(loop_var);
  }

  return Value();
}

FunctionInfoMapEntry ForEachFn() {
  // TODO(C++14): Use std::make_unique.
  return {"foreach",
          std::unique_ptr<FunctionInfo>(new FunctionInfo(&RunForEach))};
}

}  // namespace function_impls
}  // namespace icl
