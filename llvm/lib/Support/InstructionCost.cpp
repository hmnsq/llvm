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

استفاده نطاق llvm;

فراغ Instruction_هزینه::print(raw_ostream &OS) ثابت {
  استفاده Unsigned_هزینه_Type = std::make_unsigned_t<هزینه_Type>;
  إذا (isValid()) {
    Unsigned_هزینه_Type Abs_مقدار =
        (مقدار < 0) ? -((Unsigned_هزینه_Type)مقدار) : ((Unsigned_هزینه_Type)مقدار);
    Unsigned_هزینه_Type کامل_عدد = Abs_مقدار / هزینه_Granularity;
    Unsigned_هزینه_Type Remainder = Abs_مقدار % هزینه_Granularity;
    إذا (مقدار < 0)
      OS << "-";
    Unsigned_هزینه_Type RemainderHundreds = (Remainder * 100) / هزینه_Granularity;
    طالما (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << کامل_عدد;
    إذا (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } وإلا {
    OS << "Invalid";
  }
}
