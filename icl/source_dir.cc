// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/source_dir.h"

#include <assert.h>

#include <utility>

#include "icl/err.h"
#include "icl/filesystem_utils.h"
#include "icl/source_file.h"
#include "icl/value.h"

namespace icl {

namespace {

void AssertValueSourceDirString(const std::string& s) {
  if (!s.empty()) {
    assert(s.front() == '/');
    assert(s.back() == '/');
  }
}

void EnsureEndsWithSlash(std::string* s) {
  if (s->empty() || s->back() != '/')
    s->push_back('/');
}

}  // namespace

SourceDir::SourceDir() = default;

SourceDir::SourceDir(std::string&& s)
    : value_(std::move(s)) {
  EnsureEndsWithSlash(&value_);
  AssertValueSourceDirString(value_);
}

SourceDir::~SourceDir() = default;

SourceFile SourceDir::ResolveRelativeFile(
    const Value& p,
    Err* err,
    const StringPiece& source_root) const {
  // TODO(vtl): What are we supposed to do if we're null/empty?
  assert(!is_null());

  SourceFile ret;
  if (!p.VerifyTypeIs(Value::STRING, err))
    return ret;

  // It's an error to resolve an empty string or one that is a directory
  // (indicated by a trailing slash) because this is the function that expects
  // to return a file.
  const std::string& str = p.string_value();
  if (str.empty()) {
    *err = Err(p, "Empty file path.",
               "You can't use empty strings as file paths. That's just wrong.");
    return ret;
  } else if (str[str.size() - 1] == '/') {
    *err = Err(p, "File path ends in a slash.",
               "You specified the path\n  " + str + "\n"
               "and it ends in a slash, indicating you think it's a directory."
               "\nBut here you're supposed to be listing a file.");
    return ret;
  }

  if (str.size() >= 2 && str[0] == '/' && str[1] == '/') {
    // Source-relative.
    ret.value_.assign(str.data(), str.size());
    NormalizePath(&ret.value_, source_root);
    return ret;
  } else if (IsPathAbsolute(str)) {
    if (source_root.empty() ||
        !MakeAbsolutePathRelativeIfPossible(source_root, str, &ret.value_)) {
      ret.value_.append(str.data(), str.size());
    }
    NormalizePath(&ret.value_);
    return ret;
  }

  if (!source_root.empty()) {
    std::string absolute = Resolve(source_root) + str;
    NormalizePath(&absolute);
    if (!MakeAbsolutePathRelativeIfPossible(source_root, absolute,
                                            &ret.value_)) {
      ret.value_.append(absolute.data(), absolute.size());
    }
    return ret;
  }

  // With no source_root_, there's nothing we can do about
  // e.g. p=../../../path/to/file and value_=//source and we'll
  // erroneously return //file.
  ret.value_.reserve(value_.size() + str.size());
  ret.value_.assign(value_);
  ret.value_.append(str.data(), str.size());

  NormalizePath(&ret.value_);
  return ret;
}

SourceDir SourceDir::ResolveRelativeDir(const Value& p,
                                        Err* err,
                                        const StringPiece& source_root) const {
  if (!p.VerifyTypeIs(Value::STRING, err))
    return SourceDir();
  return ResolveRelativeDir(p, p.string_value(), err, source_root);
}

SourceDir SourceDir::ResolveRelativeDir(const Value& blame_but_dont_use,
                                        const StringPiece& str,
                                        Err* err,
                                        const StringPiece& source_root) const {
  // TODO(vtl): What are we supposed to do if we're null/empty?
  assert(!is_null());

  SourceDir ret;

  if (str.empty()) {
    *err = Err(blame_but_dont_use, "Empty directory path.",
               "You can't use empty strings as directories. "
               "That's just wrong.");
    return ret;
  }

  if (str.size() >= 2 && str[0] == '/' && str[1] == '/') {
    // Source-relative.
    ret.value_.assign(str.data(), str.size());
    EnsureEndsWithSlash(&ret.value_);
    NormalizePath(&ret.value_, source_root);
    return ret;
  } else if (IsPathAbsolute(str)) {
    if (source_root.empty() ||
        !MakeAbsolutePathRelativeIfPossible(source_root, str, &ret.value_)) {
      ret.value_.append(str.data(), str.size());
    }
    NormalizePath(&ret.value_);
    EnsureEndsWithSlash(&ret.value_);
    return ret;
  }

  if (!source_root.empty()) {
    std::string absolute = Resolve(source_root) + str.as_string();
    NormalizePath(&absolute);
    if (!MakeAbsolutePathRelativeIfPossible(source_root, absolute,
                                            &ret.value_)) {
      ret.value_.append(absolute.data(), absolute.size());
    }
    EnsureEndsWithSlash(&ret.value_);
    return ret;
  }

  ret.value_.reserve(value_.size() + str.size());
  ret.value_.assign(value_);
  ret.value_.append(str.data(), str.size());

  NormalizePath(&ret.value_);
  EnsureEndsWithSlash(&ret.value_);
  AssertValueSourceDirString(ret.value_);

  return ret;
}

std::string SourceDir::Resolve(const StringPiece& source_root) const {
  assert(!source_root.empty());

  if (is_null())
    return std::string();

  if (is_system_absolute())
    return value_;

  return (source_root.back() == '/')
      ? source_root.as_string() + value_.substr(2)
      : source_root.as_string() + '/' + value_.substr(2);
}

void SourceDir::SwapValue(std::string* v) {
  value_.swap(*v);
  AssertValueSourceDirString(value_);
}

}  // namespace icl
