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

#അന്തര്ഭാവ "llvm/ADT/APSInt.h"
#അന്തര്ഭാവ "llvm/ADT/FoldingSet.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ <cassert>

using നാമാകാശ llvm;

APSInt::APSInt(StringRef Str) {
  പ്രതിജ്ഞാ(!Str.empty() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  unsigned അക്കം_Bits_2 = ((Str.size() * 64) / 19) + 2;
  APInt Tmp(അക്കം_Bits_2, Str, /*radix=*/10);
  എങ്കിൽ (Str[0] == '-') {
    unsigned MinBits = Tmp.getSignificantBits();
    എങ്കിൽ (MinBits < അക്കം_Bits_2)
      Tmp = Tmp.trunc(std::max<unsigned>(1, MinBits));
    *ഇത് = APSInt(Tmp, /*isUnsigned=*/അസത്യം);
    തിരികെ;
  }
  unsigned ActiveBits = Tmp.getActiveBits();
  എങ്കിൽ (ActiveBits < അക്കം_Bits_2)
    Tmp = Tmp.trunc(std::max<unsigned>(1, ActiveBits));
  *ഇത് = APSInt(Tmp, /*isUnsigned=*/സത്യം);
}

ശൂന്യം APSInt::Profile(Folding_Set_കണ്ണി_ID& ID) സ്ഥിരം {
  ID.AddInteger((unsigned) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
