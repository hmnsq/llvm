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

استعمال نامخلا llvm;

raw_ostream &BranchProbability::print(raw_ostream &OS) ثابت {
  اگر (isUnknown())
    واپس OS << "?%";

  // Get a percentage rounded to two decimal digits. This avoids
  // implementation-defined rounding inside printf.
  دوہرا Percent = rint(((دوہرا)N / D) * 100.0 * 100.0) / 100.0;
  واپس OS << format("0x%08" PRIx32 " / 0x%08" PRIx32 " = %.2f%%", N, D,
                      Percent);
}

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طریقہ خالی BranchProbability::dump() ثابت { print(dbgs()) << '\n'; }
#اگر_ختم

BranchProbability
BranchProbability::getBranchProbability(uint64_t Numerator,
                                        uint64_t Denominator) {
  أكد(Numerator <= Denominator && "Probability cannot be bigger than 1!");
  // Scale down Denominator to fit in a 32-bit integer.
  صحیح Scale = 0;
  جب (Denominator > UINT32_MAX) {
    Denominator >>= 1;
    Scale++;
  }
  واپس BranchProbability(Numerator >> Scale, Denominator);
}

BranchProbability BranchProbability::getBranchProbability(دوہرا Prob) {
  أكد(0 <= Prob && Prob <= 1 && "Probability must be between 0 and 1!");
  واپس BranchProbability(std::باری(Prob * D), D);
}

// If ConstD is not zero, then replace D by ConstD so that division and modulo
// operations by D can be optimized, in case this function is not inlined by the
// compiler.
سانچہ <uint32_t ConstD>
ساکن uint64_t scale(uint64_t عدد, uint32_t N, uint32_t D) {
  اگر (ConstD > 0)
    D = ConstD;

  أكد(D && "divide by 0");

  // Fast path for multiplying by 1.0.
  اگر (!عدد || D == N)
    واپس عدد;

  // Split Num into upper and lower parts to multiply, then recombine.
  uint64_t حاصل_ضرب_بلند = (عدد >> 32) * N;
  uint64_t حاصل_ضرب_کم = (عدد & UINT32_MAX) * N;

  // Split into 32-bit digits.
  uint32_t Upper32 = حاصل_ضرب_بلند >> 32;
  uint32_t Lower32 = حاصل_ضرب_کم & UINT32_MAX;
  uint32_t Mid32Partial = حاصل_ضرب_بلند & UINT32_MAX;
  uint32_t Mid32 = Mid32Partial + (حاصل_ضرب_کم >> 32);

  // Carry.
  Upper32 += Mid32 < Mid32Partial;

  uint64_t Rem = (uint64_t(Upper32) << 32) | Mid32;
  uint64_t بالائی_Q = Rem / D;

  // Check for overflow.
  اگر (بالائی_Q > UINT32_MAX)
    واپس UINT64_MAX;

  Rem = ((Rem % D) << 32) | Lower32;
  uint64_t زیریں_Q = Rem / D;
  uint64_t Q = (بالائی_Q << 32) + زیریں_Q;

  // Check for overflow.
  واپس Q < زیریں_Q ? UINT64_MAX : Q;
}

uint64_t BranchProbability::scale(uint64_t عدد) ثابت {
  واپس ::scale<D>(عدد, N, D);
}

uint64_t BranchProbability::scaleByInverse(uint64_t عدد) ثابت {
  واپس ::scale<0>(عدد, D, N);
}

BranchProbability BranchProbability::قوت(غیرعلامتی N) ثابت {
  BranchProbability Res = BranchProbability::getOne();
  برائے (غیرعلامتی I = 0; I < N; ++I)
    Res *= *یہ;
  واپس Res;
}
