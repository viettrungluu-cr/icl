// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_TEST_WITH_SCOPE_H_
#define ICL_TEST_WITH_SCOPE_H_

#include <string>
#include <vector>

#include "icl/delegate.h"
#include "icl/err.h"
#include "icl/input_file.h"
#include "icl/parse_tree.h"
#include "icl/scope.h"
//#include "tools/gn/scope_per_file_provider.h"
//#include "tools/gn/target.h"
#include "icl/token.h"
//#include "tools/gn/toolchain.h"
#include "icl/value.h"

namespace icl {

// A helper class for setting up a Scope that a test can use. It makes a
// toolchain and sets up all the build state.
//
// Note: This implements |Delegate| and is not thread-safe.
class TestWithScope : public Delegate {
 public:
  TestWithScope();
  ~TestWithScope();

  TestWithScope(const TestWithScope&) = delete;
  TestWithScope& operator=(const TestWithScope&) = delete;

  Scope* scope() { return &scope_; }
  const Scope::ItemVector& items() { return items_; }

  // This buffer accumulates output from any print() commands executed in the
  // context of this test. Note that the implementation of this is not
  // threadsafe so don't write tests that call print from multiple threads.
  std::string& print_output() { return print_output_; }

  // |Delegate| methods:
  void Print(const std::string& s) override;

 private:
  Scope scope_;
  Scope::ItemVector items_;

  // Supplies the scope with built-in variables like root_out_dir.
//FIXME
//  ScopePerFileProvider scope_progammatic_provider_;

  std::string print_output_;
};

// Helper class to treat some string input as a file.
//
// Instantiate it with the contents you want, be sure to check for error, and
// then you can execute the ParseNode or whatever.
class TestParseInput {
 public:
  explicit TestParseInput(const std::string& input);
  ~TestParseInput();

  TestParseInput(const TestParseInput&) = delete;
  TestParseInput& operator=(const TestParseInput&) = delete;

  // Indicates whether and what error occurred during tokenizing and parsing.
  bool has_error() const { return parse_err_.has_error(); }
  const Err& parse_err() const { return parse_err_; }

  const InputFile& input_file() const { return input_file_; }
  const std::vector<Token>& tokens() const { return tokens_; }
  const ParseNode* parsed() const { return parsed_.get(); }

 private:
  InputFile input_file_;

  std::vector<Token> tokens_;
  std::unique_ptr<ParseNode> parsed_;

  Err parse_err_;
};

//FIXME
#if 0
// Shortcut for creating targets for tests that take the test setup, a pretty-
// style label, and a target type and sets everything up. The target will
// default to public visibility.
class TestTarget : public Target {
 public:
  TestTarget(const TestWithScope& setup,
             const std::string& label_string,
             Target::OutputType type);
  ~TestTarget() override;
};
#endif

}  // namespace icl

#endif  // ICL_TEST_WITH_SCOPE_H_
