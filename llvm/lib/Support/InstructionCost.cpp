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

शून्य InstructionCost::print(raw_ostream &OS) अचर {
  using UnsignedCostType = std::make_unsigned_t<CostType>;
  यदि (isValid()) {
    UnsignedCostType Abs_मान =
        (मान < 0) ? -((UnsignedCostType)मान) : ((UnsignedCostType)मान);
    UnsignedCostType Whole_सङ्ख्या = Abs_मान / CostGranularity;
    UnsignedCostType Remainder = Abs_मान % CostGranularity;
    यदि (मान < 0)
      OS << "-";
    UnsignedCostType RemainderHundreds = (Remainder * 100) / CostGranularity;
    जबसम्म (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << Whole_सङ्ख्या;
    यदि (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } अन्यथा {
    OS << "Invalid";
  }
}
