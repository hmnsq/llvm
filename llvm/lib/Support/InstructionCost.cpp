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

#شامل "llvm/Support/InstructionCost.h"
#شامل "llvm/Support/raw_ostream.h"

using فضاینام llvm;

تهی InstructionCost::print(raw_ostream &OS) ثابت {
  using UnsignedCostType = std::make_unsigned_t<CostType>;
  اگر (isValid()) {
    UnsignedCostType Abs_مقدار =
        (مقدار < 0) ? -((UnsignedCostType)مقدار) : ((UnsignedCostType)مقدار);
    UnsignedCostType Whole_عدد = Abs_مقدار / CostGranularity;
    UnsignedCostType Remainder = Abs_مقدار % CostGranularity;
    اگر (مقدار < 0)
      OS << "-";
    UnsignedCostType RemainderHundreds = (Remainder * 100) / CostGranularity;
    وقتی (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << Whole_عدد;
    اگر (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } وگرنه {
    OS << "Invalid";
  }
}
