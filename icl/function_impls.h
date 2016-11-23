// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_FUNCTION_IMPLS_H_
#define ICL_FUNCTION_IMPLS_H_

#include "icl/functions.h"

namespace icl {
namespace function_impls {

FunctionInfoMapEntry AssertFn();    // "assert"
FunctionInfoMapEntry DefinedFn();   // "defined"
FunctionInfoMapEntry PrintFn();     // "print"
FunctionInfoMapEntry ForEachFn();   // "foreach" (function_impls_foreach.cc)
FunctionInfoMapEntry TemplateFn();  // "template" (function_impls_template.cc)

}  // namespace function_impls
}  // namespace icl

#endif  // ICL_FUNCTION_IMPLS_H_
