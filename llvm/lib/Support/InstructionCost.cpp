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

#அந்தர்பாவ "llvm/Support/InstructionCost.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"

using நாமாகாஶ llvm;

வெறுமை InstructionCost::print(raw_ostream &OS) மாறிலி {
  using UnsignedCostType = std::make_unsigned_t<CostType>;
  எனில் (isValid()) {
    UnsignedCostType Abs_மதிப்பு =
        (மதிப்பு < 0) ? -((UnsignedCostType)மதிப்பு) : ((UnsignedCostType)மதிப்பு);
    UnsignedCostType Whole_எண் = Abs_மதிப்பு / CostGranularity;
    UnsignedCostType Remainder = Abs_மதிப்பு % CostGranularity;
    எனில் (மதிப்பு < 0)
      OS << "-";
    UnsignedCostType RemainderHundreds = (Remainder * 100) / CostGranularity;
    போது (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << Whole_எண்;
    எனில் (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } இல்லையெனில் {
    OS << "Invalid";
  }
}
