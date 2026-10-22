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

#समावेश "llvm/Support/MathExtras.h"

#अगर_घोषित _MSC_VER
#समावेश <limits>
#अन्यथा
#समावेश <cmath>
#पूर्ण_अगर

namespace llvm {

#अगर defined(_MSC_VER)
  // Visual Studio defines the HUGE_VAL class of macros using purposeful
  // constant arithmetic overflow, which it then warns on when encountered.
  स्थिर भग्न huge_valf = std::numeric_limits<भग्न>::infinity();
#अन्यथा
  स्थिर भग्न huge_valf = HUGE_VALF;
#पूर्ण_अगर

  /// Returns the number of digits in the given integer.
  पूर्णांक अंक_Digits_Base10(uint64_t X) {
    अटल constexpr काष्ठा Constexpr_तथ्य {
      uint8_t AtLeast[65] = {};
      uint64_t Boundaries[20] = {};
      अटल constexpr पूर्णांक अंक_Digits_Constexpr(uint64_t N) {
        पूर्णांक res = 1;
        जबतक (N >= 10) {
          res++;
          N /= 10;
        }
        वापस res;
      }
      constexpr Constexpr_तथ्य() {
        uint64_t Val = ~0ull;
        क्रम (uint64_t i = 0; i <= 64; i++) {
          uint64_t Digits = अंक_Digits_Constexpr(Val) - 1;
          AtLeast[i] = Digits;
          Val >>= 1;
        }
        // Special case because X=0 should return 1 and not 0
        Boundaries[0] = 0;
        Val = 10;
        क्रम (uint64_t i = 1; i < 20; i++) {
          Boundaries[i] = Val;
          Val *= 10;
        }
      }
    } तथ्य_2;

    uint64_t Base2 = X ? countl_zero(X) : 64;
    uint64_t Digits = तथ्य_2.AtLeast[Base2];
    वापस Digits + (X >= तथ्य_2.Boundaries[Digits]);
  }

} // namespace llvm
