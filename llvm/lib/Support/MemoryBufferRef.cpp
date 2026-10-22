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

#অন্তর্ভাৱ "llvm/Support/MemoryBufferRef.h"
#অন্তর্ভাৱ "llvm/Support/MemoryBuffer.h"

using নামাকাশ llvm;

MemoryBufferRef::MemoryBufferRef(ধ্রুৱ MemoryBuffer &Buffer)
    : Buffer(Buffer.getBuffer()), Identifier(Buffer.getBufferIdentifier()) {}
