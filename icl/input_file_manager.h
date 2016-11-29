// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_INPUT_FILE_MANAGER_H_
#define ICL_INPUT_FILE_MANAGER_H_

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "icl/load_file.h"  // For |ReadFileFunction|.

namespace icl {

class InputFile;
class LocationRange;
class SourceFile;

class InputFileManager {
 public:
  explicit InputFileManager(ReadFileFunction read_file_function);
  ~InputFileManager();

  InputFileManager(const InputFileManager&) = delete;
  InputFileManager& operator=(const InputFileManager&) = delete;

  // Gets (reading/loading/parsing) the file specified by |name| to |*file|
  // (|**file| will live as long as this object). Returns true on success and
  // false on failure, in which case |*file| will still be set (but with an
  // error). |origin| is the requesting location (e.g., of an import statement)
  // and is used for error reporting.
  bool GetFile(const LocationRange& origin,
               const SourceFile& name,
               const InputFile** file);

 private:
  struct InputFileInfo;

  const ReadFileFunction read_file_function_;

  // Protects access to input_files_. Do not hold when actually loading input
  // files.
  std::mutex input_files_mutex_;

  // Owning pointers to the scopes.
  using InputFileMap = std::map<SourceFile, std::unique_ptr<InputFileInfo>>;
  InputFileMap input_files_;
};

}  // namespace icl

#endif  // ICL_INPUT_FILE_MANAGER_H_
