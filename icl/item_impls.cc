// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/item_impls.h"

#include <assert.h>

#include <memory>
#include <string>
#include <utility>

#include "icl/item.h"
#include "icl/parse_tree.h"
#include "icl/scope.h"

namespace icl {

class BagImpl : public Function {
 public:
  explicit BagImpl(const char* type) : type_(type) {}
  ~BagImpl() override = default;
  Type GetType() const override { return Type::GENERIC_BLOCK; }
  Value GenericBlockFn(Scope* scope,
                       const FunctionCallNode* function,
                       const std::vector<Value>& args,
                       BlockNode* block,
                       Err* err) const override {
    NonNestableBlock non_nestable(scope, function, type_);
    if (!non_nestable.Enter(err))
      return Value();

    if (!EnsureSingleStringArg(function, args, err) ||
        !EnsureNotProcessingImport(function, scope, err))
      return Value();

    Scope block_scope(scope);
    block->Execute(&block_scope, err);
    if (err->has_error())
      return Value();

    std::unique_ptr<BagItem> bag_item(new BagItem(type_, scope->delegate(),
                                                  args[0].string_value()));
    block_scope.GetCurrentScopeValues(&bag_item->key_value_map_);
    block_scope.MarkAllUsed();

    auto collector = scope->GetItemCollector();
    if (!collector) {
      *err = Err(function, std::string("Can't define an item of type ") +
                               type_ + " in this context.");
      return Value();
    }
    collector->push_back(std::move(bag_item));

    return Value();
  }

 private:
  const char* const type_;
};

// static
FunctionMapEntry BagItem::Fn(const char* type) {
  // TODO(C++14): Use std::make_unique.
  return {type, std::unique_ptr<BagImpl>(new BagImpl(type))};
}

BagItem::BagItem(const char* type, Delegate* delegate, const std::string& name)
    : Item(type, delegate), name_(name) {}

BagItem::~BagItem() = default;


}  // namespace icl
