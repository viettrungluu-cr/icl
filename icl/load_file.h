// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_LOAD_FILE_H_
#define ICL_LOAD_FILE_H_

#include <functional>

namespace icl {

class InputFile;
class LocationRange;
class SourceFile;

using ReadFileFunction = std::function<bool(const SourceFile&, std::string*)>;

bool LoadFile(ReadFileFunction read_file_function,
              const LocationRange& origin,
              const SourceFile& name,
              InputFile* file);

}  // namespace icl

#endif  // ICL_LOAD_FILE_H_
