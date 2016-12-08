// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/runner.h"

#include <assert.h>

#include <memory>
#include <utility>

#include "icl/delegate.h"
#include "icl/err.h"
#include "icl/input_file.h"
#include "icl/item.h"
#include "icl/load_file.h"
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

Runner::RunResult Runner::Run(const SourceFile& name) {
  RunResult result;

  LocationRange no_origin;
  const InputFile* file = nullptr;
  if (!delegate_->GetInputFile(no_origin, name, &file)) {
    assert(file);
    assert(file->err().has_error());
    result.error_message_ = file->err().GetErrorMessage();
    return result;
  }
  assert(file);
  assert(!file->err().has_error());

  Scope scope(delegate_);
  scope.set_source_dir(name.GetDir());
  scope.set_item_collector(&result.items_);

  Err err;
  file->root_parse_node()->Execute(&scope, &err);
  if (err.has_error()) {
    result.error_message_ = err.GetErrorMessage();
    return result;
  }

  result.is_success_ = true;
  return result;
}

}  // namespace icl
