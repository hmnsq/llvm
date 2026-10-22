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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/Watchdog.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/llvm-config.h"

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#ଅନ୍ତର୍ଭାଵ "Unix/Watchdog.inc"
#endif
#ifdef _WIN32
#ଅନ୍ତର୍ଭାଵ "Windows/Watchdog.inc"
#endif
