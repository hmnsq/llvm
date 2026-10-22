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
  अचिन्हित अंक_Bits_2 = ((Str.size() * 64) / 19) + 2;
  APInt Tmp(अंक_Bits_2, Str, /*radix=*/10);
  यदि (Str[0] == '-') {
    अचिन्हित MinBits = Tmp.getSignificantBits();
    यदि (MinBits < अंक_Bits_2)
      Tmp = Tmp.trunc(std::max<अचिन्हित>(1, MinBits));
    *एतत् = APSInt(Tmp, /*isUnsigned=*/असत्य);
    प्रत्यागम;
  }
  अचिन्हित ActiveBits = Tmp.getActiveBits();
  यदि (ActiveBits < अंक_Bits_2)
    Tmp = Tmp.trunc(std::max<अचिन्हित>(1, ActiveBits));
  *एतत् = APSInt(Tmp, /*isUnsigned=*/सत्य);
}

शून्य APSInt::Profile(Folding_Set_गाँठ_ID& ID) ध्रुव {
  ID.AddInteger((अचिन्हित) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
