//===- SystemUtils.cpp - Utilities for low-level system tasks -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains functions used to do a variety of low-level, often
// system-specific, tasks.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/SystemUtils.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ತಾರ್ಕಿಕ llvm::Check_Bitcode_Output_ಗಮ್ಯ_Console(raw_ostream &ಹರಿವು_ಗಮ್ಯ_check) {
  ಆದರೆ (ಹರಿವು_ಗಮ್ಯ_check.is_displayed()) {
    errs() << "WARNING: You're attempting to print out a bitcode file.\n"
              "This is inadvisable as it may cause display problems. If\n"
              "you REALLY want to taste LLVM bitcode first-hand, you\n"
              "can force output with the `-f' option.\n\n";
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}
