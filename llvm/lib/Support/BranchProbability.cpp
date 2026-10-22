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

#समावेश "llvm/Support/BranchProbability.h"
#समावेश "llvm/Config/llvm-config.h"
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/Format.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <cassert>
#समावेश <cmath>

using namespace llvm;

raw_ostream &BranchProbability::print(raw_ostream &OS) स्थिर {
  अगर (isUnknown())
    वापस OS << "?%";

  // Get a percentage rounded to two decimal digits. This avoids
  // implementation-defined rounding inside printf.
  द्विगुन Percent = rint(((द्विगुन)N / D) * 100.0 * 100.0) / 100.0;
  वापस OS << format("0x%08" PRIx32 " / 0x%08" PRIx32 " = %.2f%%", N, D,
                      Percent);
}

#अगर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD व्योम BranchProbability::dump() स्थिर { print(dbgs()) << '\n'; }
#पूर्ण_अगर

BranchProbability
BranchProbability::getBranchProbability(uint64_t Numerator,
                                        uint64_t Denominator) {
  निश्चित(Numerator <= Denominator && "Probability cannot be bigger than 1!");
  // Scale down Denominator to fit in a 32-bit integer.
  पूर्णांक Scale = 0;
  जबतक (Denominator > UINT32_MAX) {
    Denominator >>= 1;
    Scale++;
  }
  वापस BranchProbability(Numerator >> Scale, Denominator);
}

BranchProbability BranchProbability::getBranchProbability(द्विगुन Prob) {
  निश्चित(0 <= Prob && Prob <= 1 && "Probability must be between 0 and 1!");
  वापस BranchProbability(std::round(Prob * D), D);
}

// If ConstD is not zero, then replace D by ConstD so that division and modulo
// operations by D can be optimized, in case this function is not inlined by the
// compiler.
ढाँचा <uint32_t ConstD>
अटल uint64_t scale(uint64_t अंक, uint32_t N, uint32_t D) {
  अगर (ConstD > 0)
    D = ConstD;

  निश्चित(D && "divide by 0");

  // Fast path for multiplying by 1.0.
  अगर (!अंक || D == N)
    वापस अंक;

  // Split Num into upper and lower parts to multiply, then recombine.
  uint64_t Product_ऊपरी = (अंक >> 32) * N;
  uint64_t Product_निचला = (अंक & UINT32_MAX) * N;

  // Split into 32-bit digits.
  uint32_t Upper32 = Product_ऊपरी >> 32;
  uint32_t Lower32 = Product_निचला & UINT32_MAX;
  uint32_t Mid32Partial = Product_ऊपरी & UINT32_MAX;
  uint32_t Mid32 = Mid32Partial + (Product_निचला >> 32);

  // Carry.
  Upper32 += Mid32 < Mid32Partial;

  uint64_t Rem = (uint64_t(Upper32) << 32) | Mid32;
  uint64_t UpperQ = Rem / D;

  // Check for overflow.
  अगर (UpperQ > UINT32_MAX)
    वापस UINT64_MAX;

  Rem = ((Rem % D) << 32) | Lower32;
  uint64_t LowerQ = Rem / D;
  uint64_t Q = (UpperQ << 32) + LowerQ;

  // Check for overflow.
  वापस Q < LowerQ ? UINT64_MAX : Q;
}

uint64_t BranchProbability::scale(uint64_t अंक) स्थिर {
  वापस ::scale<D>(अंक, N, D);
}

uint64_t BranchProbability::scaleByInverse(uint64_t अंक) स्थिर {
  वापस ::scale<0>(अंक, D, N);
}

BranchProbability BranchProbability::घात(अचिन्हित N) स्थिर {
  BranchProbability Res = BranchProbability::getOne();
  क्रम (अचिन्हित I = 0; I < N; ++I)
    Res *= *यह;
  वापस Res;
}
