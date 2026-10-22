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

#ضمن "llvm/Support/Watchdog.h"
#ضمن "llvm/Config/llvm-config.h"

// Include the platform-specific parts of this class.
#اگر_تعریف_شده LLVM_ON_UNIX
#ضمن "Unix/Watchdog.inc"
#پایان_اگر
#اگر_تعریف_شده _WIN32
#ضمن "Windows/Watchdog.inc"
#پایان_اگر
