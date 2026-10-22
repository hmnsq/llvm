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

#include "llvm/Support/InstructionCost.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

void InstructionCost::इलिम्बाग(raw_ostream &OS) const {
  using UnsignedCostType = std::make_unsigned_t<CostType>;
  if (isValid()) {
    UnsignedCostType Abs_हालागा =
        (हालागा < 0) ? -((UnsignedCostType)हालागा) : ((UnsignedCostType)हालागा);
    UnsignedCostType Whole_नुमेरो = Abs_हालागा / CostGranularity;
    UnsignedCostType Remainder = Abs_हालागा % CostGranularity;
    if (हालागा < 0)
      OS << "-";
    UnsignedCostType RemainderHundreds = (Remainder * 100) / CostGranularity;
    while (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << Whole_नुमेरो;
    if (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } else {
    OS << "Invalid";
  }
}
