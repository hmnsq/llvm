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

#include "llvm/Support/BranchProbability.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <cmath>

using namespace llvm;

raw_ostream &BranchProbability::print(raw_ostream &OS) const {
  만약 (isUnknown())
    반환 OS << "?%";

  // Get a percentage rounded to two decimal digits. This avoids
  // implementation-defined rounding inside printf.
  double Percent = rint(((double)N / D) * 100.0 * 100.0) / 100.0;
  반환 OS << format("0x%08" PRIx32 " / 0x%08" PRIx32 " = %.2f%%", N, D,
                      Percent);
}

#만약 !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void BranchProbability::dump() const { print(dbgs()) << '\n'; }
#endif

BranchProbability
BranchProbability::getBranchProbability(uint64_t Numerator,
                                        uint64_t Denominator) {
  assert(Numerator <= Denominator && "Probability cannot be bigger than 1!");
  // Scale down Denominator to fit in a 32-bit integer.
  정수 Scale = 0;
  동안 (Denominator > UINT32_MAX) {
    Denominator >>= 1;
    Scale++;
  }
  반환 BranchProbability(Numerator >> Scale, Denominator);
}

BranchProbability BranchProbability::getBranchProbability(double Prob) {
  assert(0 <= Prob && Prob <= 1 && "Probability must be between 0 and 1!");
  반환 BranchProbability(std::round(Prob * D), D);
}

// If ConstD is not zero, then replace D by ConstD so that division and modulo
// operations by D can be optimized, in case this function is not inlined by the
// compiler.
template <uint32_t ConstD>
static uint64_t scale(uint64_t Num, uint32_t N, uint32_t D) {
  만약 (ConstD > 0)
    D = ConstD;

  assert(D && "divide by 0");

  // Fast path for multiplying by 1.0.
  만약 (!Num || D == N)
    반환 Num;

  // Split Num into upper and lower parts to multiply, then recombine.
  uint64_t ProductHigh = (Num >> 32) * N;
  uint64_t ProductLow = (Num & UINT32_MAX) * N;

  // Split into 32-bit digits.
  uint32_t Upper32 = ProductHigh >> 32;
  uint32_t Lower32 = ProductLow & UINT32_MAX;
  uint32_t Mid32Partial = ProductHigh & UINT32_MAX;
  uint32_t Mid32 = Mid32Partial + (ProductLow >> 32);

  // Carry.
  Upper32 += Mid32 < Mid32Partial;

  uint64_t Rem = (uint64_t(Upper32) << 32) | Mid32;
  uint64_t UpperQ = Rem / D;

  // Check for overflow.
  만약 (UpperQ > UINT32_MAX)
    반환 UINT64_MAX;

  Rem = ((Rem % D) << 32) | Lower32;
  uint64_t LowerQ = Rem / D;
  uint64_t Q = (UpperQ << 32) + LowerQ;

  // Check for overflow.
  반환 Q < LowerQ ? UINT64_MAX : Q;
}

uint64_t BranchProbability::scale(uint64_t Num) const {
  반환 ::scale<D>(Num, N, D);
}

uint64_t BranchProbability::scaleByInverse(uint64_t Num) const {
  반환 ::scale<0>(Num, D, N);
}

BranchProbability BranchProbability::pow(unsigned N) const {
  BranchProbability Res = BranchProbability::getOne();
  위해 (unsigned I = 0; I < N; ++I)
    Res *= *this;
  반환 Res;
}
