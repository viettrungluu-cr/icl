// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_SOURCE_DIR_H_
#define ICL_SOURCE_DIR_H_

#include <assert.h>
#include <stddef.h>

#include <functional>
#include <string>

#include "icl/string_piece.h"
//FIXME
//#include "base/files/file_path.h"

namespace icl {

class Err;
class SourceFile;
class Value;

// Represents a directory within the source tree. Source dirs begin and end in
// slashes.
//
// If there is one slash at the beginning, it will mean a system-absolute file
// path. On Windows, absolute system paths will be of the form "/C:/foo/bar".
//
// Two slashes at the beginning indicate a path relative to the source root.
class SourceDir {
 public:
  SourceDir();
  explicit SourceDir(std::string&& s);

  ~SourceDir();

//FIXME move?
  // Copy and assign allowed.

  // Resolves a file or dir name relative to this source directory. Will return
  // an empty SourceDir/File on error and set the give *err pointer (required).
  // Empty input is always an error.
  //
  // If source_root is supplied, these functions will additionally handle the
  // case where the input is a system-absolute but still inside the source
  // tree. This is the case for some external tools.
  SourceFile ResolveRelativeFile(
      const Value& p,
      Err* err,
      const StringPiece& source_root = StringPiece()) const;
  SourceDir ResolveRelativeDir(
      const Value& p,
      Err* err,
      const StringPiece& source_root = StringPiece()) const;

  // Like ResolveRelativeDir but takes a separate value (which gets blamed)
  // and string to use (in cases where a substring has been extracted from the
  // value, as with label resolution).
  SourceDir ResolveRelativeDir(
      const Value& blame_but_dont_use,
      const StringPiece& p,
      Err* err,
      const StringPiece& source_root = StringPiece()) const;

  // Resolves this source file relative to some given source root. Returns
  // an empty file path on error.
//FIXME
//  base::FilePath Resolve(const base::FilePath& source_root) const;

  bool is_null() const { return value_.empty(); }
  const std::string& value() const { return value_; }

  // Returns true if this path starts with a "//" which indicates a path
  // from the source root.
  bool is_source_absolute() const {
    return value_.size() >= 2 && value_[0] == '/' && value_[1] == '/';
  }

  // Returns true if this path starts with a single slash which indicates a
  // system-absolute path.
  bool is_system_absolute() const {
    return !is_source_absolute();
  }

  // Returns a source-absolute path starting with only one slash at the
  // beginning (normally source-absolute paths start with two slashes to mark
  // them as such). This is normally used when concatenating directories
  // together.
  //
  // This function asserts that the directory is actually source-absolute. The
  // return value points into our buffer.
  StringPiece SourceAbsoluteWithOneSlash() const {
    assert(is_source_absolute());
    return StringPiece(&value_[1], value_.size() - 1);
  }

  void SwapValue(std::string* v);

  bool operator==(const SourceDir& other) const {
    return value_ == other.value_;
  }
  bool operator!=(const SourceDir& other) const {
    return !operator==(other);
  }
  bool operator<(const SourceDir& other) const {
    return value_ < other.value_;
  }

  void swap(SourceDir& other) {
    value_.swap(other.value_);
  }

 private:
  friend class SourceFile;
  std::string value_;
};

inline void swap(SourceDir& lhs, SourceDir& rhs) {
  lhs.swap(rhs);
}

}  // namespace icl

namespace std {

template<> struct hash<icl::SourceDir> {
  std::size_t operator()(const icl::SourceDir& v) const {
    return std::hash<std::string>()(v.value());
  }
};

}  // namespace std

#endif  // ICL_SOURCE_DIR_H_
