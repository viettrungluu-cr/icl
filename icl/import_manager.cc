// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "icl/import_manager.h"

#include <assert.h>

#include <algorithm>
#include <memory>
#include <mutex>

#include "icl/delegate.h"
#include "icl/err.h"
#include "icl/input_file.h"
#include "icl/load_file.h"
#include "icl/parse_tree.h"
#include "icl/scope.h"
//FIXME
//#include "tools/gn/scope_per_file_provider.h"

namespace icl {

namespace {

// Returns a newly-allocated scope on success, null on failure.
std::unique_ptr<Scope> UncachedImport(Delegate* delegate,
                                      const SourceFile& name,
                                      const ParseNode* node_for_err,
                                      Err* err) {
  const InputFile* file = nullptr;
  if (!delegate->GetInputFile(node_for_err->GetRange(), name, &file)) {
    assert(file);
    assert(file->err().has_error());
    *err = file->err();
    return nullptr;
  }
  assert(file);
  assert(!file->err().has_error());
  assert(file->root_parse_node());

  std::unique_ptr<Scope> scope(new Scope(delegate));
  scope->set_source_dir(name.GetDir());

  // Don't allow ScopePerFileProvider to provide target-related variables.
  // These will be relative to the imported file, which is probably not what
  // people mean when they use these.
//FIXME
//  ScopePerFileProvider per_file_provider(scope.get(), false);

  scope->SetProcessingImport();
  file->root_parse_node()->Execute(scope.get(), err);
  if (err->has_error()) {
    // If there was an error, append the caller location so the error message
    // displays a why the file was imported (esp. useful for failed asserts).
    err->AppendSubErr(Err(node_for_err, "whence it was imported."));
    return nullptr;
  }
  scope->ClearProcessingImport();

  return scope;
}

}  // namespace

struct ImportManager::ImportInfo {
  ImportInfo() = default;
  ~ImportInfo() = default;

  // This lock protects the unique_ptr. Once the scope is computed,
  // it is const and can be accessed read-only outside of the lock.
  std::mutex load_mutex;

  std::unique_ptr<const Scope> scope;

  // The result of loading the import. If the load failed, the scope will be
  // null but this will be set to error. In this case the thread should not
  // attempt to load the file, even if the scope is null.
  Err load_result;
};

ImportManager::ImportManager() = default;

ImportManager::~ImportManager() = default;

bool ImportManager::DoImport(const SourceFile& name,
                             const ParseNode* node_for_err,
                             Scope* scope,
                             Err* err) {
  // See if we have a cached import, but be careful to actually do the scope
  // copying outside of the lock.
  ImportInfo* import_info = nullptr;
  {
    std::lock_guard<std::mutex> lock(imports_mutex_);
    std::unique_ptr<ImportInfo>& info_ptr = imports_[name];
    if (!info_ptr)
      info_ptr.reset(new ImportInfo);

    // Promote the ImportInfo to outside of the imports lock.
    import_info = info_ptr.get();
  }

  // Now use the per-import-file lock to block this thread if another thread
  // is already processing the import.
  const Scope* import_scope = nullptr;
  {
    std::lock_guard<std::mutex> lock(import_info->load_mutex);

    if (!import_info->scope) {
      // Only load if the import hasn't already failed.
      if (!import_info->load_result.has_error()) {
        import_info->scope = UncachedImport(
            scope->delegate(), name, node_for_err, &import_info->load_result);
      }
      if (import_info->load_result.has_error()) {
        *err = import_info->load_result;
        return false;
      }
    }

    // Promote the now-read-only scope to outside the load lock.
    import_scope = import_info->scope.get();
  }

  Scope::MergeOptions options;
  options.skip_private_vars = true;
  options.mark_dest_used = true;  // Don't require all imported values be used.
  return import_scope->NonRecursiveMergeTo(scope, options, node_for_err,
                                           "import", err);
}

std::vector<SourceFile> ImportManager::GetImportedFiles() const {
  std::vector<SourceFile> imported_files;
  imported_files.resize(imports_.size());
  std::transform(imports_.begin(), imports_.end(), imported_files.begin(),
                 [](const ImportMap::value_type& val) { return val.first; });
  return imported_files;
}

}  // namespace icl
