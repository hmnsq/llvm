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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಸ್ಮರಣೆ_ಬಫರ್_Ref::ಸ್ಮರಣೆ_ಬಫರ್_Ref(ಸ್ಥಿರಾಂಕ ಸ್ಮರಣೆ_ಬಫರ್ &ಬಫರ್)
    : ಬಫರ್(ಬಫರ್.get_ಬಫರ್()), Identifier(ಬಫರ್.get_ಬಫರ್_Identifier()) {}
