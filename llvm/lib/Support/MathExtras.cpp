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

#अन्तर्भाव "llvm/Support/MathExtras.h"

#यदि_परिभाषितम् _MSC_VER
#अन्तर्भाव <limits>
#अन्यथा
#अन्तर्भाव <cmath>
#यदि_अन्तः

नामाकाश llvm {

#यदि defined(_MSC_VER)
  // Visual Studio defines the HUGE_VAL class of macros using purposeful
  // constant arithmetic overflow, which it then warns on when encountered.
  ध्रुव भिन्न huge_valf = std::numeric_limits<भिन्न>::infinity();
#अन्यथा
  ध्रुव भिन्न huge_valf = HUGE_VALF;
#यदि_अन्तः

  /// Returns the number of digits in the given integer.
  पूर्णाङ्क अङ्कः_Digits_Base10(uint64_t X) {
    स्थिर constexpr संरचना Constexpr_दत्त {
      uint8_t At_न्यूनतमम्[65] = {};
      uint64_t Boundaries[20] = {};
      स्थिर constexpr पूर्णाङ्क अङ्कः_Digits_Constexpr(uint64_t N) {
        पूर्णाङ्क res = 1;
        यावत् (N >= 10) {
          res++;
          N /= 10;
        }
        प्रत्यागम res;
      }
      constexpr Constexpr_दत्त() {
        uint64_t Val = ~0ull;
        प्रति (uint64_t i = 0; i <= 64; i++) {
          uint64_t Digits = अङ्कः_Digits_Constexpr(Val) - 1;
          At_न्यूनतमम्[i] = Digits;
          Val >>= 1;
        }
        // Special case because X=0 should return 1 and not 0
        Boundaries[0] = 0;
        Val = 10;
        प्रति (uint64_t i = 1; i < 20; i++) {
          Boundaries[i] = Val;
          Val *= 10;
        }
      }
    } दत्त_2;

    uint64_t Base2 = X ? countl_zero(X) : 64;
    uint64_t Digits = दत्त_2.At_न्यूनतमम्[Base2];
    प्रत्यागम Digits + (X >= दत्त_2.Boundaries[Digits]);
  }

} // namespace llvm
