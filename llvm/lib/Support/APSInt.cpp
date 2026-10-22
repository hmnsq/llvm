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

کارول نطاق llvm;

APSInt::APSInt(StringRef Str) {
  أكد(!Str.تشه() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  بینښه عدد_بټونه_2 = ((Str.اندازه_2() * 64) / 19) + 2;
  APInt Tmp(عدد_بټونه_2, Str, /*radix=*/10);
  إذا (Str[0] == '-') {
    بینښه Min_بټونه = Tmp.get_Significant_بټونه();
    إذا (Min_بټونه < عدد_بټونه_2)
      Tmp = Tmp.trunc(std::max<بینښه>(1, Min_بټونه));
    *هذا = APSInt(Tmp, /*isUnsigned=*/خطأ);
    أرجع;
  }
  بینښه فعال_بټونه = Tmp.get_فعال_بټونه();
  إذا (فعال_بټونه < عدد_بټونه_2)
    Tmp = Tmp.trunc(std::max<بینښه>(1, فعال_بټونه));
  *هذا = APSInt(Tmp, /*isUnsigned=*/ریښتیا);
}

فراغ APSInt::Profile(Folding_سېټ_غوټه_ID& ID) ثابت {
  ID.جمع_کړه_Integer((بینښه) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
