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

ورتوں نطاق llvm;

APSInt::APSInt(StringRef Str) {
  أكد(!Str.خالی_تھاواں() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  بنانشانی عدد_بٹ_2 = ((Str.ناپ_2() * 64) / 19) + 2;
  APInt Tmp(عدد_بٹ_2, Str, /*radix=*/10);
  إذا (Str[0] == '-') {
    بنانشانی Min_بٹ = Tmp.get_Significant_بٹ();
    إذا (Min_بٹ < عدد_بٹ_2)
      Tmp = Tmp.trunc(std::max<بنانشانی>(1, Min_بٹ));
    *هذا = APSInt(Tmp, /*isUnsigned=*/خطأ);
    أرجع;
  }
  بنانشانی سرگرم_بٹ = Tmp.get_سرگرم_بٹ();
  إذا (سرگرم_بٹ < عدد_بٹ_2)
    Tmp = Tmp.trunc(std::max<بنانشانی>(1, سرگرم_بٹ));
  *هذا = APSInt(Tmp, /*isUnsigned=*/سچ);
}

فراغ APSInt::Profile(Folding_سیٹ_گرہ_ID& ID) ثابت {
  ID.جوڑو_Integer((بنانشانی) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
