//===-- llvm/ADT/APSInt.cpp - Arbitrary Precision Signed Int ---*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the APSInt class, which is a simple class that
// represents an arbitrary sized integer that knows its signedness.
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/ADT/APSInt.h"
#અન્તર્ભાવ "llvm/ADT/FoldingSet.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ <cassert>

using નામાકાશ llvm;

APSInt::APSInt(StringRef Str) {
  પ્રતિજ્ઞા(!Str.empty() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  unsigned અંક_Bits_2 = ((Str.size() * 64) / 19) + 2;
  APInt Tmp(અંક_Bits_2, Str, /*radix=*/10);
  જો (Str[0] == '-') {
    unsigned MinBits = Tmp.getSignificantBits();
    જો (MinBits < અંક_Bits_2)
      Tmp = Tmp.trunc(std::max<unsigned>(1, MinBits));
    *આ = APSInt(Tmp, /*isUnsigned=*/ખોટું);
    પાછા;
  }
  unsigned ActiveBits = Tmp.getActiveBits();
  જો (ActiveBits < અંક_Bits_2)
    Tmp = Tmp.trunc(std::max<unsigned>(1, ActiveBits));
  *આ = APSInt(Tmp, /*isUnsigned=*/સાચું);
}

શૂન્ય APSInt::Profile(Folding_Set_ગાંઠ_ID& ID) અચલ {
  ID.AddInteger((unsigned) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
