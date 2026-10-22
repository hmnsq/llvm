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

#அந்தர்பாவ "llvm/Support/MemoryBufferRef.h"
#அந்தர்பாவ "llvm/Support/MemoryBuffer.h"

using நாமாகாஶ llvm;

MemoryBufferRef::MemoryBufferRef(மாறிலி MemoryBuffer &Buffer)
    : Buffer(Buffer.getBuffer()), Identifier(Buffer.getBufferIdentifier()) {}
