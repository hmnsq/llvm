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

#અન્તર્ભાવ "llvm/Support/MemoryBufferRef.h"
#અન્તર્ભાવ "llvm/Support/MemoryBuffer.h"

using નામાકાશ llvm;

MemoryBufferRef::MemoryBufferRef(અચલ MemoryBuffer &Buffer)
    : Buffer(Buffer.getBuffer()), Identifier(Buffer.getBufferIdentifier()) {}
