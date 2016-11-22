// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_SOURCE_FILE_H_
#define ICL_SOURCE_FILE_H_

#include <assert.h>
#include <stddef.h>

#include <functional>
#include <string>

//FIXME
//#include "base/files/file_path.h"
#include "icl/source_dir.h"
#include "icl/string_piece.h"

namespace icl {

class SourceDir;

// Represents a file within the source tree. Always begins in a slash, never
// ends in one.
class SourceFile {
 public:
  SourceFile();

  // Takes a known absolute source file. Always begins in a slash.
  explicit SourceFile(std::string&& value);

  ~SourceFile();

  // Copy and assign supported.
//FIXME support move?

  bool is_null() const { return value_.empty(); }
  const std::string& value() const { return value_; }

  // Returns everything after the last slash.
  std::string GetName() const;
  SourceDir GetDir() const;

  // Resolves this source file relative to some given source root. Returns
  // an empty file path on error.
//FIXME
//  base::FilePath Resolve(const base::FilePath& source_root) const;

  // Returns true if this file starts with a "//" which indicates a path
  // from the source root.
  bool is_source_absolute() const {
    return value_.size() >= 2 && value_[0] == '/' && value_[1] == '/';
  }

//FIXME this seems very slightly dubious (is "null" system-absolute?)
  // Returns true if this file starts with a single slash which indicates a
  // system-absolute path.
  bool is_system_absolute() const {
    return !is_source_absolute();
  }

  // Returns a source-absolute path starting with only one slash at the
  // beginning (normally source-absolute paths start with two slashes to mark
  // them as such). This is normally used when concatenating names together.
  //
  // This function asserts that the file is actually source-absolute. The
  // return value points into our buffer.
  StringPiece SourceAbsoluteWithOneSlash() const {
    assert(is_source_absolute());
    return StringPiece(&value_[1], value_.size() - 1);
  }

  bool operator==(const SourceFile& other) const {
    return value_ == other.value_;
  }
  bool operator!=(const SourceFile& other) const {
    return !operator==(other);
  }
  bool operator<(const SourceFile& other) const {
    return value_ < other.value_;
  }

  void swap(SourceFile& other) {
    value_.swap(other.value_);
  }

 private:
  friend class SourceDir;

  std::string value_;
};

inline void swap(SourceFile& lhs, SourceFile& rhs) {
  lhs.swap(rhs);
}

}  // namespace icl

namespace std {

template<> struct hash<icl::SourceFile> {
  std::size_t operator()(const icl::SourceFile& v) const {
    return std::hash<std::string>()(v.value());
  }
};

}  // namespace std

#endif  // ICL_SOURCE_FILE_H_
