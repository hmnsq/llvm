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

#ಅನ್ತರ್ಭಾವ "llvm/Support/InstructionCost.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"

using ನಾಮಾಕಾಶ llvm;

ಶೂನ್ಯ InstructionCost::print(raw_ostream &OS) ಧ್ರುವ {
  using UnsignedCostType = std::make_unsigned_t<CostType>;
  ಯದಿ (isValid()) {
    UnsignedCostType Abs_ಮೌಲ್ಯ =
        (ಮೌಲ್ಯ < 0) ? -((UnsignedCostType)ಮೌಲ್ಯ) : ((UnsignedCostType)ಮೌಲ್ಯ);
    UnsignedCostType Whole_ಸಂಖ್ಯೆ = Abs_ಮೌಲ್ಯ / CostGranularity;
    UnsignedCostType Remainder = Abs_ಮೌಲ್ಯ % CostGranularity;
    ಯದಿ (ಮೌಲ್ಯ < 0)
      OS << "-";
    UnsignedCostType RemainderHundreds = (Remainder * 100) / CostGranularity;
    ಯಾವತ್ (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << Whole_ಸಂಖ್ಯೆ;
    ಯದಿ (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } ಅನ್ಯಥಾ {
    OS << "Invalid";
  }
}
