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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/APSInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/FoldingSet.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

APSInt::APSInt(StringRef Str) {
  ਪ੍ਰਤਿਜ੍ਞਾ(!Str.ਖਾਲੀ_ਥਾਵਾਂ() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਿੱਟ_2 = ((Str.ਮਾਪ_2() * 64) / 19) + 2;
  APInt Tmp(ਅੰਕ_ਬਿੱਟ_2, Str, /*radix=*/10);
  ਜੇ (Str[0] == '-') {
    ਬਿਨਾਨਿਸ਼ਾਨੀ Min_ਬਿੱਟ = Tmp.get_Significant_ਬਿੱਟ();
    ਜੇ (Min_ਬਿੱਟ < ਅੰਕ_ਬਿੱਟ_2)
      Tmp = Tmp.trunc(std::max<ਬਿਨਾਨਿਸ਼ਾਨੀ>(1, Min_ਬਿੱਟ));
    *ਇਹ = APSInt(Tmp, /*isUnsigned=*/ਝੂਠ);
    ਵਾਪਸ;
  }
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਰਗਰਮ_ਬਿੱਟ = Tmp.get_ਸਰਗਰਮ_ਬਿੱਟ();
  ਜੇ (ਸਰਗਰਮ_ਬਿੱਟ < ਅੰਕ_ਬਿੱਟ_2)
    Tmp = Tmp.trunc(std::max<ਬਿਨਾਨਿਸ਼ਾਨੀ>(1, ਸਰਗਰਮ_ਬਿੱਟ));
  *ਇਹ = APSInt(Tmp, /*isUnsigned=*/ਸੱਚ);
}

ਖਾਲੀ APSInt::Profile(Folding_ਸੈੱਟ_ਗੰਢ_ID& ID) ਅਚਲ {
  ID.ਜੋੜੋ_Integer((ਬਿਨਾਨਿਸ਼ਾਨੀ) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
