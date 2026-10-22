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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/COM.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h"

// Include the platform-specific parts of this class.
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LLVM_ON_UNIX
#ਅਨ੍ਤਰ੍ਭਾਵ "Unix/COM.inc"
#ਜਾਂਜੇ defined(_WIN32)
#ਅਨ੍ਤਰ੍ਭਾਵ "Windows/COM.inc"
#ਜੇ_ਅੰਤ
