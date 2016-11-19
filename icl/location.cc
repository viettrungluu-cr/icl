// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/location.h"

#include <assert.h>

#include <tuple>

#include "icl/input_file.h"
#include "icl/string_number_conversions.h"

namespace icl {

Location::Location()
    : file_(nullptr),
      line_number_(-1),
      column_number_(-1) {
}

Location::Location(const InputFile* file,
                   int line_number,
                   int column_number,
                   int byte)
    : file_(file),
      line_number_(line_number),
      column_number_(column_number),
      byte_(byte) {
}

bool Location::operator==(const Location& other) const {
  return other.file_ == file_ &&
         other.line_number_ == line_number_ &&
         other.column_number_ == column_number_;
}

bool Location::operator!=(const Location& other) const {
  return !operator==(other);
}

bool Location::operator<(const Location& other) const {
  assert(file_ == other.file_);
  return std::tie(line_number_, column_number_) <
         std::tie(other.line_number_, other.column_number_);
}

std::string Location::Describe(bool include_column_number) const {
  if (!file_)
    return std::string();

  std::string ret = file_->friendly_name().empty() ? file_->name()
                                                   : file_->friendly_name();
  ret += ":";
  ret += NumberToString<int>(line_number_);
  if (include_column_number) {
    ret += ":";
    ret += NumberToString<int>(column_number_);
  }
  return ret;
}

LocationRange::LocationRange() {
}

LocationRange::LocationRange(const Location& begin, const Location& end)
    : begin_(begin),
      end_(end) {
  assert(begin_.file() == end_.file());
}

LocationRange LocationRange::Union(const LocationRange& other) const {
  assert(begin_.file() == other.begin_.file());
  return LocationRange(
      begin_ < other.begin_ ? begin_ : other.begin_,
      end_ < other.end_ ? other.end_ : end_);
}

}  // namespace icl
