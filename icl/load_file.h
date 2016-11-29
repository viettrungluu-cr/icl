// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_LOAD_FILE_H_
#define ICL_LOAD_FILE_H_

namespace icl {

class Delegate;
class InputFile;
class LocationRange;
class SourceFile;

bool LoadFile(const LocationRange& origin,
              Delegate* delegate,
              const SourceFile& name,
              InputFile* file);

}  // namespace icl

#endif  // ICL_LOAD_FILE_H_
