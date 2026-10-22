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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/InstructionCost.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਖਾਲੀ Instruction_ਲਾਗਤ::print(raw_ostream &OS) ਅਚਲ {
  ਵਰਤੋਂ Unsigned_ਲਾਗਤ_Type = std::make_unsigned_t<ਲਾਗਤ_Type>;
  ਜੇ (isValid()) {
    Unsigned_ਲਾਗਤ_Type Abs_ਮੁੱਲ =
        (ਮੁੱਲ < 0) ? -((Unsigned_ਲਾਗਤ_Type)ਮੁੱਲ) : ((Unsigned_ਲਾਗਤ_Type)ਮੁੱਲ);
    Unsigned_ਲਾਗਤ_Type ਸਾਰਾ_ਸੰਖਿਆ = Abs_ਮੁੱਲ / ਲਾਗਤ_Granularity;
    Unsigned_ਲਾਗਤ_Type Remainder = Abs_ਮੁੱਲ % ਲਾਗਤ_Granularity;
    ਜੇ (ਮੁੱਲ < 0)
      OS << "-";
    Unsigned_ਲਾਗਤ_Type RemainderHundreds = (Remainder * 100) / ਲਾਗਤ_Granularity;
    ਜਦੋਂ (RemainderHundreds % 10 == 0 && RemainderHundreds)
      RemainderHundreds /= 10;
    OS << ਸਾਰਾ_ਸੰਖਿਆ;
    ਜੇ (RemainderHundreds)
      OS << "." << RemainderHundreds;
  } ਨਹੀਂਤਾਂ {
    OS << "Invalid";
  }
}
