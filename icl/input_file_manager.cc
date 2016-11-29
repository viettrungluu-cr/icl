// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/input_file_manager.h"

#include <assert.h>

#include <memory>
#include <mutex>
#include <utility>

#include "icl/err.h"
#include "icl/input_file.h"
#include "icl/load_file.h"
#include "icl/source_file.h"

namespace icl {

struct InputFileManager::InputFileInfo {
  InputFileInfo() = default;
  ~InputFileInfo() = default;

  // This lock protects the unique_ptr. Once the input file is loaded (i.e.,
  // |input_file| is non-null), it is const and can be accessed read-only
  // outside of the lock.
  std::mutex load_mutex;

  // Even if loading fails, this will be set (with an error).
  std::unique_ptr<const InputFile> input_file;
};

InputFileManager::InputFileManager(ReadFileFunction read_file_function)
    : read_file_function_(std::move(read_file_function)) {}

InputFileManager::~InputFileManager() = default;

bool InputFileManager::LoadFile(const LocationRange& origin,
                                const SourceFile& name,
                                const InputFile** file) {
  // See if we have a cached load.
  InputFileInfo* input_file_info = nullptr;
  {
    std::lock_guard<std::mutex> lock(input_files_mutex_);
    std::unique_ptr<InputFileInfo>& info_ptr = input_files_[name];
    if (!info_ptr)
      info_ptr.reset(new InputFileInfo);

    // Promote the InputFileInfo to outside of the input files lock.
    input_file_info = info_ptr.get();
  }

  // Now use the per-input-file lock to block this thread if another thread is
  // already processing the input file.
  {
    std::lock_guard<std::mutex> lock(input_file_info->load_mutex);

    if (input_file_info->input_file) {
      *file = input_file_info->input_file.get();
      return (*file)->err().has_error();
    }

    if (!input_file_info->input_file) {
      InputFile* f = new InputFile(name);
      *file = f;
      input_file_info->input_file.reset(f);
      if (!icl::LoadFile(read_file_function_, origin, name, f)) {
        assert(f->err().has_error());
        return false;
      }
    }
  }

  assert(input_file_info->input_file);
  assert(*file == input_file_info->input_file.get());
  assert(!(*file)->err().has_error());
  return true;
}

}  // namespace icl
