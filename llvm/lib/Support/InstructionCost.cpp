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

#अन्तर्भाव "llvm/Support/InstructionCost.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

using नामाकाश llvm;

शून्य InstructionCost::print(raw_ostream &OS) ध्रुव {
  using UnsignedCostType = std::make_unsigned_t<CostType>;
  जइ (isValid()) {
    UnsignedCostType Abs_मानम् =
        (मानम् < 0) ? -((UnsignedCostType)मानम्) : ((UnsignedCostType)मानम्);
    UnsignedCostType Whole_सङ्ख्या = Abs_मानम् / CostGranularity;
    UnsignedCostType Remainder = Abs_मानम् % CostGranularity;
    जइ (मानम् < 0)
      OS << "-";
    UnsignedCostType RemainderHundreds = (Remainder * 100) / CostGranularity;
    जाव (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << Whole_सङ्ख्या;
    जइ (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } अण्णहा {
    OS << "Invalid";
  }
}
