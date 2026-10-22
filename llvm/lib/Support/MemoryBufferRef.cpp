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

#अन्तर्भाव "llvm/Support/MemoryBufferRef.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"

उपयुज्य नामाकाश llvm;

स्मृतिः_बफर_Ref::स्मृतिः_बफर_Ref(ध्रुव स्मृतिः_बफर &बफर)
    : बफर(बफर.get_बफर()), Identifier(बफर.get_बफर_Identifier()) {}
