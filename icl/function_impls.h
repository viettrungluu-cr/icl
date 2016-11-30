// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_FUNCTION_IMPLS_H_
#define ICL_FUNCTION_IMPLS_H_

#include "icl/function.h"

namespace icl {
namespace function_impls {

// Adds entries for the standard functions (which must not already be present)
// to |*functions|. (The standard functions are listed below.)
void AddStandardFunctions(FunctionMap* functions);

// Gets a |FunctionMap| with just the standard functions. (The standard
// functions are listed below.)
FunctionMap GetStandardFunctions();

// Gets a |FunctionMap| with the standard functions, plus "import". (The
// standard functions are indicated below.)
FunctionMap GetStandardFunctionsWithImport();

// Get entries for build-in functions. Standard functions are indicated by *.
FunctionMapEntry AssertFn();    // * "assert"
FunctionMapEntry DefinedFn();   // * "defined"
FunctionMapEntry ImportFn();    // * "defined"
FunctionMapEntry PrintFn();     // * "print"
FunctionMapEntry ForEachFn();   // * "foreach" (function_impls_foreach.cc)
FunctionMapEntry TemplateFn();  // * "template" (function_impls_template.cc)

}  // namespace function_impls
}  // namespace icl

#endif  // ICL_FUNCTION_IMPLS_H_
