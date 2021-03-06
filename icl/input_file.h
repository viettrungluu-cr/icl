// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_INPUT_FILE_H_
#define ICL_INPUT_FILE_H_

#include <assert.h>

#include <memory>
#include <string>
#include <vector>

#include "icl/err.h"
#include "icl/source_dir.h"
#include "icl/source_file.h"
#include "icl/token.h"

namespace icl {

class ParseNode;

class InputFile {
 public:
  explicit InputFile(const SourceFile& name);
  InputFile(InputFile&&);

  ~InputFile();

  InputFile& operator=(InputFile&&);

  // Copy construction/assignment disallowed (but move allowed).
  InputFile(const InputFile&) = delete;
  InputFile& operator=(const InputFile&) = delete;

  // The name passed into the constructor.
  const SourceFile& name() const { return name_; }

  // The directory is just a cached version of name()->GetDir() but we get this
  // a lot so computing it once up front saves a bunch of work.
  const SourceDir& dir() const { return dir_; }

  // The friendly name can be set to override the name() in cases where there
  // is no name (like SetContents is used instead) or if the name doesn't
  // make sense. This will be displayed in error messages.
  const std::string& friendly_name() const { return friendly_name_; }
  void set_friendly_name(const std::string& f) { friendly_name_ = f; }

  // Loading an input file may fail at various stages. If it has failed, |err()|
  // will report an error.
  const Err& err() const { return err_; }
  void set_err(const Err& err) { err_ = err; }

  const std::string& contents() const {
    assert(contents_loaded_);
    return contents_;
  }

  // Sets the contents of the file; this may be called at most once.
  void SetContents(std::string&& contents);

  const std::vector<Token>& tokens() const {
    assert(tokens_set_);
    return tokens_;
  }

  // Sets the tokens; this may be called at most once.
  void SetTokens(std::vector<Token>&& tokens);

  const ParseNode* root_parse_node() const {
    assert(root_parse_node_set_);
    return root_parse_node_.get();
  }

  // Sets the root parse node; this may be called at most once.
  void SetRootParseNode(std::unique_ptr<ParseNode> root_parse_node);

 private:
  SourceFile name_;
  SourceDir dir_;

  std::string friendly_name_;

  Err err_;

  // Note: |contents_| must outlive |tokens_| which in turn must outlive
  // |root_parse_node_|.
  bool contents_loaded_ = false;
  std::string contents_;

  bool tokens_set_ = false;
  std::vector<Token> tokens_;

  bool root_parse_node_set_ = false;
  std::unique_ptr<ParseNode> root_parse_node_;
};

}  // namespace icl

#endif  // ICL_INPUT_FILE_H_
