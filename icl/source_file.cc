// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/source_file.h"

#include <assert.h>

#include <utility>

#include "icl/filesystem_utils.h"
#include "icl/source_dir.h"

namespace icl {

SourceFile::SourceFile(std::string&& value) : value_(std::move(value)) {
  assert(!value_.empty());
  assert(value_.front() == '/');
  assert(value_.back() != '/');
  NormalizePath(&value_);
}

std::string SourceFile::GetName() const {
  if (is_null())
    return std::string();

  assert(value_.find('/') != std::string::npos);
  size_t last_slash = value_.rfind('/');
  return std::string(&value_[last_slash + 1],
                     value_.size() - last_slash - 1);
}

SourceDir SourceFile::GetDir() const {
  if (is_null())
    return SourceDir();

  assert(value_.find('/') != std::string::npos);
  size_t last_slash = value_.rfind('/');
  return SourceDir(value_.substr(0u, last_slash + 1));
}

std::string SourceFile::Resolve(const StringPiece& source_root) const {
  assert(!source_root.empty());

  if (is_null())
    return std::string();

  if (is_system_absolute())
    return value_;

  return (source_root.back() == '/')
      ? source_root.as_string() + value_.substr(2)
      : source_root.as_string() + '/' + value_.substr(2);
}

}  // namespace icl
