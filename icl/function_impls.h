// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ICL_FUNCTION_IMPLS_H_
#define ICL_FUNCTION_IMPLS_H_

#include "icl/functions.h"

namespace icl {
namespace function_impls {

Value RunAssert(Scope* scope,
                const FunctionCallNode* function,
                const std::vector<Value>& args,
                Err* err);
inline FunctionInfoMapEntry AssertFn() { return {"assert", &RunAssert}; }

Value RunDefined(Scope* scope,
                 const FunctionCallNode* function,
                 const ListNode* args_list,
                 Err* err);
inline FunctionInfoMapEntry DefinedFn() { return {"defined", &RunDefined}; }

Value RunPrint(Scope* scope,
               const FunctionCallNode* function,
               const std::vector<Value>& args,
               Err* err);
inline FunctionInfoMapEntry PrintFn() { return {"print", &RunPrint}; }

// Implemented in function_impls_foreach.cc:
Value RunForEach(Scope* scope,
                 const FunctionCallNode* function,
                 const ListNode* args_list,
                 Err* err);
inline FunctionInfoMapEntry ForEachFn() { return {"foreach", &RunForEach}; }

//
/*
Value RunTemplate(Scope* scope,
                  const FunctionCallNode* function,
                  const std::vector<Value>& args,
                  BlockNode* block,
                  Err* err);
*/

//FIXME
/*
extern const char kConfig[];
Value RunConfig(const FunctionCallNode* function,
                const std::vector<Value>& args,
                Scope* block_scope,
                Err* err);

extern const char kDeclareArgs[];
Value RunDeclareArgs(Scope* scope,
                     const FunctionCallNode* function,
                     const std::vector<Value>& args,
                     BlockNode* block,
                     Err* err);

extern const char kForwardVariablesFrom[];
Value RunForwardVariablesFrom(Scope* scope,
                              const FunctionCallNode* function,
                              const ListNode* args_list,
                              Err* err);

extern const char kGetEnv[];
Value RunGetEnv(Scope* scope,
                const FunctionCallNode* function,
                const std::vector<Value>& args,
                Err* err);

extern const char kGetPathInfo[];
Value RunGetPathInfo(Scope* scope,
                     const FunctionCallNode* function,
                     const std::vector<Value>& args,
                     Err* err);

extern const char kImport[];
Value RunImport(Scope* scope,
                const FunctionCallNode* function,
                const std::vector<Value>& args,
                Err* err);

extern const char kProcessFileTemplate[];
Value RunProcessFileTemplate(Scope* scope,
                             const FunctionCallNode* function,
                             const std::vector<Value>& args,
                             Err* err);

extern const char kReadFile[];
Value RunReadFile(Scope* scope,
                  const FunctionCallNode* function,
                  const std::vector<Value>& args,
                  Err* err);

extern const char kRebasePath[];
Value RunRebasePath(Scope* scope,
                    const FunctionCallNode* function,
                    const std::vector<Value>& args,
                    Err* err);

extern const char kSplitList[];
Value RunSplitList(Scope* scope,
                   const FunctionCallNode* function,
                   const ListNode* args_list,
                   Err* err);

extern const char kWriteFile[];
Value RunWriteFile(Scope* scope,
                   const FunctionCallNode* function,
                   const std::vector<Value>& args,
                   Err* err);
*/

}  // namespace function_impls
}  // namespace icl

#endif  // ICL_FUNCTION_IMPLS_H_
