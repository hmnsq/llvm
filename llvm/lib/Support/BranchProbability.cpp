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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/BranchProbability.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Format.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <cmath>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

raw_ostream &BranchProbability::print(raw_ostream &OS) ਅਚਲ {
  ਜੇ (isUnknown())
    ਵਾਪਸ OS << "?%";

  // Get a percentage rounded to two decimal digits. This avoids
  // implementation-defined rounding inside printf.
  ਦੁੱਗਣਾ Percent = rint(((ਦੁੱਗਣਾ)N / D) * 100.0 * 100.0) / 100.0;
  ਵਾਪਸ OS << format("0x%08" PRIx32 " / 0x%08" PRIx32 " = %.2f%%", N, D,
                      Percent);
}

#ਜੇ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ਵਿਧੀ ਖਾਲੀ BranchProbability::dump() ਅਚਲ { print(dbgs()) << '\n'; }
#ਜੇ_ਅੰਤ

BranchProbability
BranchProbability::getBranchProbability(uint64_t Numerator,
                                        uint64_t Denominator) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Numerator <= Denominator && "Probability cannot be bigger than 1!");
  // Scale down Denominator to fit in a 32-bit integer.
  ਪੂਰਨ Scale = 0;
  ਜਦੋਂ (Denominator > UINT32_MAX) {
    Denominator >>= 1;
    Scale++;
  }
  ਵਾਪਸ BranchProbability(Numerator >> Scale, Denominator);
}

BranchProbability BranchProbability::getBranchProbability(ਦੁੱਗਣਾ Prob) {
  ਪ੍ਰਤਿਜ੍ਞਾ(0 <= Prob && Prob <= 1 && "Probability must be between 0 and 1!");
  ਵਾਪਸ BranchProbability(std::ਗੇੜਾ(Prob * D), D);
}

// If ConstD is not zero, then replace D by ConstD so that division and modulo
// operations by D can be optimized, in case this function is not inlined by the
// compiler.
ਸਾਂਚਾ <uint32_t ConstD>
ਸਥਿਰ uint64_t scale(uint64_t ਅੰਕ, uint32_t N, uint32_t D) {
  ਜੇ (ConstD > 0)
    D = ConstD;

  ਪ੍ਰਤਿਜ੍ਞਾ(D && "divide by 0");

  // Fast path for multiplying by 1.0.
  ਜੇ (!ਅੰਕ || D == N)
    ਵਾਪਸ ਅੰਕ;

  // Split Num into upper and lower parts to multiply, then recombine.
  uint64_t ਗੁਣਨਫਲ_ਉੱਚਾ = (ਅੰਕ >> 32) * N;
  uint64_t ਗੁਣਨਫਲ_ਨੀਵਾਂ = (ਅੰਕ & UINT32_MAX) * N;

  // Split into 32-bit digits.
  uint32_t Upper32 = ਗੁਣਨਫਲ_ਉੱਚਾ >> 32;
  uint32_t Lower32 = ਗੁਣਨਫਲ_ਨੀਵਾਂ & UINT32_MAX;
  uint32_t Mid32Partial = ਗੁਣਨਫਲ_ਉੱਚਾ & UINT32_MAX;
  uint32_t Mid32 = Mid32Partial + (ਗੁਣਨਫਲ_ਨੀਵਾਂ >> 32);

  // Carry.
  Upper32 += Mid32 < Mid32Partial;

  uint64_t Rem = (uint64_t(Upper32) << 32) | Mid32;
  uint64_t ਉੱਤਲਾ_Q = Rem / D;

  // Check for overflow.
  ਜੇ (ਉੱਤਲਾ_Q > UINT32_MAX)
    ਵਾਪਸ UINT64_MAX;

  Rem = ((Rem % D) << 32) | Lower32;
  uint64_t ਹੇਠਲਾ_Q = Rem / D;
  uint64_t Q = (ਉੱਤਲਾ_Q << 32) + ਹੇਠਲਾ_Q;

  // Check for overflow.
  ਵਾਪਸ Q < ਹੇਠਲਾ_Q ? UINT64_MAX : Q;
}

uint64_t BranchProbability::scale(uint64_t ਅੰਕ) ਅਚਲ {
  ਵਾਪਸ ::scale<D>(ਅੰਕ, N, D);
}

uint64_t BranchProbability::scaleByInverse(uint64_t ਅੰਕ) ਅਚਲ {
  ਵਾਪਸ ::scale<0>(ਅੰਕ, D, N);
}

BranchProbability BranchProbability::ਘਾਤ(ਬਿਨਾਨਿਸ਼ਾਨੀ N) ਅਚਲ {
  BranchProbability Res = BranchProbability::getOne();
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I < N; ++I)
    Res *= *ਇਹ;
  ਵਾਪਸ Res;
}
