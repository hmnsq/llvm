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

#অন্তর্ভাব "llvm/Support/MemoryBufferRef.h"
#অন্তর্ভাব "llvm/Support/MemoryBuffer.h"

using নামাকাশ llvm;

Memory_বাফার_Ref::Memory_বাফার_Ref(ধ্রুবক Memory_বাফার &বাফার)
    : বাফার(বাফার.get_বাফার()), Identifier(বাফার.get_বাফার_Identifier()) {}
