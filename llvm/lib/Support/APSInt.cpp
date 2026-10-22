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

#অন্তর্ভাৱ "llvm/ADT/APSInt.h"
#অন্তর্ভাৱ "llvm/ADT/FoldingSet.h"
#অন্তর্ভাৱ "llvm/ADT/StringRef.h"
#অন্তর্ভাৱ <cassert>

using নামাকাশ llvm;

APSInt::APSInt(StringRef Str) {
  প্রতিজ্ঞা(!Str.empty() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  unsigned অঙ্ক_Bits_2 = ((Str.size() * 64) / 19) + 2;
  APInt Tmp(অঙ্ক_Bits_2, Str, /*radix=*/10);
  যদি (Str[0] == '-') {
    unsigned MinBits = Tmp.getSignificantBits();
    যদি (MinBits < অঙ্ক_Bits_2)
      Tmp = Tmp.trunc(std::max<unsigned>(1, MinBits));
    *এতত্ = APSInt(Tmp, /*isUnsigned=*/মিছা);
    ঘূৰাই;
  }
  unsigned ActiveBits = Tmp.getActiveBits();
  যদি (ActiveBits < অঙ্ক_Bits_2)
    Tmp = Tmp.trunc(std::max<unsigned>(1, ActiveBits));
  *এতত্ = APSInt(Tmp, /*isUnsigned=*/সঁচা);
}

শূন্য APSInt::Profile(Folding_Set_গ্ৰন্থি_ID& ID) ধ্রুৱ {
  ID.AddInteger((unsigned) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
