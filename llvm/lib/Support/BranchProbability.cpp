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

#અન્તર્ભાવ "llvm/Support/BranchProbability.h"
#અન્તર્ભાવ "llvm/Config/llvm-config.h"
#અન્તર્ભાવ "llvm/Support/Debug.h"
#અન્તર્ભાવ "llvm/Support/Format.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <cmath>

using નામાકાશ llvm;

raw_ostream &BranchProbability::print(raw_ostream &OS) અચલ {
  જો (isUnknown())
    પાછા OS << "?%";

  // Get a percentage rounded to two decimal digits. This avoids
  // implementation-defined rounding inside printf.
  દ્વિગુણ Percent = rint(((દ્વિગુણ)N / D) * 100.0 * 100.0) / 100.0;
  પાછા OS << format("0x%08" PRIx32 " / 0x%08" PRIx32 " = %.2f%%", N, D,
                      Percent);
}

#જો !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD શૂન્ય BranchProbability::dump() અચલ { print(dbgs()) << '\n'; }
#endif

BranchProbability
BranchProbability::getBranchProbability(uint64_t Numerator,
                                        uint64_t Denominator) {
  પ્રતિજ્ઞા(Numerator <= Denominator && "Probability cannot be bigger than 1!");
  // Scale down Denominator to fit in a 32-bit integer.
  પૂર્ણાંક Scale = 0;
  જ્યારે (Denominator > UINT32_MAX) {
    Denominator >>= 1;
    Scale++;
  }
  પાછા BranchProbability(Numerator >> Scale, Denominator);
}

BranchProbability BranchProbability::getBranchProbability(દ્વિગુણ Prob) {
  પ્રતિજ્ઞા(0 <= Prob && Prob <= 1 && "Probability must be between 0 and 1!");
  પાછા BranchProbability(std::round(Prob * D), D);
}

// If ConstD is not zero, then replace D by ConstD so that division and modulo
// operations by D can be optimized, in case this function is not inlined by the
// compiler.
template <uint32_t ConstD>
સ્થિર uint64_t scale(uint64_t અંક, uint32_t N, uint32_t D) {
  જો (ConstD > 0)
    D = ConstD;

  પ્રતિજ્ઞા(D && "divide by 0");

  // Fast path for multiplying by 1.0.
  જો (!અંક || D == N)
    પાછા અંક;

  // Split Num into upper and lower parts to multiply, then recombine.
  uint64_t Product_ઉપલું = (અંક >> 32) * N;
  uint64_t Product_નીચલું = (અંક & UINT32_MAX) * N;

  // Split into 32-bit digits.
  uint32_t Upper32 = Product_ઉપલું >> 32;
  uint32_t Lower32 = Product_નીચલું & UINT32_MAX;
  uint32_t Mid32Partial = Product_ઉપલું & UINT32_MAX;
  uint32_t Mid32 = Mid32Partial + (Product_નીચલું >> 32);

  // Carry.
  Upper32 += Mid32 < Mid32Partial;

  uint64_t Rem = (uint64_t(Upper32) << 32) | Mid32;
  uint64_t UpperQ = Rem / D;

  // Check for overflow.
  જો (UpperQ > UINT32_MAX)
    પાછા UINT64_MAX;

  Rem = ((Rem % D) << 32) | Lower32;
  uint64_t LowerQ = Rem / D;
  uint64_t Q = (UpperQ << 32) + LowerQ;

  // Check for overflow.
  પાછા Q < LowerQ ? UINT64_MAX : Q;
}

uint64_t BranchProbability::scale(uint64_t અંક) અચલ {
  પાછા ::scale<D>(અંક, N, D);
}

uint64_t BranchProbability::scaleByInverse(uint64_t અંક) અચલ {
  પાછા ::scale<0>(અંક, D, N);
}

BranchProbability BranchProbability::pow(unsigned N) અચલ {
  BranchProbability Res = BranchProbability::getOne();
  માટે (unsigned I = 0; I < N; ++I)
    Res *= *આ;
  પાછા Res;
}
