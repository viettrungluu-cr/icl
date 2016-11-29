// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>

#include <iostream>

#include "icl/delegate.h"
#include "icl/function_impls.h"
#include "icl/import_manager.h"
#include "icl/input_file_manager.h"
#include "icl/runner.h"
#include "icl/source_file.h"

namespace {

class DelegateImpl : public icl::Delegate {
 public:
  DelegateImpl()
      : functions_(icl::function_impls::GetStandardFunctionsWithImport()),
        input_file_manager_(&DelegateImpl::ReadFile) {}
  ~DelegateImpl() = default;

  DelegateImpl(const DelegateImpl&) = delete;
  DelegateImpl& operator=(const DelegateImpl&) = delete;

  // |icl::Delegate| methods:
  bool GetInputFile(const icl::LocationRange& origin,
                    const icl::SourceFile& name,
                    const icl::InputFile** file) override {
    return input_file_manager_.GetFile(origin, name, file);
  }
  void Print(const std::string& s) override {
    std::cout << s;
  }
  const icl::FunctionMap& GetFunctions() const override {
    return functions_;
  }
  icl::ImportManager* GetImportManager() override {
    return &import_manager_;
  }

 private:
  static bool ReadFile(const icl::SourceFile& name, std::string* contents) {
    if (name.value() == "//initial") {
      *contents = "print(\"hello\")\n"
                  "import(\"//import1\")\n"
                  "print(\"hello to you too\")\n"
                  "import(\"//import2\")\n";
      return true;
    }
    if (name.value() == "//import1") {
      *contents = "print(\"nice to meet you, I'm import1\")\n";
      return true;
    }
    if (name.value() == "//import2") {
      *contents = "print(\"nice to meet you too, I'm import2\")\n";
      return true;
    }
    assert(false);
    return false;
  }

  const icl::FunctionMap functions_;
  icl::InputFileManager input_file_manager_;
  icl::ImportManager import_manager_;
};

}  // namespace

int main() {
  DelegateImpl delegate;
  icl::Runner runner(&delegate);
  auto result = runner.Run(icl::SourceFile("//initial"));
  if (!result.is_success()) {
    fprintf(stderr, "PANIC: %s", result.error_message().c_str());
    return -1;
  }
  return 0;
}
