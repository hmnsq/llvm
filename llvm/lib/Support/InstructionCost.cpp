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

فراغ InstructionCost::print(raw_ostream &OS) ثابت {
  using UnsignedCostType = std::make_unsigned_t<CostType>;
  إذا (isValid()) {
    UnsignedCostType Abs_بەها =
        (بەها < 0) ? -((UnsignedCostType)بەها) : ((UnsignedCostType)بەها);
    UnsignedCostType Whole_ژمار = Abs_بەها / CostGranularity;
    UnsignedCostType Remainder = Abs_بەها % CostGranularity;
    إذا (بەها < 0)
      OS << "-";
    UnsignedCostType RemainderHundreds = (Remainder * 100) / CostGranularity;
    طالما (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << Whole_ژمار;
    إذا (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } وإلا {
    OS << "Invalid";
  }
}
