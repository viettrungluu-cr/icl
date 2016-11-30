// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>

#include <iostream>

#include "icl/delegate.h"
#include "icl/function_impls.h"
#include "icl/input_file_manager.h"
#include "icl/item_impls.h"
#include "icl/runner.h"
#include "icl/source_file.h"

namespace {

const char kMyBag[] = "my_bag";

class DelegateImpl : public icl::Delegate {
 public:
  DelegateImpl() : functions_(MakeFunctions()),
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
    assert(false);
    return nullptr;
  }

 private:
  static bool ReadFile(const icl::SourceFile& name, std::string* contents) {
    assert(name.value() == "//noname");
    *contents = "my_bag(\"foo\") {\n"
                "  x = 123\n"
                "  y = [\"a\", \"b\", \"c\"]\n"
                "}\n"
                "\n"
                "my_bag(\"bar\") {\n"
                "  hello = 123\n"
                "  hello = \"world\"\n"
                "}\n";
    return true;
  }

  static icl::FunctionMap MakeFunctions() {
    icl::FunctionMap functions;
    icl::function_impls::AddStandardFunctions(&functions);
    functions.insert(icl::BagItem::Fn(kMyBag));
    return functions;
  }

  const icl::FunctionMap functions_;
  icl::InputFileManager input_file_manager_;
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
  for (const auto& item : result.items()) {
    assert(item);
    assert(item->type() == kMyBag);
    auto bag_item = static_cast<const icl::BagItem*>(item.get());
    std::cout << bag_item->name() << ':' << std::endl;
    for (const auto& kv : bag_item->key_value_map()) {
      std::cout << "  " << kv.first.as_string() << " = "
                << kv.second.ToString(true) << std::endl;
    }
    std::cout << std::endl;
  }
  return 0;
}
