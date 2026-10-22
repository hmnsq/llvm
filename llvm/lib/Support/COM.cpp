//===-- COM.cpp - Implement COM utility classes -----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements utility classes related to COM.
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/COM.h"

#അന്തര്ഭാവ "llvm/Config/llvm-config.h"

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#അന്തര്ഭാവ "Unix/COM.inc"
#അല്ലെങ്കിലെങ്കിൽ defined(_WIN32)
#അന്തര്ഭാവ "Windows/COM.inc"
#endif
