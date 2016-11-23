// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_DELEGATE_H_
#define ICL_DELEGATE_H_

#include <string>

#include "icl/functions.h"

namespace icl {

class SourceFile;

// Interface for the user to provide various required functionality/settings.
//
// Lifetime: A delegate must outlive not just the |Scope| (and
// parsing/execution), but also the resulting |Item|s.
//
// Thread safety: If the delegate will be used in a multithreaded way (including
// parsing/executing files with the same |Scope| on multiple threads), then the
// delegate must be thread-safe.
class Delegate {
 public:
  // Loads the contents of the file named |name| (which must be non-"null") to
  // |*contents| (|contents| must be non-null and point to an empty string).
  // Returns true on success or false on failure (in which case |*contents| is
  // undefined).
  virtual bool LoadFile(const SourceFile& name, std::string* contents) = 0;

  // TODO(vtl): Should this take a |StringPiece| instead?
  virtual void Print(const std::string& s) = 0;

  virtual const FunctionMap& GetFunctions() const = 0;

 protected:
  Delegate() = default;
  ~Delegate() = default;
};

}  // namespace icl

#endif  // ICL_DELEGATE_H_
