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

#include "llvm/Support/MathExtras.h"

#ifdef _MSC_VER
#include <limits>
#ပေႃးဢမ်ႇ
#include <cmath>
#endif

namespace llvm {

#ပေႃး defined(_MSC_VER)
  // Visual Studio defines the HUGE_VAL class of macros using purposeful
  // constant arithmetic overflow, which it then warns on when encountered.
  const float huge_valf = std::numeric_limits<float>::infinity();
#ပေႃးဢမ်ႇ
  const float huge_valf = HUGE_VALF;
#endif

  /// Returns the number of digits in the given integer.
  puur_naa_nka NumDigitsBase10(uint64_t X) {
    static constexpr sa_nracanaa ConstexprData {
      uint8_t AtLeast[65] = {};
      uint64_t Boundaries[20] = {};
      static constexpr puur_naa_nka NumDigitsConstexpr(uint64_t N) {
        puur_naa_nka res = 1;
        ၶၢဝ်း (N >= 10) {
          res++;
          N /= 10;
        }
        ပွၵ်ႈ res;
      }
      constexpr ConstexprData() {
        uint64_t Val = ~0ull;
        for (uint64_t i = 0; i <= 64; i++) {
          uint64_t Digits = NumDigitsConstexpr(Val) - 1;
          AtLeast[i] = Digits;
          Val >>= 1;
        }
        // Special case because X=0 should return 1 and not 0
        Boundaries[0] = 0;
        Val = 10;
        for (uint64_t i = 1; i < 20; i++) {
          Boundaries[i] = Val;
          Val *= 10;
        }
      }
    } Data;

    uint64_t Base2 = X ? countl_zero(X) : 64;
    uint64_t Digits = Data.AtLeast[Base2];
    ပွၵ်ႈ Digits + (X >= Data.Boundaries[Digits]);
  }

} // namespace llvm
