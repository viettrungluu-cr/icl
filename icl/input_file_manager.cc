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

#if 0
#include <utility>

#include "base/bind.h"
#include "base/stl_util.h"
#include "tools/gn/filesystem_utils.h"
#include "tools/gn/parser.h"
#include "tools/gn/scheduler.h"
#include "tools/gn/scope_per_file_provider.h"
#include "tools/gn/tokenizer.h"

namespace {

void InvokeFileLoadCallback(const InputFileManager::FileLoadCallback& cb,
                            const ParseNode* node) {
  cb.Run(node);
}

bool DoLoadFile(const LocationRange& origin,
                const BuildSettings* build_settings,
                const SourceFile& name,
                InputFile* file,
                std::vector<Token>* tokens,
                std::unique_ptr<ParseNode>* root,
                Err* err) {
  // Read.
  base::FilePath primary_path = build_settings->GetFullPath(name);
  if (!file->Load(primary_path)) {
    if (!build_settings->secondary_source_path().empty()) {
      // Fall back to secondary source tree.
      base::FilePath secondary_path =
          build_settings->GetFullPathSecondary(name);
      if (!file->Load(secondary_path)) {
        *err = Err(origin, "Can't load input file.",
                   "Unable to load:\n  " +
                   FilePathToUTF8(primary_path) + "\n"
                   "I also checked in the secondary tree for:\n  " +
                   FilePathToUTF8(secondary_path));
        return false;
      }
    } else {
      *err = Err(origin,
                 "Unable to load \"" + FilePathToUTF8(primary_path) + "\".");
      return false;
    }
  }

  // Tokenize.
  *tokens = Tokenizer::Tokenize(file, err);
  if (err->has_error())
    return false;

  // Parse.
  *root = Parser::Parse(*tokens, err);
  if (err->has_error())
    return false;

  return true;
}

}  // namespace

InputFileManager::InputFileData::InputFileData(const SourceFile& file_name)
    : file(file_name),
      loaded(false) {
}

InputFileManager::InputFileData::~InputFileData() {
}

InputFileManager::InputFileManager() = default;

InputFileManager::~InputFileManager() = default;

const ParseNode* InputFileManager::SyncLoadFile(
    const LocationRange& origin,
    const BuildSettings* build_settings,
    const SourceFile& file_name,
    Err* err) {
  base::AutoLock lock(lock_);

  InputFileData* data = nullptr;
  InputFileMap::iterator found = input_files_.find(file_name);
  if (found == input_files_.end()) {
    // Haven't seen this file yet, start loading right now.
    std::unique_ptr<InputFileData> new_data(new InputFileData(file_name));
    data = new_data.get();
    input_files_[file_name] = std::move(new_data);

    base::AutoUnlock unlock(lock_);
    if (!LoadFile(origin, build_settings, file_name, &data->file, err))
      return nullptr;
  } else {
    // This file has either been loaded or is pending loading.
    data = found->second.get();

    if (!data->loaded) {
      // Wait for the already-pending sync load to complete.
      if (!data->completion_event) {
        data->completion_event.reset(new base::WaitableEvent(
            base::WaitableEvent::ResetPolicy::AUTOMATIC,
            base::WaitableEvent::InitialState::NOT_SIGNALED));
      }
      {
        base::AutoUnlock unlock(lock_);
        data->completion_event->Wait();
      }
      // If there were multiple waiters on the same event, we now need to wake
      // up the next one.
      data->completion_event->Signal();
    }
  }

  // The other load could have failed. It is possible that this thread's error
  // will be reported to the scheduler before the other thread's (and the first
  // error reported "wins"). Forward the parse error from the other load for
  // this thread so that the error message is useful.
  if (!data->parsed_root)
    *err = data->parse_error;
  return data->parsed_root.get();
}

void InputFileManager::AddDynamicInput(
    const SourceFile& name,
    InputFile** file,
    std::vector<Token>** tokens,
    std::unique_ptr<ParseNode>** parse_root) {
  std::unique_ptr<InputFileData> data(new InputFileData(name));
  *file = &data->file;
  *tokens = &data->tokens;
  *parse_root = &data->parsed_root;
  {
    base::AutoLock lock(lock_);
    dynamic_inputs_.push_back(std::move(data));
  }
}

int InputFileManager::GetInputFileCount() const {
  base::AutoLock lock(lock_);
  return static_cast<int>(input_files_.size());
}

void InputFileManager::GetAllPhysicalInputFileNames(
    std::vector<base::FilePath>* result) const {
  base::AutoLock lock(lock_);
  result->reserve(input_files_.size());
  for (const auto& file : input_files_) {
    if (!file.second->file.physical_name().empty())
      result->push_back(file.second->file.physical_name());
  }
}

bool InputFileManager::LoadFile(const LocationRange& origin,
                                const BuildSettings* build_settings,
                                const SourceFile& name,
                                InputFile* file,
                                Err* err) {
  std::vector<Token> tokens;
  std::unique_ptr<ParseNode> root;
  bool success = DoLoadFile(origin, build_settings, name, file,
                            &tokens, &root, err);
  // Can't return early. We have to ensure that the completion event is
  // signaled in all cases bacause another thread could be blocked on this one.

//FIXME
  // Save this pointer for running the callbacks below, which happens after the
  // scoped ptr ownership is taken away inside the lock.
  ParseNode* unowned_root = root.get();
  {
    base::AutoLock lock(lock_);
    DCHECK(input_files_.find(name) != input_files_.end());

    InputFileData* data = input_files_[name].get();
    data->loaded = true;
    if (success) {
      data->tokens.swap(tokens);
      data->parsed_root = std::move(root);
    } else {
      data->parse_error = *err;
    }
  }

  return success;
}
#endif
