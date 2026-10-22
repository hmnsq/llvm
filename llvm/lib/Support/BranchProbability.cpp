//===-------------- lib/Support/BranchProbability.cpp -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements Branch Probability class.
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/BranchProbability.h"
#شامل "llvm/Config/llvm-config.h"
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/Format.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <cassert>
#شامل <cmath>

using فضاینام llvm;

raw_ostream &BranchProbability::print(raw_ostream &OS) ثابت {
  اگر (isUnknown())
    بازگشت OS << "?%";

  // Get a percentage rounded to two decimal digits. This avoids
  // implementation-defined rounding inside printf.
  دوگانه Percent = rint(((دوگانه)N / D) * 100.0 * 100.0) / 100.0;
  بازگشت OS << format("0x%08" PRIx32 " / 0x%08" PRIx32 " = %.2f%%", N, D,
                      Percent);
}

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD تهی BranchProbability::dump() ثابت { print(dbgs()) << '\n'; }
#endif

BranchProbability
BranchProbability::getBranchProbability(uint64_t Numerator,
                                        uint64_t Denominator) {
  أكد(Numerator <= Denominator && "Probability cannot be bigger than 1!");
  // Scale down Denominator to fit in a 32-bit integer.
  صحیح Scale = 0;
  وقتی (Denominator > UINT32_MAX) {
    Denominator >>= 1;
    Scale++;
  }
  بازگشت BranchProbability(Numerator >> Scale, Denominator);
}

BranchProbability BranchProbability::getBranchProbability(دوگانه Prob) {
  أكد(0 <= Prob && Prob <= 1 && "Probability must be between 0 and 1!");
  بازگشت BranchProbability(std::round(Prob * D), D);
}

// If ConstD is not zero, then replace D by ConstD so that division and modulo
// operations by D can be optimized, in case this function is not inlined by the
// compiler.
template <uint32_t ConstD>
ایستا uint64_t scale(uint64_t رقم, uint32_t N, uint32_t D) {
  اگر (ConstD > 0)
    D = ConstD;

  أكد(D && "divide by 0");

  // Fast path for multiplying by 1.0.
  اگر (!رقم || D == N)
    بازگشت رقم;

  // Split Num into upper and lower parts to multiply, then recombine.
  uint64_t Product_بالا = (رقم >> 32) * N;
  uint64_t Product_کم = (رقم & UINT32_MAX) * N;

  // Split into 32-bit digits.
  uint32_t Upper32 = Product_بالا >> 32;
  uint32_t Lower32 = Product_کم & UINT32_MAX;
  uint32_t Mid32Partial = Product_بالا & UINT32_MAX;
  uint32_t Mid32 = Mid32Partial + (Product_کم >> 32);

  // Carry.
  Upper32 += Mid32 < Mid32Partial;

  uint64_t Rem = (uint64_t(Upper32) << 32) | Mid32;
  uint64_t UpperQ = Rem / D;

  // Check for overflow.
  اگر (UpperQ > UINT32_MAX)
    بازگشت UINT64_MAX;

  Rem = ((Rem % D) << 32) | Lower32;
  uint64_t LowerQ = Rem / D;
  uint64_t Q = (UpperQ << 32) + LowerQ;

  // Check for overflow.
  بازگشت Q < LowerQ ? UINT64_MAX : Q;
}

uint64_t BranchProbability::scale(uint64_t رقم) ثابت {
  بازگشت ::scale<D>(رقم, N, D);
}

uint64_t BranchProbability::scaleByInverse(uint64_t رقم) ثابت {
  بازگشت ::scale<0>(رقم, D, N);
}

BranchProbability BranchProbability::pow(unsigned N) ثابت {
  BranchProbability Res = BranchProbability::getOne();
  برای (unsigned I = 0; I < N; ++I)
    Res *= *این;
  بازگشت Res;
}
