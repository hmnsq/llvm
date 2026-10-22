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

#ಅನ್ತರ್ಭಾವ "llvm/Support/COM.h"

#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h"

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#ಅನ್ತರ್ಭಾವ "Unix/COM.inc"
#ನೋಚೇದ್ಯದಿ defined(_WIN32)
#ಅನ್ತರ್ಭಾವ "Windows/COM.inc"
#endif
