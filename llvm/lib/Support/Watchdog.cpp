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

#అన్తర్భావ "llvm/Support/Watchdog.h"
#అన్తర్భావ "llvm/Config/llvm-config.h"

// Include the platform-specific parts of this class.
#నిర్వచితమైతే LLVM_ON_UNIX
#అన్తర్భావ "Unix/Watchdog.inc"
#అయితే_ముగింపు
#నిర్వచితమైతే _WIN32
#అన్తర్భావ "Windows/Watchdog.inc"
#అయితే_ముగింపు
