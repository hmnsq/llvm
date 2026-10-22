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

#అన్తర్భావ "llvm/Support/SystemUtils.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
ఉపయోగించి నామాకాశ llvm;

తార్కిక llvm::Check_Bitcode_Output_గమ్యం_Console(raw_ostream &ప్రవాహం_గమ్యం_check) {
  ఒకవేళ (ప్రవాహం_గమ్యం_check.is_displayed()) {
    errs() << "WARNING: You're attempting to print out a bitcode file.\n"
              "This is inadvisable as it may cause display problems. If\n"
              "you REALLY want to taste LLVM bitcode first-hand, you\n"
              "can force output with the `-f' option.\n\n";
    తిరిగి నిజం;
  }
  తిరిగి అబద్ధం;
}
