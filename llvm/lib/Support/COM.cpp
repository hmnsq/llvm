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

#شامل "llvm/Support/COM.h"

#شامل "llvm/Config/llvm-config.h"

// Include the platform-specific parts of this class.
#اگر_تعریف_شدہ LLVM_ON_UNIX
#شامل "Unix/COM.inc"
#وگرنہ_اگر defined(_WIN32)
#شامل "Windows/COM.inc"
#اگر_ختم
