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

کارول نطاق llvm;

فراغ Instruction_لګښت::print(raw_ostream &OS) ثابت {
  کارول Unsigned_لګښت_Type = std::make_unsigned_t<لګښت_Type>;
  إذا (isValid()) {
    Unsigned_لګښت_Type Abs_ارزښت =
        (ارزښت < 0) ? -((Unsigned_لګښت_Type)ارزښت) : ((Unsigned_لګښت_Type)ارزښت);
    Unsigned_لګښت_Type ټولیز_شمېره = Abs_ارزښت / لګښت_Granularity;
    Unsigned_لګښت_Type Remainder = Abs_ارزښت % لګښت_Granularity;
    إذا (ارزښت < 0)
      OS << "-";
    Unsigned_لګښت_Type RemainderHundreds = (Remainder * 100) / لګښت_Granularity;
    طالما (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << ټولیز_شمېره;
    إذا (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } وإلا {
    OS << "Invalid";
  }
}
