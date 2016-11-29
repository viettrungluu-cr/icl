// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_RUNNER_H_
#define ICL_RUNNER_H_

#include <memory>
#include <string>

#include "icl/input_file.h"

namespace icl {

class Delegate;
class Scope;
class SourceFile;

class Runner {
 public:
  class RunResult {
   public:
    RunResult();
    RunResult(RunResult&&);
    ~RunResult();

    RunResult& operator=(RunResult&&);

    // Copy construction/assignment disallowed (but move allowed).
    RunResult(const RunResult&) = delete;
    RunResult& operator=(const RunResult&) = delete;

    bool is_success() const { return is_success_; }
    const std::string& error_message() const { return error_message_; }

   private:
    friend class Runner;

    bool is_success_ = false;
    std::string error_message_;

    const InputFile* file_ = nullptr;
    std::unique_ptr<Scope> scope_;
  };

  explicit Runner(Delegate* delegate);
  ~Runner();

  Runner(const Runner&) = delete;
  Runner& operator=(const Runner&) = delete;

  RunResult Run(const SourceFile& name);

 private:
  Delegate* const delegate_;
};

}  // namespace icl

#endif  // ICL_RUNNER_H_
