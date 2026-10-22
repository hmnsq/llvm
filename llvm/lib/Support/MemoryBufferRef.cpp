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

#شامل "llvm/Support/MemoryBufferRef.h"
#شامل "llvm/Support/MemoryBuffer.h"

استعمال نامخلا llvm;

حافظہ_بفر_Ref::حافظہ_بفر_Ref(ثابت حافظہ_بفر &بفر)
    : بفر(بفر.get_بفر()), Identifier(بفر.get_بفر_Identifier()) {}
