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

#समावेश "llvm/Support/InstructionCost.h"
#समावेश "llvm/Support/raw_ostream.h"

using namespace llvm;

व्योम InstructionCost::print(raw_ostream &OS) स्थिर {
  using UnsignedCostType = std::make_unsigned_t<CostType>;
  अगर (isValid()) {
    UnsignedCostType Abs_मूल्य =
        (मूल्य < 0) ? -((UnsignedCostType)मूल्य) : ((UnsignedCostType)मूल्य);
    UnsignedCostType Whole_संख्या = Abs_मूल्य / CostGranularity;
    UnsignedCostType Remainder = Abs_मूल्य % CostGranularity;
    अगर (मूल्य < 0)
      OS << "-";
    UnsignedCostType RemainderHundreds = (Remainder * 100) / CostGranularity;
    जबतक (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << Whole_संख्या;
    अगर (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } अन्यथा {
    OS << "Invalid";
  }
}
