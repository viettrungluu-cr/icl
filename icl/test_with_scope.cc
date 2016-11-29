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

TestWithScope::~TestWithScope() {
}

void TestWithScope::Print(const std::string& s) {
  print_output_.append(s);
}

bool TestWithScope::GetInputFile(const SourceFile& name,
                                 InputFile** file,
                                 Err* err) {
  assert(false);  // Not implemented!
  return false;
}

bool TestWithScope::LoadFile(const SourceFile& name, std::string* contents) {
  assert(false);  // Not implemented!
  return false;
}

const FunctionMap& TestWithScope::GetFunctions() const {
  return functions_;
}

TestParseInput::TestParseInput(std::string&& input)
    : input_file_(SourceFile("//test")) {
  input_file_.SetContents(std::move(input));

  tokens_ = Tokenizer::Tokenize(&input_file_, &parse_err_);
  if (!parse_err_.has_error())
    parsed_ = Parser::Parse(tokens_, &parse_err_);
}

TestParseInput::~TestParseInput() {
}

/*
TestTarget::TestTarget(const TestWithScope& setup,
                       const std::string& label_string,
                       Target::OutputType type)
    : Target(setup.settings(), setup.ParseLabel(label_string)) {
  visibility().SetPublic();
  set_output_type(type);
}

TestTarget::~TestTarget() {
}
*/

}  // namespace icl
