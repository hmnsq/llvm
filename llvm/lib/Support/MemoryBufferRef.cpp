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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemoryBufferRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemoryBuffer.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਸਮ੍ਰਿਤੀ_ਬਫਰ_Ref::ਸਮ੍ਰਿਤੀ_ਬਫਰ_Ref(ਅਚਲ ਸਮ੍ਰਿਤੀ_ਬਫਰ &ਬਫਰ)
    : ਬਫਰ(ਬਫਰ.get_ਬਫਰ()), Identifier(ਬਫਰ.get_ਬਫਰ_Identifier()) {}
