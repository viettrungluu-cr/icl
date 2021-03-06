// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/test_with_scope.h"

#include <utility>

#include "icl/function_impls.h"
#include "icl/parser.h"
#include "icl/source_file.h"
#include "icl/tokenizer.h"

namespace icl {

TestWithScope::TestWithScope()
    : functions_(icl::function_impls::GetStandardFunctions()),
      scope_(this) {
//FIXME
//      scope_progammatic_provider_(&scope_, true) {
  scope_.set_item_collector(&items_);
}

TestWithScope::~TestWithScope() = default;

const FunctionMap& TestWithScope::GetFunctions() const {
  return functions_;
}

ImportManager* TestWithScope::GetImportManager() {
  assert(false);  // Not implemented! Import statements not supported.
  return nullptr;
}

bool TestWithScope::GetInputFile(const LocationRange& origin,
                                 const SourceFile& name,
                                 const InputFile** file) {
  assert(false);  // Not implemented!
  *file = nullptr;
  return false;
}

void TestWithScope::Print(const std::string& s) {
  print_output_.append(s);
}

StringPiece TestWithScope::GetSourceRoot() const {
  return "/";
}

TestParseInput::TestParseInput(std::string&& input)
    : input_file_(SourceFile("//test")) {
  input_file_.SetContents(std::move(input));

  tokens_ = Tokenizer::Tokenize(&input_file_, &parse_err_);
  if (!parse_err_.has_error())
    parsed_ = Parser::Parse(tokens_, &parse_err_);
}

TestParseInput::~TestParseInput() = default;

}  // namespace icl
