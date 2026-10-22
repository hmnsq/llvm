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

#شامل "llvm/Support/MathExtras.h"

#اگر_تعریف_شدہ _MSC_VER
#شامل <limits>
#ورنہ
#شامل <cmath>
#اگر_ختم

نامخلا llvm {

#اگر defined(_MSC_VER)
  // Visual Studio defines the HUGE_VAL class of macros using purposeful
  // constant arithmetic overflow, which it then warns on when encountered.
  ثابت اعشاری huge_valf = std::numeric_limits<اعشاری>::infinity();
#ورنہ
  ثابت اعشاری huge_valf = HUGE_VALF;
#اگر_ختم

  /// Returns the number of digits in the given integer.
  صحیح عدد_Digits_Base10(uint64_t X) {
    ساکن constexpr ساخت Constexpr_ڈیٹا {
      uint8_t At_کم_سے_کم[65] = {};
      uint64_t Boundaries[20] = {};
      ساکن constexpr صحیح عدد_Digits_Constexpr(uint64_t N) {
        صحیح res = 1;
        جب (N >= 10) {
          res++;
          N /= 10;
        }
        واپس res;
      }
      constexpr Constexpr_ڈیٹا() {
        uint64_t Val = ~0ull;
        برائے (uint64_t i = 0; i <= 64; i++) {
          uint64_t Digits = عدد_Digits_Constexpr(Val) - 1;
          At_کم_سے_کم[i] = Digits;
          Val >>= 1;
        }
        // Special case because X=0 should return 1 and not 0
        Boundaries[0] = 0;
        Val = 10;
        برائے (uint64_t i = 1; i < 20; i++) {
          Boundaries[i] = Val;
          Val *= 10;
        }
      }
    } ڈیٹا_2;

    uint64_t Base2 = X ? countl_zero(X) : 64;
    uint64_t Digits = ڈیٹا_2.At_کم_سے_کم[Base2];
    واپس Digits + (X >= ڈیٹا_2.Boundaries[Digits]);
  }

} // namespace llvm
