// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/function_impls.h"

#include <stddef.h>

#include <string>

#include "icl/delegate.h"
#include "icl/err.h"
#include "icl/import_manager.h"
#include "icl/parse_tree.h"
#include "icl/scope.h"
#include "icl/source_file.h"
#include "icl/value.h"

namespace icl {
namespace function_impls {

void AddStandardFunctions(FunctionMap* functions) {
  functions->insert(function_impls::AssertFn());
  functions->insert(function_impls::DefinedFn());
  functions->insert(function_impls::PrintFn());
  functions->insert(function_impls::ForEachFn());
  functions->insert(function_impls::TemplateFn());
}

FunctionMap GetStandardFunctions() {
  FunctionMap functions;
  AddStandardFunctions(&functions);
  return functions;
}

FunctionMap GetStandardFunctionsWithImport() {
  FunctionMap functions;
  AddStandardFunctions(&functions);
  functions.insert(function_impls::ImportFn());
  return functions;
}

// assert ----------------------------------------------------------------------

namespace {

class AssertImpl : public Function {
 public:
  AssertImpl() = default;
  ~AssertImpl() override = default;
  Type GetType() const override { return Type::GENERIC_NO_BLOCK; }
  Value GenericNoBlockFn(Scope* scope,
                         const FunctionCallNode* function,
                         const std::vector<Value>& args,
                         Err* err) const override {
    if (args.size() != 1 && args.size() != 2) {
      *err = Err(function->function(), "Wrong number of arguments.",
                 "assert() takes one or two argument, "
                 "were you expecting somethig else?");
    } else if (!args[0].VerifyTypeIs(Value::BOOLEAN, err)) {
      // Nothing to do here.
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
        // break if you do "assert(\nfoo && bar)" and we may show the second
        // line as the source, oh well. The way around this is to check to see
        // if the origin node is inside our function call block.
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
};

}  // namespace

FunctionMapEntry AssertFn() {
  // TODO(C++14): Use std::make_unique.
  return {"assert", std::unique_ptr<AssertImpl>(new AssertImpl())};
}

// defined ---------------------------------------------------------------------

namespace {

class DefinedImpl : public Function {
 public:
  DefinedImpl() = default;
  ~DefinedImpl() override = default;
  Type GetType() const override { return Type::SELF_EVALUATING_ARGS_NO_BLOCK; }
  Value SelfEvaluatingArgsNoBlockFn(Scope* scope,
                                    const FunctionCallNode* function,
                                    const ListNode* args_list,
                                    Err* err) const override {
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
};

}  // namespace

FunctionMapEntry DefinedFn() {
  // TODO(C++14): Use std::make_unique.
  return {"defined", std::unique_ptr<DefinedImpl>(new DefinedImpl())};
}

// import ----------------------------------------------------------------------

namespace {

class ImportImpl : public Function {
 public:
  ImportImpl() = default;
  ~ImportImpl() override = default;
  Type GetType() const override { return Type::GENERIC_NO_BLOCK; }
  Value GenericNoBlockFn(Scope* scope,
                         const FunctionCallNode* function,
                         const std::vector<Value>& args,
                         Err* err) const override {
    if (!EnsureSingleStringArg(function, args, err))
      return Value();

//FIXME
    SourceFile import_file((std::string(args[0].string_value())));
//    const SourceDir& input_dir = scope->GetSourceDir();
//    SourceFile import_file =
//        input_dir.ResolveRelativeFile(args[0], err,
//            scope->settings()->build_settings()->root_path_utf8());
    if (!err->has_error()) {
      scope->delegate()->GetImportManager()->DoImport(import_file, function,
                                                      scope, err);
    }
    return Value();
  }
};

}  // namespace

FunctionMapEntry ImportFn() {
  // TODO(C++14): Use std::make_unique.
  return {"import", std::unique_ptr<ImportImpl>(new ImportImpl())};
}

// print -----------------------------------------------------------------------

namespace {

class PrintImpl : public Function {
 public:
  PrintImpl() = default;
  ~PrintImpl() override = default;
  Type GetType() const override { return Type::GENERIC_NO_BLOCK; }
  Value GenericNoBlockFn(Scope* scope,
                         const FunctionCallNode* function,
                         const std::vector<Value>& args,
                         Err* err) const override {
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
};

}  // namespace

FunctionMapEntry PrintFn() {
  // TODO(C++14): Use std::make_unique.
  return {"print", std::unique_ptr<PrintImpl>(new PrintImpl())};
}

//FIXME
#if 0
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
#endif

}  // namespace function_impls
}  // namespace icl
