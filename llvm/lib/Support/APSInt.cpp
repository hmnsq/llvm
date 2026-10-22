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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/APSInt.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/FoldingSet.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ <cassert>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

APSInt::APSInt(StringRef Str) {
  ಪ್ರತಿಜ್ಞಾ(!Str.ಖಾಲಿಗಳು() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2 = ((Str.ಗಾತ್ರ_3() * 64) / 19) + 2;
  APInt Tmp(ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2, Str, /*radix=*/10);
  ಆದರೆ (Str[0] == '-') {
    ಚಿಹ್ನೆರಹಿತ Min_ಬಿಟ್ಟುಗಳು = Tmp.get_Significant_ಬಿಟ್ಟುಗಳು();
    ಆದರೆ (Min_ಬಿಟ್ಟುಗಳು < ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2)
      Tmp = Tmp.trunc(std::max<ಚಿಹ್ನೆರಹಿತ>(1, Min_ಬಿಟ್ಟುಗಳು));
    *ಇದು = APSInt(Tmp, /*isUnsigned=*/ಸುಳ್ಳು);
    ಹಿಂತಿರುಗಿ;
  }
  ಚಿಹ್ನೆರಹಿತ ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು = Tmp.get_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು();
  ಆದರೆ (ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು < ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2)
    Tmp = Tmp.trunc(std::max<ಚಿಹ್ನೆರಹಿತ>(1, ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು));
  *ಇದು = APSInt(Tmp, /*isUnsigned=*/ನಿಜ);
}

ಶೂನ್ಯ APSInt::Profile(Folding_ಗಣ_ಗಂಟು_ID& ID) ಸ್ಥಿರಾಂಕ {
  ID.ಕೂಡಿಸು_Integer((ಚಿಹ್ನೆರಹಿತ) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
