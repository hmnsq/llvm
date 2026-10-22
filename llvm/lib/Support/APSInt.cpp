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

using ನಾಮಾಕಾಶ llvm;

APSInt::APSInt(StringRef Str) {
  ಪ್ರತಿಜ್ಞಾ(!Str.empty() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  unsigned ಅಂಕೆ_Bits_2 = ((Str.size() * 64) / 19) + 2;
  APInt Tmp(ಅಂಕೆ_Bits_2, Str, /*radix=*/10);
  ಯದಿ (Str[0] == '-') {
    unsigned MinBits = Tmp.getSignificantBits();
    ಯದಿ (MinBits < ಅಂಕೆ_Bits_2)
      Tmp = Tmp.trunc(std::max<unsigned>(1, MinBits));
    *ಏತತ್ = APSInt(Tmp, /*isUnsigned=*/ಅಸತ್ಯ);
    ಪ್ರತ್ಯಾಗಮ;
  }
  unsigned ActiveBits = Tmp.getActiveBits();
  ಯದಿ (ActiveBits < ಅಂಕೆ_Bits_2)
    Tmp = Tmp.trunc(std::max<unsigned>(1, ActiveBits));
  *ಏತತ್ = APSInt(Tmp, /*isUnsigned=*/ಸತ್ಯ);
}

ಶೂನ್ಯ APSInt::Profile(Folding_Set_ಗಂಟು_ID& ID) ಧ್ರುವ {
  ID.AddInteger((unsigned) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
