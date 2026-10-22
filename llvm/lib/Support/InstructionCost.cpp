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

#అన్తర్భావ "llvm/Support/InstructionCost.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"

ఉపయోగించి నామాకాశ llvm;

శూన్య Instruction_ఖర్చు::print(raw_ostream &OS) స్థిరరాశి {
  ఉపయోగించి Unsigned_ఖర్చు_Type = std::make_unsigned_t<ఖర్చు_Type>;
  ఒకవేళ (isValid()) {
    Unsigned_ఖర్చు_Type Abs_విలువ =
        (విలువ < 0) ? -((Unsigned_ఖర్చు_Type)విలువ) : ((Unsigned_ఖర్చు_Type)విలువ);
    Unsigned_ఖర్చు_Type సంపూర్ణ_సంఖ్య = Abs_విలువ / ఖర్చు_Granularity;
    Unsigned_ఖర్చు_Type Remainder = Abs_విలువ % ఖర్చు_Granularity;
    ఒకవేళ (విలువ < 0)
      OS << "-";
    Unsigned_ఖర్చు_Type RemainderHundreds = (Remainder * 100) / ఖర్చు_Granularity;
    అయితే (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << సంపూర్ణ_సంఖ్య;
    ఒకవేళ (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } లేకపోతే {
    OS << "Invalid";
  }
}
