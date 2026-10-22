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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MathExtras.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _MSC_VER
#ਅਨ੍ਤਰ੍ਭਾਵ <limits>
#ਨਹੀਂਤਾਂ
#ਅਨ੍ਤਰ੍ਭਾਵ <cmath>
#ਜੇ_ਅੰਤ

ਨਾਮਾਕਾਸ਼ llvm {

#ਜੇ defined(_MSC_VER)
  // Visual Studio defines the HUGE_VAL class of macros using purposeful
  // constant arithmetic overflow, which it then warns on when encountered.
  ਅਚਲ ਦਸ਼ਮਲਵ huge_valf = std::numeric_limits<ਦਸ਼ਮਲਵ>::infinity();
#ਨਹੀਂਤਾਂ
  ਅਚਲ ਦਸ਼ਮਲਵ huge_valf = HUGE_VALF;
#ਜੇ_ਅੰਤ

  /// Returns the number of digits in the given integer.
  ਪੂਰਨ ਅੰਕ_Digits_Base10(uint64_t X) {
    ਸਥਿਰ constexpr ਬਣਤਰ Constexpr_ਅੰਕੜੇ {
      uint8_t At_ਘੱਟ_ਤੋਂ_ਘੱਟ[65] = {};
      uint64_t Boundaries[20] = {};
      ਸਥਿਰ constexpr ਪੂਰਨ ਅੰਕ_Digits_Constexpr(uint64_t N) {
        ਪੂਰਨ res = 1;
        ਜਦੋਂ (N >= 10) {
          res++;
          N /= 10;
        }
        ਵਾਪਸ res;
      }
      constexpr Constexpr_ਅੰਕੜੇ() {
        uint64_t Val = ~0ull;
        ਲਈ (uint64_t i = 0; i <= 64; i++) {
          uint64_t Digits = ਅੰਕ_Digits_Constexpr(Val) - 1;
          At_ਘੱਟ_ਤੋਂ_ਘੱਟ[i] = Digits;
          Val >>= 1;
        }
        // Special case because X=0 should return 1 and not 0
        Boundaries[0] = 0;
        Val = 10;
        ਲਈ (uint64_t i = 1; i < 20; i++) {
          Boundaries[i] = Val;
          Val *= 10;
        }
      }
    } ਅੰਕੜੇ_2;

    uint64_t Base2 = X ? countl_zero(X) : 64;
    uint64_t Digits = ਅੰਕੜੇ_2.At_ਘੱਟ_ਤੋਂ_ਘੱਟ[Base2];
    ਵਾਪਸ Digits + (X >= ਅੰਕੜੇ_2.Boundaries[Digits]);
  }

} // namespace llvm
