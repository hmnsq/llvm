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

#અન્તર્ભાવ "llvm/Support/MathExtras.h"

#ifdef _MSC_VER
#અન્તર્ભાવ <limits>
#નહીંતર
#અન્તર્ભાવ <cmath>
#endif

નામાકાશ llvm {

#જો defined(_MSC_VER)
  // Visual Studio defines the HUGE_VAL class of macros using purposeful
  // constant arithmetic overflow, which it then warns on when encountered.
  અચલ દશાંશ huge_valf = std::numeric_limits<દશાંશ>::infinity();
#નહીંતર
  અચલ દશાંશ huge_valf = HUGE_VALF;
#endif

  /// Returns the number of digits in the given integer.
  પૂર્ણાંક અંક_Digits_Base10(uint64_t X) {
    સ્થિર constexpr સંરચના ConstexprData {
      uint8_t AtLeast[65] = {};
      uint64_t Boundaries[20] = {};
      સ્થિર constexpr પૂર્ણાંક અંક_Digits_Constexpr(uint64_t N) {
        પૂર્ણાંક res = 1;
        જ્યારે (N >= 10) {
          res++;
          N /= 10;
        }
        પાછા res;
      }
      constexpr ConstexprData() {
        uint64_t Val = ~0ull;
        માટે (uint64_t i = 0; i <= 64; i++) {
          uint64_t Digits = અંક_Digits_Constexpr(Val) - 1;
          AtLeast[i] = Digits;
          Val >>= 1;
        }
        // Special case because X=0 should return 1 and not 0
        Boundaries[0] = 0;
        Val = 10;
        માટે (uint64_t i = 1; i < 20; i++) {
          Boundaries[i] = Val;
          Val *= 10;
        }
      }
    } Data;

    uint64_t Base2 = X ? countl_zero(X) : 64;
    uint64_t Digits = Data.AtLeast[Base2];
    પાછા Digits + (X >= Data.Boundaries[Digits]);
  }

} // namespace llvm
