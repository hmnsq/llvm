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

#شامل "llvm/Support/InstructionCost.h"
#شامل "llvm/Support/raw_ostream.h"

استعمال نامخلا llvm;

خالی Instruction_لاگت::print(raw_ostream &OS) ثابت {
  استعمال Unsigned_لاگت_Type = std::make_unsigned_t<لاگت_Type>;
  اگر (isValid()) {
    Unsigned_لاگت_Type Abs_قدر =
        (قدر < 0) ? -((Unsigned_لاگت_Type)قدر) : ((Unsigned_لاگت_Type)قدر);
    Unsigned_لاگت_Type پورا_نمبر = Abs_قدر / لاگت_Granularity;
    Unsigned_لاگت_Type Remainder = Abs_قدر % لاگت_Granularity;
    اگر (قدر < 0)
      OS << "-";
    Unsigned_لاگت_Type RemainderHundreds = (Remainder * 100) / لاگت_Granularity;
    جب (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << پورا_نمبر;
    اگر (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } ورنہ {
    OS << "Invalid";
  }
}
