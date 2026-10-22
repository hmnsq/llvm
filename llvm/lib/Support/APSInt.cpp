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

उपयुज्य नामाकाश llvm;

APSInt::APSInt(StringRef Str) {
  प्रतिज्ञा(!Str.रिक्तानि() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  अचिह्नित अङ्कः_द्वयङ्काः_2 = ((Str.आकार_2() * 64) / 19) + 2;
  APInt Tmp(अङ्कः_द्वयङ्काः_2, Str, /*radix=*/10);
  यदि (Str[0] == '-') {
    अचिह्नित Min_द्वयङ्काः = Tmp.get_Significant_द्वयङ्काः();
    यदि (Min_द्वयङ्काः < अङ्कः_द्वयङ्काः_2)
      Tmp = Tmp.trunc(std::max<अचिह्नित>(1, Min_द्वयङ्काः));
    *एतत् = APSInt(Tmp, /*isUnsigned=*/असत्य);
    प्रत्यागम;
  }
  अचिह्नित सक्रिय_द्वयङ्काः = Tmp.get_सक्रिय_द्वयङ्काः();
  यदि (सक्रिय_द्वयङ्काः < अङ्कः_द्वयङ्काः_2)
    Tmp = Tmp.trunc(std::max<अचिह्नित>(1, सक्रिय_द्वयङ्काः));
  *एतत् = APSInt(Tmp, /*isUnsigned=*/सत्य);
}

शून्य APSInt::Profile(Folding_समुच्चयः_ग्रन्थिः_ID& ID) ध्रुव {
  ID.सङ्कलय_Integer((अचिह्नित) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
