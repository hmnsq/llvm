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

#అన్తర్భావ "llvm/Support/MathExtras.h"

#నిర్వచితమైతే _MSC_VER
#అన్తర్భావ <limits>
#లేకపోతే
#అన్తర్భావ <cmath>
#అయితే_ముగింపు

నామాకాశ llvm {

#ఒకవేళ defined(_MSC_VER)
  // Visual Studio defines the HUGE_VAL class of macros using purposeful
  // constant arithmetic overflow, which it then warns on when encountered.
  స్థిరరాశి దశాంశం huge_valf = std::numeric_limits<దశాంశం>::infinity();
#లేకపోతే
  స్థిరరాశి దశాంశం huge_valf = HUGE_VALF;
#అయితే_ముగింపు

  /// Returns the number of digits in the given integer.
  పూర్ణాంకం అంకె_Digits_Base10(uint64_t X) {
    స్థిర constexpr నిర్మాణం Constexpr_దత్తాంశం {
      uint8_t At_కనిష్ఠ[65] = {};
      uint64_t Boundaries[20] = {};
      స్థిర constexpr పూర్ణాంకం అంకె_Digits_Constexpr(uint64_t N) {
        పూర్ణాంకం res = 1;
        అయితే (N >= 10) {
          res++;
          N /= 10;
        }
        తిరిగి res;
      }
      constexpr Constexpr_దత్తాంశం() {
        uint64_t Val = ~0ull;
        కొరకు (uint64_t i = 0; i <= 64; i++) {
          uint64_t Digits = అంకె_Digits_Constexpr(Val) - 1;
          At_కనిష్ఠ[i] = Digits;
          Val >>= 1;
        }
        // Special case because X=0 should return 1 and not 0
        Boundaries[0] = 0;
        Val = 10;
        కొరకు (uint64_t i = 1; i < 20; i++) {
          Boundaries[i] = Val;
          Val *= 10;
        }
      }
    } దత్తాంశం_2;

    uint64_t Base2 = X ? countl_zero(X) : 64;
    uint64_t Digits = దత్తాంశం_2.At_కనిష్ఠ[Base2];
    తిరిగి Digits + (X >= దత్తాంశం_2.Boundaries[Digits]);
  }

} // namespace llvm
