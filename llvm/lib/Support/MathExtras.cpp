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

#അന്തര്ഭാവ "llvm/Support/MathExtras.h"

#ifdef _MSC_VER
#അന്തര്ഭാവ <limits>
#അല്ലെങ്കിൽ
#അന്തര്ഭാവ <cmath>
#endif

നാമാകാശ llvm {

#എങ്കിൽ defined(_MSC_VER)
  // Visual Studio defines the HUGE_VAL class of macros using purposeful
  // constant arithmetic overflow, which it then warns on when encountered.
  സ്ഥിരം ദശാംശം huge_valf = std::numeric_limits<ദശാംശം>::infinity();
#അല്ലെങ്കിൽ
  സ്ഥിരം ദശാംശം huge_valf = HUGE_VALF;
#endif

  /// Returns the number of digits in the given integer.
  പൂർണ്ണസംഖ്യ അക്കം_Digits_Base10(uint64_t X) {
    സ്ഥിതം constexpr ഘടന ConstexprData {
      uint8_t AtLeast[65] = {};
      uint64_t Boundaries[20] = {};
      സ്ഥിതം constexpr പൂർണ്ണസംഖ്യ അക്കം_Digits_Constexpr(uint64_t N) {
        പൂർണ്ണസംഖ്യ res = 1;
        ആകുമ്പോൾ (N >= 10) {
          res++;
          N /= 10;
        }
        തിരികെ res;
      }
      constexpr ConstexprData() {
        uint64_t Val = ~0ull;
        വേണ്ടി (uint64_t i = 0; i <= 64; i++) {
          uint64_t Digits = അക്കം_Digits_Constexpr(Val) - 1;
          AtLeast[i] = Digits;
          Val >>= 1;
        }
        // Special case because X=0 should return 1 and not 0
        Boundaries[0] = 0;
        Val = 10;
        വേണ്ടി (uint64_t i = 1; i < 20; i++) {
          Boundaries[i] = Val;
          Val *= 10;
        }
      }
    } Data;

    uint64_t Base2 = X ? countl_zero(X) : 64;
    uint64_t Digits = Data.AtLeast[Base2];
    തിരികെ Digits + (X >= Data.Boundaries[Digits]);
  }

} // namespace llvm
