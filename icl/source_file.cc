// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/source_file.h"

#include <assert.h>

#include <utility>

//FIXME
//#include "tools/gn/filesystem_utils.h"
//#include "tools/gn/source_dir.h"

namespace icl {

SourceFile::SourceFile() = default;

SourceFile::SourceFile(std::string&& value) : value_(std::move(value)) {
  assert(!value_.empty());
  assert(value_.front() == '/');
  assert(value_.back() != '/');
//FIXME
//  NormalizePath(&value_);
}

SourceFile::~SourceFile() = default;

std::string SourceFile::GetName() const {
  if (is_null())
    return std::string();

  assert(value_.find('/') != std::string::npos);
  size_t last_slash = value_.rfind('/');
  return std::string(&value_[last_slash + 1],
                     value_.size() - last_slash - 1);
}

//FIXME
/*
SourceDir SourceFile::GetDir() const {
  if (is_null())
    return SourceDir();

  assert(value_.find('/') != std::string::npos);
  size_t last_slash = value_.rfind('/');
  return SourceDir(StringPiece(&value_.front(), last_slash + 1));
}
*/

//FIXME
/*
base::FilePath SourceFile::Resolve(const base::FilePath& source_root) const {
  if (is_null())
    return base::FilePath();

  std::string converted;
  if (is_system_absolute()) {
    if (value_.size() > 2 && value_[2] == ':') {
      // Windows path, strip the leading slash.
      converted.assign(&value_[1], value_.size() - 1);
    } else {
      converted.assign(value_);
    }
    return base::FilePath(UTF8ToFilePath(converted));
  }

  converted.assign(&value_[2], value_.size() - 2);
  if (source_root.empty())
    return UTF8ToFilePath(converted).NormalizePathSeparatorsTo('/');
  return source_root.Append(UTF8ToFilePath(converted))
      .NormalizePathSeparatorsTo('/');
}
*/

}  // namespace icl
