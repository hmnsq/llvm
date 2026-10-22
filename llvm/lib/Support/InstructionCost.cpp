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

#અન્તર્ભાવ "llvm/Support/InstructionCost.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"

using નામાકાશ llvm;

શૂન્ય InstructionCost::print(raw_ostream &OS) અચલ {
  using UnsignedCostType = std::make_unsigned_t<CostType>;
  જો (isValid()) {
    UnsignedCostType Abs_કિંમત =
        (કિંમત < 0) ? -((UnsignedCostType)કિંમત) : ((UnsignedCostType)કિંમત);
    UnsignedCostType Whole_સંખ્યા = Abs_કિંમત / CostGranularity;
    UnsignedCostType Remainder = Abs_કિંમત % CostGranularity;
    જો (કિંમત < 0)
      OS << "-";
    UnsignedCostType RemainderHundreds = (Remainder * 100) / CostGranularity;
    જ્યારે (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << Whole_સંખ્યા;
    જો (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } નહીંતર {
    OS << "Invalid";
  }
}
