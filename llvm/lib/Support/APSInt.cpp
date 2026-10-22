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

#شامل "llvm/ADT/APSInt.h"
#شامل "llvm/ADT/FoldingSet.h"
#شامل "llvm/ADT/StringRef.h"
#شامل <cassert>

using فضاینام llvm;

APSInt::APSInt(StringRef Str) {
  أكد(!Str.empty() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  unsigned رقم_Bits_2 = ((Str.size() * 64) / 19) + 2;
  APInt Tmp(رقم_Bits_2, Str, /*radix=*/10);
  اگر (Str[0] == '-') {
    unsigned MinBits = Tmp.getSignificantBits();
    اگر (MinBits < رقم_Bits_2)
      Tmp = Tmp.trunc(std::max<unsigned>(1, MinBits));
    *این = APSInt(Tmp, /*isUnsigned=*/نادرست);
    بازگشت;
  }
  unsigned ActiveBits = Tmp.getActiveBits();
  اگر (ActiveBits < رقم_Bits_2)
    Tmp = Tmp.trunc(std::max<unsigned>(1, ActiveBits));
  *این = APSInt(Tmp, /*isUnsigned=*/درست);
}

تهی APSInt::Profile(Folding_Set_گره_ID& ID) ثابت {
  ID.AddInteger((unsigned) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
