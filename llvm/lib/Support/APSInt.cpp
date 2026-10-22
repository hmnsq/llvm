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

استعمال نامخلا llvm;

APSInt::APSInt(StringRef Str) {
  أكد(!Str.خالی_خانے() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  غیرعلامتی عدد_بٹ_2 = ((Str.سائز_2() * 64) / 19) + 2;
  APInt Tmp(عدد_بٹ_2, Str, /*radix=*/10);
  اگر (Str[0] == '-') {
    غیرعلامتی Min_بٹ = Tmp.get_Significant_بٹ();
    اگر (Min_بٹ < عدد_بٹ_2)
      Tmp = Tmp.trunc(std::max<غیرعلامتی>(1, Min_بٹ));
    *یہ = APSInt(Tmp, /*isUnsigned=*/جھوٹ);
    واپس;
  }
  غیرعلامتی فعال_بٹ = Tmp.get_فعال_بٹ();
  اگر (فعال_بٹ < عدد_بٹ_2)
    Tmp = Tmp.trunc(std::max<غیرعلامتی>(1, فعال_بٹ));
  *یہ = APSInt(Tmp, /*isUnsigned=*/سچ);
}

خالی APSInt::Profile(Folding_سیٹ_گرہ_ID& ID) ثابت {
  ID.جوڑو_Integer((غیرعلامتی) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
