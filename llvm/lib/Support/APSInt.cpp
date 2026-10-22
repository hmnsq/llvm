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

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/APSInt.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/FoldingSet.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ <cassert>

using ନାମାକାଶ llvm;

APSInt::APSInt(StringRef Str) {
  ପ୍ରତିଜ୍ଞା(!Str.empty() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  unsigned ଅଙ୍କ_Bits_2 = ((Str.size() * 64) / 19) + 2;
  APInt Tmp(ଅଙ୍କ_Bits_2, Str, /*radix=*/10);
  ଯଦି (Str[0] == '-') {
    unsigned MinBits = Tmp.getSignificantBits();
    ଯଦି (MinBits < ଅଙ୍କ_Bits_2)
      Tmp = Tmp.trunc(std::max<unsigned>(1, MinBits));
    *ଏହି = APSInt(Tmp, /*isUnsigned=*/ମିଥ୍ୟା);
    ଫେରନ୍ତୁ;
  }
  unsigned ActiveBits = Tmp.getActiveBits();
  ଯଦି (ActiveBits < ଅଙ୍କ_Bits_2)
    Tmp = Tmp.trunc(std::max<unsigned>(1, ActiveBits));
  *ଏହି = APSInt(Tmp, /*isUnsigned=*/ସତ୍ୟ);
}

ଶୂନ୍ୟ APSInt::Profile(Folding_Set_ଗଣ୍ଠି_ID& ID) ଅଚଳ {
  ID.AddInteger((unsigned) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
