// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/runner.h"

#include <memory>
#include <utility>

#include "icl/delegate.h"
#include "icl/err.h"
#include "icl/input_file.h"
#include "icl/parse_tree.h"
#include "icl/parser.h"
#include "icl/scope.h"
#include "icl/source_file.h"
#include "icl/tokenizer.h"

namespace icl {

Runner::RunResult::RunResult() = default;
Runner::RunResult::RunResult(RunResult&&) = default;
Runner::RunResult::~RunResult() = default;
Runner::RunResult& Runner::RunResult::operator=(RunResult&&) = default;

Runner::Runner(Delegate* delegate) : delegate_(delegate) {}

Runner::~Runner() = default;

Runner::RunResult Runner::Run(const SourceFile& source_file) {
  RunResult result;

  std::string source_file_contents;
  if (!delegate_->LoadFile(source_file, &source_file_contents)) {
    result.error_message_ =
        "ERROR " + source_file.value() + ": Failed to load file.";
    return result;
  }

  InputFile input_file(source_file);
  input_file.SetContents(std::move(source_file_contents));

  // |err| may refer to things in |*scope|, so it's safer to create it before
  // |err|.
  auto scope = std::unique_ptr<Scope>(new Scope(delegate_));
  Err err;

  auto tokens = Tokenizer::Tokenize(&input_file, &err);
  if (err.has_error()) {
    result.error_message_ = err.GetErrorMessage();
    return result;
  }

  auto ast = Parser::Parse(tokens, &err);
  if (err.has_error()) {
    result.error_message_ = err.GetErrorMessage();
    return result;
  }

  // TODO(C++14): Use std::make_unique.
  ast->Execute(scope.get(), &err);
  if (err.has_error()) {
    result.error_message_ = err.GetErrorMessage();
    return result;
  }

  result.is_success_ = true;
  result.scope_ = std::move(scope);
  return result;
}

}  // namespace icl
