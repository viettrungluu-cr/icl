// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_DELEGATE_H_
#define ICL_DELEGATE_H_

#include <string>

#include "icl/functions.h"

namespace icl {

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
  // TODO(vtl): Should this take a |StringPiece| instead?
  virtual void Print(const std::string& s) = 0;

  virtual const FunctionInfoMap& GetFunctions() const = 0;

 protected:
  Delegate() = default;
  ~Delegate() = default;
};

}  // namespace icl

#endif  // ICL_DELEGATE_H_
