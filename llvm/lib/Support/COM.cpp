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

#অন্তর্ভাব "llvm/Support/COM.h"

#অন্তর্ভাব "llvm/Config/llvm-config.h"

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#অন্তর্ভাব "Unix/COM.inc"
#নোচেদ্যদি defined(_WIN32)
#অন্তর্ভাব "Windows/COM.inc"
#endif
