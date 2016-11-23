// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_RUNNER_H_
#define ICL_RUNNER_H_

#include <memory>
#include <string>

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

    bool succeeded() const { return succeeded_; }
    const std::string& error_message() const { return error_message_; }

   private:
    friend class Runner;

    bool succeeded_ = false;
    std::string error_message_;

    std::unique_ptr<Scope> scope_;
  };

  explicit Runner(Delegate* delegate);
  ~Runner();

  Runner(const Runner&) = delete;
  Runner& operator=(const Runner&) = delete;

  RunResult Run(const SourceFile& source_file);

 private:
  Delegate* const delegate_;
};

}  // namespace icl

#endif  // ICL_RUNNER_H_
