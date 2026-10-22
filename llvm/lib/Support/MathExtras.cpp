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

#অন্তর্ভাৱ "llvm/Support/MathExtras.h"

#ifdef _MSC_VER
#অন্তর্ভাৱ <limits>
#অন্যথা
#অন্তর্ভাৱ <cmath>
#endif

নামাকাশ llvm {

#যদি defined(_MSC_VER)
  // Visual Studio defines the HUGE_VAL class of macros using purposeful
  // constant arithmetic overflow, which it then warns on when encountered.
  ধ্রুৱ ভিন্ন huge_valf = std::numeric_limits<ভিন্ন>::infinity();
#অন্যথা
  ধ্রুৱ ভিন্ন huge_valf = HUGE_VALF;
#endif

  /// Returns the number of digits in the given integer.
  পূর্ণাঙ্ক অঙ্ক_Digits_Base10(uint64_t X) {
    স্থির constexpr সংরচনা ConstexprData {
      uint8_t AtLeast[65] = {};
      uint64_t Boundaries[20] = {};
      স্থির constexpr পূর্ণাঙ্ক অঙ্ক_Digits_Constexpr(uint64_t N) {
        পূর্ণাঙ্ক res = 1;
        যেতিয়ালৈ (N >= 10) {
          res++;
          N /= 10;
        }
        ঘূৰাই res;
      }
      constexpr ConstexprData() {
        uint64_t Val = ~0ull;
        বাবে (uint64_t i = 0; i <= 64; i++) {
          uint64_t Digits = অঙ্ক_Digits_Constexpr(Val) - 1;
          AtLeast[i] = Digits;
          Val >>= 1;
        }
        // Special case because X=0 should return 1 and not 0
        Boundaries[0] = 0;
        Val = 10;
        বাবে (uint64_t i = 1; i < 20; i++) {
          Boundaries[i] = Val;
          Val *= 10;
        }
      }
    } Data;

    uint64_t Base2 = X ? countl_zero(X) : 64;
    uint64_t Digits = Data.AtLeast[Base2];
    ঘূৰাই Digits + (X >= Data.Boundaries[Digits]);
  }

} // namespace llvm
