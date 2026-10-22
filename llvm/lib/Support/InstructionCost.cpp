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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/InstructionCost.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"

using ନାମାକାଶ llvm;

ଶୂନ୍ୟ InstructionCost::print(raw_ostream &OS) ଅଚଳ {
  using UnsignedCostType = std::make_unsigned_t<CostType>;
  ଯଦି (isValid()) {
    UnsignedCostType Abs_ମୂଲ୍ୟ =
        (ମୂଲ୍ୟ < 0) ? -((UnsignedCostType)ମୂଲ୍ୟ) : ((UnsignedCostType)ମୂଲ୍ୟ);
    UnsignedCostType Whole_ସଂଖ୍ୟା = Abs_ମୂଲ୍ୟ / CostGranularity;
    UnsignedCostType Remainder = Abs_ମୂଲ୍ୟ % CostGranularity;
    ଯଦି (ମୂଲ୍ୟ < 0)
      OS << "-";
    UnsignedCostType RemainderHundreds = (Remainder * 100) / CostGranularity;
    ଯେତେବେଳେ (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << Whole_ସଂଖ୍ୟା;
    ଯଦି (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } ଅନ୍ୟଥା {
    OS << "Invalid";
  }
}
