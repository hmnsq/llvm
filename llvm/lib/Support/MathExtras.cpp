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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _MSC_VER
#ಅನ್ತರ್ಭಾವ <limits>
#ಇಲ್ಲದಿದ್ದರೆ
#ಅನ್ತರ್ಭಾವ <cmath>
#ಆದರೆ_ಅಂತ್ಯ

ನಾಮಾಕಾಶ llvm {

#ಆದರೆ defined(_MSC_VER)
  // Visual Studio defines the HUGE_VAL class of macros using purposeful
  // constant arithmetic overflow, which it then warns on when encountered.
  ಸ್ಥಿರಾಂಕ ದಶಮಾಂಶ huge_valf = std::numeric_limits<ದಶಮಾಂಶ>::infinity();
#ಇಲ್ಲದಿದ್ದರೆ
  ಸ್ಥಿರಾಂಕ ದಶಮಾಂಶ huge_valf = HUGE_VALF;
#ಆದರೆ_ಅಂತ್ಯ

  /// Returns the number of digits in the given integer.
  ಪೂರ್ಣಾಂಕ ಅಂಕೆ_Digits_Base10(uint64_t X) {
    ಸ್ಥಿರ constexpr ರಚನೆ Constexpr_ದತ್ತಾಂಶ {
      uint8_t At_ಕನಿಷ್ಠ[65] = {};
      uint64_t Boundaries[20] = {};
      ಸ್ಥಿರ constexpr ಪೂರ್ಣಾಂಕ ಅಂಕೆ_Digits_Constexpr(uint64_t N) {
        ಪೂರ್ಣಾಂಕ res = 1;
        ಆಗ (N >= 10) {
          res++;
          N /= 10;
        }
        ಹಿಂತಿರುಗಿ res;
      }
      constexpr Constexpr_ದತ್ತಾಂಶ() {
        uint64_t Val = ~0ull;
        ಗಾಗಿ (uint64_t i = 0; i <= 64; i++) {
          uint64_t Digits = ಅಂಕೆ_Digits_Constexpr(Val) - 1;
          At_ಕನಿಷ್ಠ[i] = Digits;
          Val >>= 1;
        }
        // Special case because X=0 should return 1 and not 0
        Boundaries[0] = 0;
        Val = 10;
        ಗಾಗಿ (uint64_t i = 1; i < 20; i++) {
          Boundaries[i] = Val;
          Val *= 10;
        }
      }
    } ದತ್ತಾಂಶ_2;

    uint64_t Base2 = X ? countl_zero(X) : 64;
    uint64_t Digits = ದತ್ತಾಂಶ_2.At_ಕನಿಷ್ಠ[Base2];
    ಹಿಂತಿರುಗಿ Digits + (X >= ದತ್ತಾಂಶ_2.Boundaries[Digits]);
  }

} // namespace llvm
