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

#அந்தர்பாவ "llvm/Support/BranchProbability.h"
#அந்தர்பாவ "llvm/Config/llvm-config.h"
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/Format.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <cassert>
#அந்தர்பாவ <cmath>

using நாமாகாஶ llvm;

raw_ostream &BranchProbability::print(raw_ostream &OS) மாறிலி {
  எனில் (isUnknown())
    திரும்பு OS << "?%";

  // Get a percentage rounded to two decimal digits. This avoids
  // implementation-defined rounding inside printf.
  இரட்டை Percent = rint(((இரட்டை)N / D) * 100.0 * 100.0) / 100.0;
  திரும்பு OS << format("0x%08" PRIx32 " / 0x%08" PRIx32 " = %.2f%%", N, D,
                      Percent);
}

#எனில் !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD வெறுமை BranchProbability::dump() மாறிலி { print(dbgs()) << '\n'; }
#endif

BranchProbability
BranchProbability::getBranchProbability(uint64_t Numerator,
                                        uint64_t Denominator) {
  ப்ரதிஜ்ஞா(Numerator <= Denominator && "Probability cannot be bigger than 1!");
  // Scale down Denominator to fit in a 32-bit integer.
  முழு Scale = 0;
  போது (Denominator > UINT32_MAX) {
    Denominator >>= 1;
    Scale++;
  }
  திரும்பு BranchProbability(Numerator >> Scale, Denominator);
}

BranchProbability BranchProbability::getBranchProbability(இரட்டை Prob) {
  ப்ரதிஜ்ஞா(0 <= Prob && Prob <= 1 && "Probability must be between 0 and 1!");
  திரும்பு BranchProbability(std::round(Prob * D), D);
}

// If ConstD is not zero, then replace D by ConstD so that division and modulo
// operations by D can be optimized, in case this function is not inlined by the
// compiler.
template <uint32_t ConstD>
நிலையான uint64_t scale(uint64_t இலக்கம், uint32_t N, uint32_t D) {
  எனில் (ConstD > 0)
    D = ConstD;

  ப்ரதிஜ்ஞா(D && "divide by 0");

  // Fast path for multiplying by 1.0.
  எனில் (!இலக்கம் || D == N)
    திரும்பு இலக்கம்;

  // Split Num into upper and lower parts to multiply, then recombine.
  uint64_t Product_உயர்வு = (இலக்கம் >> 32) * N;
  uint64_t Product_கீழ் = (இலக்கம் & UINT32_MAX) * N;

  // Split into 32-bit digits.
  uint32_t Upper32 = Product_உயர்வு >> 32;
  uint32_t Lower32 = Product_கீழ் & UINT32_MAX;
  uint32_t Mid32Partial = Product_உயர்வு & UINT32_MAX;
  uint32_t Mid32 = Mid32Partial + (Product_கீழ் >> 32);

  // Carry.
  Upper32 += Mid32 < Mid32Partial;

  uint64_t Rem = (uint64_t(Upper32) << 32) | Mid32;
  uint64_t UpperQ = Rem / D;

  // Check for overflow.
  எனில் (UpperQ > UINT32_MAX)
    திரும்பு UINT64_MAX;

  Rem = ((Rem % D) << 32) | Lower32;
  uint64_t LowerQ = Rem / D;
  uint64_t Q = (UpperQ << 32) + LowerQ;

  // Check for overflow.
  திரும்பு Q < LowerQ ? UINT64_MAX : Q;
}

uint64_t BranchProbability::scale(uint64_t இலக்கம்) மாறிலி {
  திரும்பு ::scale<D>(இலக்கம், N, D);
}

uint64_t BranchProbability::scaleByInverse(uint64_t இலக்கம்) மாறிலி {
  திரும்பு ::scale<0>(இலக்கம், D, N);
}

BranchProbability BranchProbability::pow(unsigned N) மாறிலி {
  BranchProbability Res = BranchProbability::getOne();
  க்கு (unsigned I = 0; I < N; ++I)
    Res *= *இது;
  திரும்பு Res;
}
