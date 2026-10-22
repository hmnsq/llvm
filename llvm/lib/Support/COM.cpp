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

#అన్తర్భావ "llvm/Support/COM.h"

#అన్తర్భావ "llvm/Config/llvm-config.h"

// Include the platform-specific parts of this class.
#నిర్వచితమైతే LLVM_ON_UNIX
#అన్తర్భావ "Unix/COM.inc"
#లేదావేళ defined(_WIN32)
#అన్తర్భావ "Windows/COM.inc"
#అయితే_ముగింపు
