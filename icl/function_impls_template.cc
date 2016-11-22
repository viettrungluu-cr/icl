// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/function_impls.h"

//FIXME
//#include "icl/functions.h"
#include "icl/parse_tree.h"
#include "icl/scope.h"
#include "icl/template.h"
#include "icl/value.h"

namespace icl {
namespace function_impls {

Value RunTemplate(Scope* scope,
                  const FunctionCallNode* function,
                  const std::vector<Value>& args,
                  BlockNode* block,
                  Err* err) {
  // Of course you can have configs and targets in a template. But here, we're
  // not actually executing the block, only declaring it. Marking the template
  // declaration as non-nestable means that you can't put it inside a target,
  // for example.
  NonNestableBlock non_nestable(scope, function, "template");
  if (!non_nestable.Enter(err))
    return Value();

  // TODO(brettw) determine if the function is built-in and throw an error if
  // it is.
  if (args.size() != 1) {
    *err = Err(function->function(),
               "Need exactly one string arg to template.");
    return Value();
  }
  if (!args[0].VerifyTypeIs(Value::STRING, err))
    return Value();
  std::string template_name = args[0].string_value();

  const Template* existing_template = scope->GetTemplate(template_name);
  if (existing_template) {
    *err = Err(function, "Duplicate template definition.",
               "A template with this name was already defined.");
    err->AppendSubErr(Err(existing_template->GetDefinitionRange(),
                          "Previous definition."));
    return Value();
  }

  scope->AddTemplate(template_name, MakeRefCounted<Template>(scope, function));

  // The template object above created a closure around the variables in the
  // current scope. The template code will execute in that context when it's
  // invoked. But this means that any variables defined above that are used
  // by the template won't get marked used just by defining the template. The
  // result can be spurious unused variable errors.
  //
  // The "right" thing to do would be to walk the syntax tree inside the
  // template, find all identifier references, and mark those variables used.
  // This is annoying and error-prone to implement and takes extra time to run
  // for this narrow use case.
  //
  // Templates are most often defined in .gni files which don't get
  // used-variable checking anyway, and this case is annoying enough that the
  // incremental value of unused variable checking isn't worth the
  // alternatives. So all values in scope before this template definition are
  // exempted from unused variable checking.
  scope->MarkAllUsed();

  return Value();
}

}  // namespace function_impls
}  // namespace icl
