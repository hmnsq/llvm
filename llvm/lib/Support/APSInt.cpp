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

#அந்தர்பாவ "llvm/ADT/APSInt.h"
#அந்தர்பாவ "llvm/ADT/FoldingSet.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ <cassert>

using நாமாகாஶ llvm;

APSInt::APSInt(StringRef Str) {
  ப்ரதிஜ்ஞா(!Str.empty() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  unsigned இலக்கம்_Bits_2 = ((Str.size() * 64) / 19) + 2;
  APInt Tmp(இலக்கம்_Bits_2, Str, /*radix=*/10);
  எனில் (Str[0] == '-') {
    unsigned MinBits = Tmp.getSignificantBits();
    எனில் (MinBits < இலக்கம்_Bits_2)
      Tmp = Tmp.trunc(std::max<unsigned>(1, MinBits));
    *இது = APSInt(Tmp, /*isUnsigned=*/பொய்);
    திரும்பு;
  }
  unsigned ActiveBits = Tmp.getActiveBits();
  எனில் (ActiveBits < இலக்கம்_Bits_2)
    Tmp = Tmp.trunc(std::max<unsigned>(1, ActiveBits));
  *இது = APSInt(Tmp, /*isUnsigned=*/உண்மை);
}

வெறுமை APSInt::Profile(Folding_Set_முனை_ID& ID) மாறிலி {
  ID.AddInteger((unsigned) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
