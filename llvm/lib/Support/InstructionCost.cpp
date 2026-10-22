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

#ضمن "llvm/Support/InstructionCost.h"
#ضمن "llvm/Support/raw_ostream.h"

استعمال نطاق llvm;

فراغ Instruction_لاڳت::print(raw_ostream &OS) ثابت {
  استعمال Unsigned_لاڳت_Type = std::make_unsigned_t<لاڳت_Type>;
  إذا (isValid()) {
    Unsigned_لاڳت_Type Abs_قدر =
        (قدر < 0) ? -((Unsigned_لاڳت_Type)قدر) : ((Unsigned_لاڳت_Type)قدر);
    Unsigned_لاڳت_Type سڄو_عدد = Abs_قدر / لاڳت_Granularity;
    Unsigned_لاڳت_Type Remainder = Abs_قدر % لاڳت_Granularity;
    إذا (قدر < 0)
      OS << "-";
    Unsigned_لاڳت_Type RemainderHundreds = (Remainder * 100) / لاڳت_Granularity;
    طالما (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << سڄو_عدد;
    إذا (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } وإلا {
    OS << "Invalid";
  }
}
