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

#অন্তর্ভাব "llvm/Support/SystemUtils.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
using নামাকাশ llvm;

তার্কিক llvm::CheckBitcodeOutputToConsole(raw_ostream &stream_to_check) {
  যদি (stream_to_check.is_displayed()) {
    errs() << "WARNING: You're attempting to print out a bitcode file.\n"
              "This is inadvisable as it may cause display problems. If\n"
              "you REALLY want to taste LLVM bitcode first-hand, you\n"
              "can force output with the `-f' option.\n\n";
    প্রত্যাগম সত্য;
  }
  প্রত্যাগম অসত্য;
}
