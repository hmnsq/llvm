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

#ضمن "llvm/Support/MemoryBufferRef.h"
#ضمن "llvm/Support/MemoryBuffer.h"

using نطاق llvm;

MemoryBufferRef::MemoryBufferRef(ثابت MemoryBuffer &Buffer)
    : Buffer(Buffer.getBuffer()), Identifier(Buffer.getBufferIdentifier()) {}
