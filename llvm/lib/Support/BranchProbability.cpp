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

استعمال نطاق llvm;

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
LLVM_DUMP_طريقو فراغ BranchProbability::dump() ثابت { print(dbgs()) << '\n'; }
#جيڪڏهن_پورو

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
  أرجع BranchProbability(std::ڦيرو(Prob * D), D);
}

// If ConstD is not zero, then replace D by ConstD so that division and modulo
// operations by D can be optimized, in case this function is not inlined by the
// compiler.
سانچو <uint32_t ConstD>
ساكن uint64_t scale(uint64_t انگ, uint32_t N, uint32_t D) {
  إذا (ConstD > 0)
    D = ConstD;

  أكد(D && "divide by 0");

  // Fast path for multiplying by 1.0.
  إذا (!انگ || D == N)
    أرجع انگ;

  // Split Num into upper and lower parts to multiply, then recombine.
  uint64_t حاصل_ضرب_مٿاهون = (انگ >> 32) * N;
  uint64_t حاصل_ضرب_هيٺيون = (انگ & UINT32_MAX) * N;

  // Split into 32-bit digits.
  uint32_t Upper32 = حاصل_ضرب_مٿاهون >> 32;
  uint32_t Lower32 = حاصل_ضرب_هيٺيون & UINT32_MAX;
  uint32_t Mid32Partial = حاصل_ضرب_مٿاهون & UINT32_MAX;
  uint32_t Mid32 = Mid32Partial + (حاصل_ضرب_هيٺيون >> 32);

  // Carry.
  Upper32 += Mid32 < Mid32Partial;

  uint64_t Rem = (uint64_t(Upper32) << 32) | Mid32;
  uint64_t مٿانهون_Q = Rem / D;

  // Check for overflow.
  إذا (مٿانهون_Q > UINT32_MAX)
    أرجع UINT64_MAX;

  Rem = ((Rem % D) << 32) | Lower32;
  uint64_t هيٺاهون_Q = Rem / D;
  uint64_t Q = (مٿانهون_Q << 32) + هيٺاهون_Q;

  // Check for overflow.
  أرجع Q < هيٺاهون_Q ? UINT64_MAX : Q;
}

uint64_t BranchProbability::scale(uint64_t انگ) ثابت {
  أرجع ::scale<D>(انگ, N, D);
}

uint64_t BranchProbability::scaleByInverse(uint64_t انگ) ثابت {
  أرجع ::scale<0>(انگ, D, N);
}

BranchProbability BranchProbability::قوت(بنانشان N) ثابت {
  BranchProbability Res = BranchProbability::getOne();
  لكل (بنانشان I = 0; I < N; ++I)
    Res *= *هذا;
  أرجع Res;
}
