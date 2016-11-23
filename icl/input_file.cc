// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/input_file.h"

#include <assert.h>

#include <utility>

namespace icl {

InputFile::InputFile(const SourceFile& name)
    : name_(name), dir_(name_.GetDir()) {}

InputFile::~InputFile() = default;

void InputFile::SetContents(std::string&& c) {
  assert(!contents_loaded_);
  contents_loaded_ = true;
  contents_ = std::move(c);
}

}  // namespace icl
