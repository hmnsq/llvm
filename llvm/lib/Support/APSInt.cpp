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

استعمال نطاق llvm;

APSInt::APSInt(StringRef Str) {
  أكد(!Str.خالي_جايون() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  بنانشان انگ_بٽ_2 = ((Str.سائيز_2() * 64) / 19) + 2;
  APInt Tmp(انگ_بٽ_2, Str, /*radix=*/10);
  إذا (Str[0] == '-') {
    بنانشان Min_بٽ = Tmp.get_Significant_بٽ();
    إذا (Min_بٽ < انگ_بٽ_2)
      Tmp = Tmp.trunc(std::max<بنانشان>(1, Min_بٽ));
    *هذا = APSInt(Tmp, /*isUnsigned=*/خطأ);
    أرجع;
  }
  بنانشان سرگرم_بٽ = Tmp.get_سرگرم_بٽ();
  إذا (سرگرم_بٽ < انگ_بٽ_2)
    Tmp = Tmp.trunc(std::max<بنانشان>(1, سرگرم_بٽ));
  *هذا = APSInt(Tmp, /*isUnsigned=*/سچ);
}

فراغ APSInt::Profile(Folding_سيٽ_ڳنڍ_ID& ID) ثابت {
  ID.جوڙيو_Integer((بنانشان) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
