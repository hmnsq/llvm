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

#अन्तर्भाव "llvm/ADT/APSInt.h"
#अन्तर्भाव "llvm/ADT/FoldingSet.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव <cassert>

using नामाकाश llvm;

APSInt::APSInt(StringRef Str) {
  प्रतिज्ञा(!Str.empty() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  unsigned अङ्कः_Bits_2 = ((Str.size() * 64) / 19) + 2;
  APInt Tmp(अङ्कः_Bits_2, Str, /*radix=*/10);
  जइ (Str[0] == '-') {
    unsigned MinBits = Tmp.getSignificantBits();
    जइ (MinBits < अङ्कः_Bits_2)
      Tmp = Tmp.trunc(std::max<unsigned>(1, MinBits));
    *एतत् = APSInt(Tmp, /*isUnsigned=*/मिच्छ);
    प्रत्यागम;
  }
  unsigned ActiveBits = Tmp.getActiveBits();
  जइ (ActiveBits < अङ्कः_Bits_2)
    Tmp = Tmp.trunc(std::max<unsigned>(1, ActiveBits));
  *एतत् = APSInt(Tmp, /*isUnsigned=*/सच्च);
}

शून्य APSInt::Profile(Folding_Set_ग्रन्थिः_ID& ID) ध्रुव {
  ID.AddInteger((unsigned) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
