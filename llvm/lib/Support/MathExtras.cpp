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

#ضمن "llvm/Support/MathExtras.h"

#جيڪڏهن_وصف_ٿيل _MSC_VER
#ضمن <limits>
#وإلا
#ضمن <cmath>
#جيڪڏهن_پورو

نطاق llvm {

#إذا defined(_MSC_VER)
  // Visual Studio defines the HUGE_VAL class of macros using purposeful
  // constant arithmetic overflow, which it then warns on when encountered.
  ثابت عائم huge_valf = std::numeric_limits<عائم>::infinity();
#وإلا
  ثابت عائم huge_valf = HUGE_VALF;
#جيڪڏهن_پورو

  /// Returns the number of digits in the given integer.
  صحيح انگ_Digits_Base10(uint64_t X) {
    ساكن constexpr بنية Constexpr_معلومات {
      uint8_t At_گهٽ_کان_گهٽ[65] = {};
      uint64_t Boundaries[20] = {};
      ساكن constexpr صحيح انگ_Digits_Constexpr(uint64_t N) {
        صحيح res = 1;
        طالما (N >= 10) {
          res++;
          N /= 10;
        }
        أرجع res;
      }
      constexpr Constexpr_معلومات() {
        uint64_t Val = ~0ull;
        لكل (uint64_t i = 0; i <= 64; i++) {
          uint64_t Digits = انگ_Digits_Constexpr(Val) - 1;
          At_گهٽ_کان_گهٽ[i] = Digits;
          Val >>= 1;
        }
        // Special case because X=0 should return 1 and not 0
        Boundaries[0] = 0;
        Val = 10;
        لكل (uint64_t i = 1; i < 20; i++) {
          Boundaries[i] = Val;
          Val *= 10;
        }
      }
    } معلومات_2;

    uint64_t Base2 = X ? countl_zero(X) : 64;
    uint64_t Digits = معلومات_2.At_گهٽ_کان_گهٽ[Base2];
    أرجع Digits + (X >= معلومات_2.Boundaries[Digits]);
  }

} // namespace llvm
