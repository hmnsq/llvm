//===-- MathExtras.cpp - Implement the MathExtras header --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the MathExtras.h header
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/MathExtras.h"

#ifdef _MSC_VER
#ଅନ୍ତର୍ଭାଵ <limits>
#ଅନ୍ୟଥା
#ଅନ୍ତର୍ଭାଵ <cmath>
#endif

ନାମାକାଶ llvm {

#ଯଦି defined(_MSC_VER)
  // Visual Studio defines the HUGE_VAL class of macros using purposeful
  // constant arithmetic overflow, which it then warns on when encountered.
  ଅଚଳ ଦଶମିକ huge_valf = std::numeric_limits<ଦଶମିକ>::infinity();
#ଅନ୍ୟଥା
  ଅଚଳ ଦଶମିକ huge_valf = HUGE_VALF;
#endif

  /// Returns the number of digits in the given integer.
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_Digits_Base10(uint64_t X) {
    ସ୍ଥିର constexpr ସଂରଚନା ConstexprData {
      uint8_t AtLeast[65] = {};
      uint64_t Boundaries[20] = {};
      ସ୍ଥିର constexpr ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_Digits_Constexpr(uint64_t N) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = 1;
        ଯେତେବେଳେ (N >= 10) {
          res++;
          N /= 10;
        }
        ଫେରନ୍ତୁ res;
      }
      constexpr ConstexprData() {
        uint64_t Val = ~0ull;
        ପାଇଁ (uint64_t i = 0; i <= 64; i++) {
          uint64_t Digits = ଅଙ୍କ_Digits_Constexpr(Val) - 1;
          AtLeast[i] = Digits;
          Val >>= 1;
        }
        // Special case because X=0 should return 1 and not 0
        Boundaries[0] = 0;
        Val = 10;
        ପାଇଁ (uint64_t i = 1; i < 20; i++) {
          Boundaries[i] = Val;
          Val *= 10;
        }
      }
    } Data;

    uint64_t Base2 = X ? countl_zero(X) : 64;
    uint64_t Digits = Data.AtLeast[Base2];
    ଫେରନ୍ତୁ Digits + (X >= Data.Boundaries[Digits]);
  }

} // namespace llvm
