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

#समावेश "llvm/ADT/APSInt.h"
#समावेश "llvm/ADT/FoldingSet.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश <cassert>

using namespace llvm;

APSInt::APSInt(StringRef Str) {
  निश्चित(!Str.empty() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  अचिन्हित अंक_Bits_2 = ((Str.आकार_2() * 64) / 19) + 2;
  APInt Tmp(अंक_Bits_2, Str, /*radix=*/10);
  अगर (Str[0] == '-') {
    अचिन्हित MinBits = Tmp.getSignificantBits();
    अगर (MinBits < अंक_Bits_2)
      Tmp = Tmp.छाँटो(std::max<अचिन्हित>(1, MinBits));
    *यह = APSInt(Tmp, /*isUnsigned=*/false);
    वापस;
  }
  अचिन्हित ActiveBits = Tmp.getActiveBits();
  अगर (ActiveBits < अंक_Bits_2)
    Tmp = Tmp.छाँटो(std::max<अचिन्हित>(1, ActiveBits));
  *यह = APSInt(Tmp, /*isUnsigned=*/true);
}

व्योम APSInt::Profile(Folding_Set_गाँठ_ID& ID) स्थिर {
  ID.AddInteger((अचिन्हित) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
