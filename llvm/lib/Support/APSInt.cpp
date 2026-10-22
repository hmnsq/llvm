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

#ضمن "llvm/ADT/APSInt.h"
#ضمن "llvm/ADT/FoldingSet.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن <cassert>

using نطاق llvm;

APSInt::APSInt(StringRef Str) {
  أكد(!Str.empty() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  unsigned NumBits = ((Str.size() * 64) / 19) + 2;
  APInt Tmp(NumBits, Str, /*radix=*/10);
  𐤀𐤌 (Str[0] == '-') {
    unsigned MinBits = Tmp.getSignificantBits();
    𐤀𐤌 (MinBits < NumBits)
      Tmp = Tmp.trunc(std::max<unsigned>(1, MinBits));
    *هذا = APSInt(Tmp, /*isUnsigned=*/خطأ);
    أرجع;
  }
  unsigned ActiveBits = Tmp.getActiveBits();
  𐤀𐤌 (ActiveBits < NumBits)
    Tmp = Tmp.trunc(std::max<unsigned>(1, ActiveBits));
  *هذا = APSInt(Tmp, /*isUnsigned=*/true);
}

فراغ APSInt::Profile(FoldingSetNodeID& ID) ثابت {
  ID.AddInteger((unsigned) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
