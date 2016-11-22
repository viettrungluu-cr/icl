// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_ITEM_H_
#define ICL_ITEM_H_

#include <string>

namespace icl {

//class Config;
class ParseNode;
class Settings;
//class Target;

// A named item (target, config, etc.) that participates in the dependency
// graph.
class Item {
 public:
  explicit Item(const Settings* settings);
  virtual ~Item();

  const Settings* settings() const { return settings_; }

  const ParseNode* defined_from() const { return defined_from_; }
  void set_defined_from(const ParseNode* df) { defined_from_ = df; }

/*
  // Manual RTTI.
  virtual Config* AsConfig();
  virtual const Config* AsConfig() const;
  virtual Target* AsTarget();
  virtual const Target* AsTarget() const;

  // Returns a name like "target" or "config" for the type of item this is, to
  // be used in logging and error messages.
  std::string GetItemTypeName() const;

  // Called when this item is resolved, meaning it and all of its dependents
  // have no unresolved deps. Returns true on success. Sets the error and
  // returns false on failure.
  virtual bool OnResolved(Err* err);
*/

 private:
  const Settings* settings_;
  const ParseNode* defined_from_;
};

}  // namespace icl

#endif  // ICL_ITEM_H_
