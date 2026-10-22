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

#ضمن "llvm/Support/InstructionCost.h"
#ضمن "llvm/Support/raw_ostream.h"

using نطاق llvm;

فراغ InstructionCost::اطبع(raw_ostream &OS) ثابت {
  using UnsignedCostType = std::make_unsigned_t<CostType>;
  إذا (isValid()) {
    UnsignedCostType Abs_قيمة =
        (قيمة < 0) ? -((UnsignedCostType)قيمة) : ((UnsignedCostType)قيمة);
    UnsignedCostType Whole_رقم = Abs_قيمة / CostGranularity;
    UnsignedCostType Remainder = Abs_قيمة % CostGranularity;
    إذا (قيمة < 0)
      OS << "-";
    UnsignedCostType RemainderHundreds = (Remainder * 100) / CostGranularity;
    طالما (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << Whole_رقم;
    إذا (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } وإلا {
    OS << "Invalid";
  }
}
