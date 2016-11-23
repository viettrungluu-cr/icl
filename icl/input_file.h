// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_INPUT_FILE_H_
#define ICL_INPUT_FILE_H_

#include <assert.h>

#include <string>

#include "icl/source_dir.h"
#include "icl/source_file.h"

namespace icl {

class InputFile {
 public:
  explicit InputFile(const SourceFile& name);

  ~InputFile();

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

  const std::string& contents() const {
    assert(contents_loaded_);
    return contents_;
  }

  // Sets the contents of the file; this may be called at most once.
  void SetContents(std::string&& c);

 private:
  const SourceFile name_;
  const SourceDir dir_;

  std::string friendly_name_;

  bool contents_loaded_ = false;
  std::string contents_;
};

}  // namespace icl

#endif  // ICL_INPUT_FILE_H_
