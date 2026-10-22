//===- MemoryBufferRef.cpp - Memory Buffer Reference ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements the MemoryBufferRef interface.
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/MemoryBufferRef.h"
#समावेश "llvm/Support/MemoryBuffer.h"

using namespace llvm;

Memory_बफर_Ref::Memory_बफर_Ref(स्थिर Memory_बफर &बफर)
    : बफर(बफर.get_बफर()), Identifier(बफर.get_बफर_Identifier()) {}
