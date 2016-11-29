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

  bool LoadFile(const LocationRange& origin,
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

//FIXME
#if 0
#include <set>
#include <utility>
#include <vector>

#include "base/callback.h"
#include "base/containers/hash_tables.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/synchronization/lock.h"
#include "base/synchronization/waitable_event.h"
#include "tools/gn/build_settings.h"
#include "tools/gn/input_file.h"
#include "tools/gn/parse_tree.h"
#include "tools/gn/settings.h"

class Err;
class LocationRange;
class ParseNode;
class Token;

// Manages loading and parsing files from disk. This doesn't actually have
// any context for executing the results, so potentially multiple configs
// could use the same input file (saving parsing).
//
// This class is threadsafe.
//
// InputFile objects must never be deleted while the program is running since
// various state points into them.
class InputFileManager : public base::RefCountedThreadSafe<InputFileManager> {
 public:
  InputFileManager();

  // Loads and parses the given file synchronously, returning the root block
  // corresponding to the parsed result. On error, return NULL and the given
  // Err is set.
  const ParseNode* SyncLoadFile(const LocationRange& origin,
                                const BuildSettings* build_settings,
                                const SourceFile& file_name,
                                Err* err);

  // Creates an entry to manage the memory associated with keeping a parsed
  // set of code in memory.
  //
  // The values pointed to by the parameters will be filled with pointers to
  // the file, tokens, and parse node that this class created. The calling
  // code is responsible for populating these values and maintaining
  // threadsafety. This class' only job is to hold onto the memory and delete
  // it when the program exits.
  //
  // This solves the problem that sometimes we need to execute something
  // dynamic and save the result, but the values all have references to the
  // nodes and file that created it. Either we need to reset the origin of
  // the values and lose context for error reporting, or somehow keep the
  // associated parse nodes, tokens, and file data in memory. This function
  // allows the latter.
  void AddDynamicInput(const SourceFile& name,
                       InputFile** file,
                       std::vector<Token>** tokens,
                       std::unique_ptr<ParseNode>** parse_root);

  // Does not count dynamic input.
  int GetInputFileCount() const;

  // Fills the vector with all input files.
  void GetAllPhysicalInputFileNames(std::vector<base::FilePath>* result) const;

 private:
  friend class base::RefCountedThreadSafe<InputFileManager>;

  struct InputFileData {
    explicit InputFileData(const SourceFile& file_name);
    ~InputFileData();

    // This is protected by the owning |InputFileManager|'s lock.
    bool loaded = false;

    // Don't touch this outside the lock until it's marked loaded.
    InputFile file;

    // Event to signal when the load is complete (or fails). This is lazily
    // created only when a thread is synchronously waiting for this load (which
    // only happens for imports).
    std::unique_ptr<base::WaitableEvent> completion_event;

    std::vector<Token> tokens;

    // Null before the file is loaded or if loading failed.
    std::unique_ptr<ParseNode> parsed_root;
    Err parse_error;
  };

  virtual ~InputFileManager();

  // Loads the given file. On error, sets the Err and return false.
  bool LoadFile(const LocationRange& origin,
                const BuildSettings* build_settings,
                const SourceFile& name,
                InputFile* file,
                Err* err);

  mutable base::Lock lock_;

  // Maps repo-relative filenames to the corresponding owned pointer.
  typedef base::hash_map<SourceFile, std::unique_ptr<InputFileData>>
      InputFileMap;
  InputFileMap input_files_;

  // Tracks all dynamic inputs. The data are holders for memory management
  // purposes and should not be read or modified by this class. The values
  // will be vended out to the code creating the dynamic input, who is in
  // charge of the threadsafety requirements.
  //
  // See AddDynamicInput().
  std::vector<std::unique_ptr<InputFileData>> dynamic_inputs_;

  DISALLOW_COPY_AND_ASSIGN(InputFileManager);
};
#endif

#endif  // ICL_INPUT_FILE_MANAGER_H_
