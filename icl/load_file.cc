// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/load_file.h"

#include <memory>
#include <utility>
#include <vector>

#include "icl/delegate.h"
#include "icl/err.h"
#include "icl/input_file.h"
#include "icl/location.h"
#include "icl/parse_tree.h"
#include "icl/parser.h"
#include "icl/token.h"
#include "icl/tokenizer.h"

namespace icl {

bool LoadFile(const LocationRange& origin,
              Delegate* delegate,
              const SourceFile& name,
              InputFile* file,
              Err* err) {
  {
    std::string contents;
    if (!delegate->LoadFile(name, &contents)) {
      *err = Err(origin, "Unable to load \"" + name.value() + "\".");
      return false;
    }
    assert(file->name() == name);
    file->SetContents(std::move(contents));
  }

  {
    std::vector<Token> tokens = Tokenizer::Tokenize(file, err);
    if (err->has_error())
      return false;
    file->SetTokens(std::move(tokens));
  }

  {
    std::unique_ptr<ParseNode> root_parse_node =
        Parser::Parse(file->tokens(), err);
    if (err->has_error())
      return false;
    file->SetRootParseNode(std::move(root_parse_node));
  }

  return true;
}

}  // namespace icl
