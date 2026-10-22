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

#അന്തര്ഭാവ "llvm/Support/MemoryBufferRef.h"
#അന്തര്ഭാവ "llvm/Support/MemoryBuffer.h"

using നാമാകാശ llvm;

MemoryBufferRef::MemoryBufferRef(സ്ഥിരം MemoryBuffer &Buffer)
    : Buffer(Buffer.getBuffer()), Identifier(Buffer.getBufferIdentifier()) {}
