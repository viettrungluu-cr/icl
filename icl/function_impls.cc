// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/function_impls.h"

//FIXME clean up includes

#include <stddef.h>

#include <string>
#include <utility>

#include "icl/delegate.h"
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
namespace function_impls {

// assert ----------------------------------------------------------------------

Value RunAssert(Scope* scope,
                const FunctionCallNode* function,
                const std::vector<Value>& args,
                Err* err) {
  if (args.size() != 1 && args.size() != 2) {
    *err = Err(function->function(), "Wrong number of arguments.",
               "assert() takes one or two argument, "
               "were you expecting somethig else?");
  } else if (args[0].type() != Value::BOOLEAN) {
    *err = Err(function->function(), "Assertion value not a bool.");
  } else if (!args[0].boolean_value()) {
    if (args.size() == 2) {
      // Optional string message.
      if (args[1].type() != Value::STRING) {
        *err = Err(function->function(), "Assertion failed.",
            "<<<ERROR MESSAGE IS NOT A STRING>>>");
      } else {
        *err = Err(function->function(), "Assertion failed.",
            args[1].string_value());
      }
    } else {
      *err = Err(function->function(), "Assertion failed.");
    }

    if (args[0].origin()) {
      // If you do "assert(foo)" we'd ideally like to show you where foo was
      // set, and in this case the origin of the args will tell us that.
      // However, if you do "assert(foo && bar)" the source of the value will
      // be the assert like, which isn't so helpful.
      //
      // So we try to see if the args are from the same line or not. This will
      // break if you do "assert(\nfoo && bar)" and we may show the second line
      // as the source, oh well. The way around this is to check to see if the
      // origin node is inside our function call block.
      Location origin_location = args[0].origin()->GetRange().begin();
      if (origin_location.file() != function->function().location().file() ||
          origin_location.line_number() !=
              function->function().location().line_number()) {
        err->AppendSubErr(Err(args[0].origin()->GetRange(), "",
                              "This is where it was set."));
      }
    }
  }
  return Value();
}

// defined ---------------------------------------------------------------------

Value RunDefined(Scope* scope,
                 const FunctionCallNode* function,
                 const ListNode* args_list,
                 Err* err) {
  const auto& args_vector = args_list->contents();
  if (args_vector.size() != 1) {
    *err = Err(function, "Wrong number of arguments to defined().",
               "Expecting exactly one.");
    return Value();
  }

  const IdentifierNode* identifier = args_vector[0]->AsIdentifier();
  if (identifier) {
    // Passed an identifier "defined(foo)".
    if (scope->GetValue(identifier->value().value()))
      return Value(function, true);
    return Value(function, false);
  }

  const AccessorNode* accessor = args_vector[0]->AsAccessor();
  if (accessor) {
    // Passed an accessor "defined(foo.bar)".
    if (accessor->member()) {
      // The base of the accessor must be a scope if it's defined.
      const Value* base = scope->GetValue(accessor->base().value());
      if (!base) {
        *err = Err(accessor, "Undefined identifier");
        return Value();
      }
      if (!base->VerifyTypeIs(Value::SCOPE, err))
        return Value();

      // Check the member inside the scope to see if its defined.
      if (base->scope_value()->GetValue(accessor->member()->value().value()))
        return Value(function, true);
      return Value(function, false);
    }
  }

  // Argument is invalid.
  *err = Err(function, "Bad thing passed to defined().",
      "It should be of the form defined(foo) or defined(foo.bar).");
  return Value();
}

// print -----------------------------------------------------------------------

Value RunPrint(Scope* scope,
               const FunctionCallNode* function,
               const std::vector<Value>& args,
               Err* err) {
  std::string output;
  for (size_t i = 0; i < args.size(); i++) {
    if (i != 0)
      output.push_back(' ');
    output.append(args[i].ToString(false));
  }
  output.push_back('\n');

  scope->delegate()->Print(output);

  return Value();
}

//FIXME
#if 0
// config ----------------------------------------------------------------------

const char kConfig[] = "config";

Value RunConfig(const FunctionCallNode* function,
                const std::vector<Value>& args,
                Scope* scope,
                Err* err) {
  NonNestableBlock non_nestable(scope, function, "config");
  if (!non_nestable.Enter(err))
    return Value();

  if (!EnsureSingleStringArg(function, args, err) ||
      !EnsureNotProcessingImport(function, scope, err))
    return Value();

  Label label(MakeLabelForScope(scope, function, args[0].string_value()));

  if (g_scheduler->verbose_logging())
    g_scheduler->Log("Defining config", label.GetUserVisibleName(true));

  // Create the new config.
  std::unique_ptr<Config> config(new Config(scope->settings(), label));
  config->set_defined_from(function);
  if (!Visibility::FillItemVisibility(config.get(), scope, err))
    return Value();

  // Fill the flags and such.
  const SourceDir& input_dir = scope->GetSourceDir();
  ConfigValuesGenerator gen(&config->own_values(), scope, input_dir, err);
  gen.Run();
  if (err->has_error())
    return Value();

  // Read sub-configs.
  const Value* configs_value = scope->GetValue(variables::kConfigs, true);
  if (configs_value) {
    ExtractListOfUniqueLabels(*configs_value, scope->GetSourceDir(),
                              ToolchainLabelForScope(scope),
                              &config->configs(), err);
  }
  if (err->has_error())
    return Value();

  // Save the generated item.
  Scope::ItemVector* collector = scope->GetItemCollector();
  if (!collector) {
    *err = Err(function, "Can't define a config in this context.");
    return Value();
  }
  collector->push_back(config.release());

  return Value();
}

// declare_args ----------------------------------------------------------------

const char kDeclareArgs[] = "declare_args";

Value RunDeclareArgs(Scope* scope,
                     const FunctionCallNode* function,
                     const std::vector<Value>& args,
                     BlockNode* block,
                     Err* err) {
  NonNestableBlock non_nestable(scope, function, "declare_args");
  if (!non_nestable.Enter(err))
    return Value();

  Scope block_scope(scope);
  block->Execute(&block_scope, err);
  if (err->has_error())
    return Value();

  // Pass the values from our scope into the Args object for adding to the
  // scope with the proper values (taking into account the defaults given in
  // the block_scope, and arguments passed into the build).
  Scope::KeyValueMap values;
  block_scope.GetCurrentScopeValues(&values);
  scope->settings()->build_settings()->build_args().DeclareArgs(
      values, scope, err);
  return Value();
}

// getenv ----------------------------------------------------------------------

const char kGetEnv[] = "getenv";

Value RunGetEnv(Scope* scope,
                const FunctionCallNode* function,
                const std::vector<Value>& args,
                Err* err) {
  if (!EnsureSingleStringArg(function, args, err))
    return Value();

  std::unique_ptr<base::Environment> env(base::Environment::Create());

  std::string result;
  if (!env->GetVar(args[0].string_value().c_str(), &result))
    return Value(function, "");  // Not found, return empty string.
  return Value(function, result);
}

// import ----------------------------------------------------------------------

const char kImport[] = "import";

Value RunImport(Scope* scope,
                const FunctionCallNode* function,
                const std::vector<Value>& args,
                Err* err) {
  if (!EnsureSingleStringArg(function, args, err))
    return Value();

  const SourceDir& input_dir = scope->GetSourceDir();
  SourceFile import_file =
      input_dir.ResolveRelativeFile(args[0], err,
          scope->settings()->build_settings()->root_path_utf8());
  if (!err->has_error()) {
    scope->settings()->import_manager().DoImport(import_file, function,
                                                 scope, err);
  }
  return Value();
}

// split_list ------------------------------------------------------------------

const char kSplitList[] = "split_list";

Value RunSplitList(Scope* scope,
                   const FunctionCallNode* function,
                   const ListNode* args_list,
                   Err* err) {
  const auto& args_vector = args_list->contents();
  if (args_vector.size() != 2) {
    *err = Err(function, "Wrong number of arguments to split_list().",
               "Expecting exactly two.");
    return Value();
  }

  ParseNodeValueAdapter list_adapter;
  if (!list_adapter.InitForType(scope, args_vector[0].get(), Value::LIST, err))
    return Value();
  const std::vector<Value>& input = list_adapter.get().list_value();

  ParseNodeValueAdapter count_adapter;
  if (!count_adapter.InitForType(scope, args_vector[1].get(), Value::INTEGER,
                                 err))
    return Value();
  int64_t count = count_adapter.get().int_value();
  if (count <= 0) {
    *err = Err(function, "Requested result size is not positive.");
    return Value();
  }

  Value result(function, Value::LIST);
  result.list_value().resize(count);

  // Every result list gets at least this many items in it.
  int64_t min_items_per_list = static_cast<int64_t>(input.size()) / count;

  // This many result lists get an extra item which is the remainder from above.
  int64_t extra_items = static_cast<int64_t>(input.size()) % count;

  // Allocate all lists that have a remainder assigned to them (max items).
  int64_t max_items_per_list = min_items_per_list + 1;
  auto last_item_end = input.begin();
  for (int64_t i = 0; i < extra_items; i++) {
    result.list_value()[i] = Value(function, Value::LIST);

    auto begin_add = last_item_end;
    last_item_end += max_items_per_list;
    result.list_value()[i].list_value().assign(begin_add, last_item_end);
  }

  // Allocate all smaller items that don't have a remainder.
  for (int64_t i = extra_items; i < count; i++) {
    result.list_value()[i] = Value(function, Value::LIST);

    auto begin_add = last_item_end;
    last_item_end += min_items_per_list;
    result.list_value()[i].list_value().assign(begin_add, last_item_end);
  }

  return result;
}
#endif

}  // namespace function_impls
}  // namespace icl
