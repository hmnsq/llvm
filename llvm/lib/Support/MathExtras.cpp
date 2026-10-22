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

#ಅನ್ತರ್ಭಾವ "llvm/Support/MathExtras.h"

#ifdef _MSC_VER
#ಅನ್ತರ್ಭಾವ <limits>
#ಅನ್ಯಥಾ
#ಅನ್ತರ್ಭಾವ <cmath>
#endif

ನಾಮಾಕಾಶ llvm {

#ಯದಿ defined(_MSC_VER)
  // Visual Studio defines the HUGE_VAL class of macros using purposeful
  // constant arithmetic overflow, which it then warns on when encountered.
  ಧ್ರುವ ಭಿನ್ನ huge_valf = std::numeric_limits<ಭಿನ್ನ>::infinity();
#ಅನ್ಯಥಾ
  ಧ್ರುವ ಭಿನ್ನ huge_valf = HUGE_VALF;
#endif

  /// Returns the number of digits in the given integer.
  ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_Digits_Base10(uint64_t X) {
    ಸ್ಥಿರ constexpr ಸಂರಚನಾ ConstexprData {
      uint8_t AtLeast[65] = {};
      uint64_t Boundaries[20] = {};
      ಸ್ಥಿರ constexpr ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_Digits_Constexpr(uint64_t N) {
        ಪೂರ್ಣಾಙ್ಕ res = 1;
        ಯಾವತ್ (N >= 10) {
          res++;
          N /= 10;
        }
        ಪ್ರತ್ಯಾಗಮ res;
      }
      constexpr ConstexprData() {
        uint64_t Val = ~0ull;
        ಪ್ರತಿ (uint64_t i = 0; i <= 64; i++) {
          uint64_t Digits = ಅಂಕೆ_Digits_Constexpr(Val) - 1;
          AtLeast[i] = Digits;
          Val >>= 1;
        }
        // Special case because X=0 should return 1 and not 0
        Boundaries[0] = 0;
        Val = 10;
        ಪ್ರತಿ (uint64_t i = 1; i < 20; i++) {
          Boundaries[i] = Val;
          Val *= 10;
        }
      }
    } Data;

    uint64_t Base2 = X ? countl_zero(X) : 64;
    uint64_t Digits = Data.AtLeast[Base2];
    ಪ್ರತ್ಯಾಗಮ Digits + (X >= Data.Boundaries[Digits]);
  }

} // namespace llvm
