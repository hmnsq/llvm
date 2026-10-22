//===---- Watchdog.cpp - Implement Watchdog ---------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements the Watchdog class.
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/Watchdog.h"
#समावेश "llvm/Config/llvm-config.h"

// Include the platform-specific parts of this class.
#अगर_घोषित LLVM_ON_UNIX
#समावेश "Unix/Watchdog.inc"
#पूर्ण_अगर
#अगर_घोषित _WIN32
#समावेश "Windows/Watchdog.inc"
#पूर्ण_अगर
