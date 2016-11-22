// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_ERR_H_
#define ICL_ERR_H_

#include <string>
#include <vector>

#include "icl/location.h"
#include "icl/token.h"

namespace icl {

class ParseNode;
class Value;

// Result of doing some operation. Check has_error() to see if an error
// occurred.
//
// An error has a location and a message. Below that, is some optional help
// text to go with the annotation of the location.
//
// An error can also have sub-errors which are additionally printed out
// below. They can provide additional context.
class Err {
 public:
  typedef std::vector<LocationRange> RangeList;

  // Indicates no error.
  Err();

  // Error at a single point.
  Err(const Location& location,
      const std::string& msg,
      const std::string& help = std::string());

  // Error at a given range.
  Err(const LocationRange& range,
      const std::string& msg,
      const std::string& help = std::string());

  // Error at a given token.
  Err(const Token& token,
      const std::string& msg,
      const std::string& help_text = std::string());

  // Error at a given node.
  Err(const ParseNode* node,
      const std::string& msg,
      const std::string& help_text = std::string());

  // Error at a given value.
  Err(const Value& value,
      const std::string msg,
      const std::string& help_text = std::string());

  Err(const Err& other);

  ~Err();

  bool has_error() const { return has_error_; }
  const Location& location() const { return location_; }
  const std::string& message() const { return message_; }
  const std::string& help_text() const { return help_text_; }

  void AppendRange(const LocationRange& range) { ranges_.push_back(range); }
  const RangeList& ranges() const { return ranges_; }

  void AppendSubErr(const Err& err);

  // Appends a human-readable error message to |*out|.
  void AppendErrorMessage(std::string* out) const;

  // Convenience form of |AppendErrorMessage()|.
  std::string GetErrorMessage() const;

 private:
  void AppendErrorMessageInternal(bool is_sub_err, std::string* out) const;

  bool has_error_;
  Location location_;

  std::vector<LocationRange> ranges_;

  std::string message_;
  std::string help_text_;

  std::vector<Err> sub_errs_;
};

}  // namespace icl

#endif  // ICL_ERR_H_
