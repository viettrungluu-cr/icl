// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_ITEM_IMPLS_H_
#define ICL_ITEM_IMPLS_H_

#include <string>
#include <unordered_map>

#include "icl/function.h"
#include "icl/item.h"
#include "icl/string_piece.h"
#include "icl/value.h"

namespace icl {

class BagImpl;

// Bags just accept all variables.
class BagItem : public Item {
 public:
  // This should match |Scope::KeyValueMap|.
  using KeyValueMap = std::unordered_map<StringPiece, Value, StringPieceHash>;

  static FunctionMapEntry Fn(const char* type);

  ~BagItem() override;

  BagItem(const BagItem&) = delete;
  BagItem& operator=(const BagItem&) = delete;

  const std::string& name() const { return name_; }
  const KeyValueMap& key_value_map() const { return key_value_map_; }

 private:
  friend class BagImpl;

  BagItem(const char* type, Delegate* delegate, const std::string& name);

  const std::string name_;
  KeyValueMap key_value_map_;
};


}  // namespace icl

#endif  // ICL_ITEM_IMPLS_H_
