// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_DELEGATE_H_
#define ICL_DELEGATE_H_

#include <string>

#include "icl/function.h"
#include "icl/string_piece.h"

namespace icl {

class Err;
class ImportManager;
class InputFile;
class LocationRange;
class SourceFile;

// Interface for the user to provide various required functionality/settings.
//
// Lifetime: A delegate must outlive not just the |Scope| (and
// parsing/execution), but also the resulting |Item|s.
//
// Thread safety: If the delegate will be used in a multithreaded way (including
// parsing/executing files with the same |Scope| on multiple threads), then the
// delegate must be thread-safe.
class Delegate {
 public:
//FIXME needs a comment
  virtual const FunctionMap& GetFunctions() const = 0;

  // Gets the import manager (typically just a member). Must return non-null if
  // import statements are supported.
  // TODO(vtl): Should this be const?
  virtual ImportManager* GetImportManager() = 0;

  // Gets (reading/loading/parsing) the input file specified by |name| to
  // |*file| (|**file| will live as long as this object). Returns true on
  // success and false on failure, in which case |*file| will still be set (but
  // with an error). |origin| is the requesting location (e.g., of an import
  // statement) and is used for error reporting.
  //
  // Note: Typically this is implemented by having an |InputFileManager| member.
  // TODO(vtl): Should this be const?
  virtual bool GetInputFile(const LocationRange& origin,
                            const SourceFile& name,
                            const InputFile** file) = 0;

//FIXME needs a comment
  virtual StringPiece GetSourceRoot() const = 0;

  // TODO(vtl): Should this take a |StringPiece| instead?
  virtual void Print(const std::string& s) = 0;

 protected:
  Delegate() = default;
  ~Delegate() = default;
};

}  // namespace icl

#endif  // ICL_DELEGATE_H_
