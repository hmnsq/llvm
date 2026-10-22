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

#ضمن "llvm/Support/BranchProbability.h"
#ضمن "llvm/Config/llvm-config.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/Format.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <cassert>
#ضمن <cmath>

کارول نطاق llvm;

raw_ostream &BranchProbability::print(raw_ostream &OS) ثابت {
  إذا (isUnknown())
    أرجع OS << "?%";

  // Get a percentage rounded to two decimal digits. This avoids
  // implementation-defined rounding inside printf.
  مزدوج Percent = rint(((مزدوج)N / D) * 100.0 * 100.0) / 100.0;
  أرجع OS << format("0x%08" PRIx32 " / 0x%08" PRIx32 " = %.2f%%", N, D,
                      Percent);
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_لاره فراغ BranchProbability::dump() ثابت { print(dbgs()) << '\n'; }
#د_که_پای

BranchProbability
BranchProbability::getBranchProbability(uint64_t Numerator,
                                        uint64_t Denominator) {
  أكد(Numerator <= Denominator && "Probability cannot be bigger than 1!");
  // Scale down Denominator to fit in a 32-bit integer.
  صحيح Scale = 0;
  طالما (Denominator > UINT32_MAX) {
    Denominator >>= 1;
    Scale++;
  }
  أرجع BranchProbability(Numerator >> Scale, Denominator);
}

BranchProbability BranchProbability::getBranchProbability(مزدوج Prob) {
  أكد(0 <= Prob && Prob <= 1 && "Probability must be between 0 and 1!");
  أرجع BranchProbability(std::پړاو(Prob * D), D);
}

// If ConstD is not zero, then replace D by ConstD so that division and modulo
// operations by D can be optimized, in case this function is not inlined by the
// compiler.
کينډۍ <uint32_t ConstD>
ساكن uint64_t scale(uint64_t عدد, uint32_t N, uint32_t D) {
  إذا (ConstD > 0)
    D = ConstD;

  أكد(D && "divide by 0");

  // Fast path for multiplying by 1.0.
  إذا (!عدد || D == N)
    أرجع عدد;

  // Split Num into upper and lower parts to multiply, then recombine.
  uint64_t حاصل_ضرب_لوړ = (عدد >> 32) * N;
  uint64_t حاصل_ضرب_ټیټ = (عدد & UINT32_MAX) * N;

  // Split into 32-bit digits.
  uint32_t Upper32 = حاصل_ضرب_لوړ >> 32;
  uint32_t Lower32 = حاصل_ضرب_ټیټ & UINT32_MAX;
  uint32_t Mid32Partial = حاصل_ضرب_لوړ & UINT32_MAX;
  uint32_t Mid32 = Mid32Partial + (حاصل_ضرب_ټیټ >> 32);

  // Carry.
  Upper32 += Mid32 < Mid32Partial;

  uint64_t Rem = (uint64_t(Upper32) << 32) | Mid32;
  uint64_t پاسنی_Q = Rem / D;

  // Check for overflow.
  إذا (پاسنی_Q > UINT32_MAX)
    أرجع UINT64_MAX;

  Rem = ((Rem % D) << 32) | Lower32;
  uint64_t ښکتنی_Q = Rem / D;
  uint64_t Q = (پاسنی_Q << 32) + ښکتنی_Q;

  // Check for overflow.
  أرجع Q < ښکتنی_Q ? UINT64_MAX : Q;
}

uint64_t BranchProbability::scale(uint64_t عدد) ثابت {
  أرجع ::scale<D>(عدد, N, D);
}

uint64_t BranchProbability::scaleByInverse(uint64_t عدد) ثابت {
  أرجع ::scale<0>(عدد, D, N);
}

BranchProbability BranchProbability::توان(بینښه N) ثابت {
  BranchProbability Res = BranchProbability::getOne();
  لكل (بینښه I = 0; I < N; ++I)
    Res *= *هذا;
  أرجع Res;
}
