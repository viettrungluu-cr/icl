// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <stdlib.h>

#include <iostream>
#include <memory>
#include <vector>

#include "icl/delegate.h"
#include "icl/err.h"
#include "icl/function_impls.h"
#include "icl/input_file.h"
#include "icl/parse_tree.h"
#include "icl/parser.h"
#include "icl/scope.h"
#include "icl/source_file.h"
#include "icl/token.h"
#include "icl/tokenizer.h"

namespace {

void PanicOnError(const icl::Err& err, const char* msg) {
  if (err.has_error()) {
    std::cerr << "PANIC: " << msg << std::endl << err.GetErrorMessage();
    abort();
  }
}

class DelegateImpl : public icl::Delegate {
 public:
  DelegateImpl() = default;
  ~DelegateImpl() = default;

  DelegateImpl(const DelegateImpl&) = delete;
  DelegateImpl& operator=(const DelegateImpl&) = delete;

  // |icl::Delegate| methods:
  bool LoadFile(const icl::SourceFile& name, std::string* contents) override {
    assert(name.value() == "//noname");
    *contents = "a = \"hello world\"\n"
                "print(a)";
    return true;
  }
  void Print(const std::string& s) override {
    std::cout << s;
  }
  const icl::FunctionInfoMap& GetFunctions() const override {
    return functions_;
  }

 private:
  static const icl::FunctionInfoMap functions_;
};

// static
const icl::FunctionInfoMap DelegateImpl::functions_ =
    {icl::function_impls::AssertFn(), icl::function_impls::DefinedFn(),
     icl::function_impls::PrintFn(), icl::function_impls::ForEachFn(),
     icl::function_impls::TemplateFn()};

}  // namespace

int main() {
  icl::InputFile input(icl::SourceFile("//noname"));
  input.SetContents("a = \"hello world\"\n"
                    "print(a)");

  icl::Err err;
  std::vector<icl::Token> tokens = icl::Tokenizer::Tokenize(&input, &err);
  PanicOnError(err, "tokenization failed");

  std::unique_ptr<icl::ParseNode> ast = icl::Parser::Parse(tokens, &err);
  PanicOnError(err, "parse failed");

  DelegateImpl delegate;
  icl::Scope scope(&delegate);
  ast->Execute(&scope, &err);
  PanicOnError(err, "execute failed");

  return 0;
}
