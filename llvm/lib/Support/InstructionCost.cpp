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

#അന്തര്ഭാവ "llvm/Support/InstructionCost.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"

using നാമാകാശ llvm;

ശൂന്യം InstructionCost::print(raw_ostream &OS) സ്ഥിരം {
  using UnsignedCostType = std::make_unsigned_t<CostType>;
  എങ്കിൽ (isValid()) {
    UnsignedCostType Abs_മൂല്യം =
        (മൂല്യം < 0) ? -((UnsignedCostType)മൂല്യം) : ((UnsignedCostType)മൂല്യം);
    UnsignedCostType Whole_സംഖ്യ = Abs_മൂല്യം / CostGranularity;
    UnsignedCostType Remainder = Abs_മൂല്യം % CostGranularity;
    എങ്കിൽ (മൂല്യം < 0)
      OS << "-";
    UnsignedCostType RemainderHundreds = (Remainder * 100) / CostGranularity;
    ആകുമ്പോൾ (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << Whole_സംഖ്യ;
    എങ്കിൽ (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } അല്ലെങ്കിൽ {
    OS << "Invalid";
  }
}
