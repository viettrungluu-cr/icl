// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/runner.h"

#include <memory>
#include <utility>

#include "icl/delegate.h"
#include "icl/err.h"
#include "icl/input_file.h"
#include "icl/load_file.h"
#include "icl/parse_tree.h"
#include "icl/parser.h"
#include "icl/scope.h"
#include "icl/source_file.h"
#include "icl/tokenizer.h"

namespace icl {

Runner::RunResult::RunResult(const SourceFile& name) : file_(name) {}
Runner::RunResult::RunResult(RunResult&&) = default;
Runner::RunResult::~RunResult() = default;
Runner::RunResult& Runner::RunResult::operator=(RunResult&&) = default;

Runner::Runner(Delegate* delegate) : delegate_(delegate) {}

Runner::~Runner() = default;

Runner::RunResult Runner::Run(const SourceFile& source_file) {
  RunResult result(source_file);

  LocationRange no_origin;
  Err err;
  if (!LoadFile(no_origin, delegate_, source_file, &result.file_, &err)) {
    result.error_message_ = err.GetErrorMessage();
    return result;
  }

  // TODO(C++14): Use std::make_unique.
  auto scope = std::unique_ptr<Scope>(new Scope(delegate_));

  result.file_.root_parse_node()->Execute(scope.get(), &err);
  if (err.has_error()) {
    result.error_message_ = err.GetErrorMessage();
    return result;
  }

  result.is_success_ = true;
  result.scope_ = std::move(scope);
  return result;
}

}  // namespace icl
