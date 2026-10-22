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

استفاده نطاق llvm;

APSInt::APSInt(StringRef Str) {
  أكد(!Str.خالی() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  بیعلامت رقم_بیتها_2 = ((Str.اندازه_2() * 64) / 19) + 2;
  APInt Tmp(رقم_بیتها_2, Str, /*radix=*/10);
  إذا (Str[0] == '-') {
    بیعلامت Min_بیتها = Tmp.get_Significant_بیتها();
    إذا (Min_بیتها < رقم_بیتها_2)
      Tmp = Tmp.trunc(std::max<بیعلامت>(1, Min_بیتها));
    *هذا = APSInt(Tmp, /*isUnsigned=*/خطأ);
    أرجع;
  }
  بیعلامت فعال_بیتها = Tmp.get_فعال_بیتها();
  إذا (فعال_بیتها < رقم_بیتها_2)
    Tmp = Tmp.trunc(std::max<بیعلامت>(1, فعال_بیتها));
  *هذا = APSInt(Tmp, /*isUnsigned=*/درست);
}

فراغ APSInt::Profile(Folding_مجموعه_گره_ID& ID) ثابت {
  ID.جمع_کن_Integer((بیعلامت) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
