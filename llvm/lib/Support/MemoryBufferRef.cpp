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

#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBufferRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBuffer.h"

using ನಾಮಾಕಾಶ llvm;

MemoryBufferRef::MemoryBufferRef(ಧ್ರುವ MemoryBuffer &Buffer)
    : Buffer(Buffer.getBuffer()), Identifier(Buffer.getBufferIdentifier()) {}
