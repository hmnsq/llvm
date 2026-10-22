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
  जर (Str[0] == '-') {
    अचिन्हित MinBits = Tmp.getSignificantBits();
    जर (MinBits < अंक_Bits_2)
      Tmp = Tmp.trunc(std::max<अचिन्हित>(1, MinBits));
    *हे = APSInt(Tmp, /*isUnsigned=*/असत्य);
    परत;
  }
  अचिन्हित ActiveBits = Tmp.getActiveBits();
  जर (ActiveBits < अंक_Bits_2)
    Tmp = Tmp.trunc(std::max<अचिन्हित>(1, ActiveBits));
  *हे = APSInt(Tmp, /*isUnsigned=*/सत्य);
}

शून्य APSInt::Profile(Folding_Set_गाठ_ID& ID) स्थिरांक {
  ID.AddInteger((अचिन्हित) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
