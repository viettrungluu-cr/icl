// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>

#include <iostream>

#include "icl/delegate.h"
#include "icl/function_impls.h"
#include "icl/runner.h"
#include "icl/source_file.h"

namespace {

class DelegateImpl : public icl::Delegate {
 public:
  DelegateImpl() : functions_(icl::function_impls::GetStandardFunctions()) {}
  ~DelegateImpl() = default;

  DelegateImpl(const DelegateImpl&) = delete;
  DelegateImpl& operator=(const DelegateImpl&) = delete;

  // |icl::Delegate| methods:
  bool LoadFile(const icl::SourceFile& name, std::string* contents) override {
    assert(name.value() == "//noname");
    *contents = "a = \"hello world\"\n"
                "print(a)";
    return true;
  }
  void Print(const std::string& s) override {
    std::cout << s;
  }
  const icl::FunctionMap& GetFunctions() const override {
    return functions_;
  }

 private:
  const icl::FunctionMap functions_;
};

}  // namespace

int main() {
  DelegateImpl delegate;
  icl::Runner runner(&delegate);
  auto result = runner.Run(icl::SourceFile("//noname"));
  if (!result.is_success()) {
    fprintf(stderr, "PANIC: %s", result.error_message().c_str());
    return -1;
  }
  return 0;
}
