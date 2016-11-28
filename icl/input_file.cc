// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/input_file.h"

#include <assert.h>

#include <utility>

#include "icl/parse_tree.h"

namespace icl {

InputFile::InputFile(const SourceFile& name)
    : name_(name), dir_(name_.GetDir()) {}

InputFile::InputFile(InputFile&&) = default;

InputFile::~InputFile() = default;

InputFile& InputFile::operator=(InputFile&&) = default;

void InputFile::SetContents(std::string&& contents) {
  assert(!contents_loaded_);
  contents_loaded_ = true;
  contents_ = std::move(contents);
}

void InputFile::SetTokens(std::vector<Token>&& tokens) {
  assert(!tokens_set_);
  tokens_set_ = true;
  tokens_ = std::move(tokens);
}

void InputFile::SetRootParseNode(std::unique_ptr<ParseNode> root_parse_node) {
  assert(!root_parse_node_set_);
  root_parse_node_set_ = true;
  root_parse_node_ = std::move(root_parse_node);
}

}  // namespace icl
