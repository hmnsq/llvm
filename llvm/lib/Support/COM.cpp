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

#અન્તર્ભાવ "llvm/Support/COM.h"

#અન્તર્ભાવ "llvm/Config/llvm-config.h"

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#અન્તર્ભાવ "Unix/COM.inc"
#અથવાજો defined(_WIN32)
#અન્તર્ભાવ "Windows/COM.inc"
#endif
