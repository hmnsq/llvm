//===- InstructionCost.cpp --------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file includes the function definitions for the InstructionCost class
/// that is used when calculating the cost of an instruction, or a group of
/// instructions.
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/InstructionCost.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"

using নামাকাশ llvm;

শূন্য InstructionCost::print(raw_ostream &OS) ধ্রুবক {
  using UnsignedCostType = std::make_unsigned_t<CostType>;
  যদি (isValid()) {
    UnsignedCostType Abs_মান =
        (মান < 0) ? -((UnsignedCostType)মান) : ((UnsignedCostType)মান);
    UnsignedCostType Whole_সংখ্যা = Abs_মান / CostGranularity;
    UnsignedCostType Remainder = Abs_মান % CostGranularity;
    যদি (মান < 0)
      OS << "-";
    UnsignedCostType RemainderHundreds = (Remainder * 100) / CostGranularity;
    যখন (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << Whole_সংখ্যা;
    যদি (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } অন্যথা {
    OS << "Invalid";
  }
}
