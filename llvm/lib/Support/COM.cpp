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

#समावेश "llvm/Support/COM.h"

#समावेश "llvm/Config/llvm-config.h"

// Include the platform-specific parts of this class.
#अगर_घोषित LLVM_ON_UNIX
#समावेश "Unix/COM.inc"
#या_अगर defined(_WIN32)
#समावेश "Windows/COM.inc"
#पूर्ण_अगर
