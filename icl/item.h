// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_ITEM_H_
#define ICL_ITEM_H_

#include <string>

namespace icl {

class Delegate;
class ParseNode;

// A named item (target, config, etc.) defined in an input file.
class Item {
 public:
  virtual ~Item() = default;

  const char* type() const { return type_; }
  const Delegate* delegate() const { return delegate_; }

  const ParseNode* defined_from() const { return defined_from_; }
  void set_defined_from(const ParseNode* df) { defined_from_ = df; }

 protected:
  // Note: |type| (the pointer value, not just the string value!) should
  // identify the implementing subclass, since it may be used for manually RTTI
  // purposes (by comparing pointer values!). It should also be a friendly,
  // human-readable name.
  Item(const char* type, Delegate* delegate)
      : type_(type), delegate_(delegate) {}

  Item(const Item&) = delete;
  Item& operator=(const Item&) = delete;

 private:
  const char* const type_;
  Delegate* const delegate_;
  const ParseNode* defined_from_ = nullptr;
};

}  // namespace icl

#endif  // ICL_ITEM_H_
