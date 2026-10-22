//====--------------- lib/Support/BlockFrequency.cpp -----------*- C++ -*-====//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements Block Frequency class.
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/BlockFrequency.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/BranchProbability.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MathExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ScaledNumber.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਖੰਡ_Frequency &ਖੰਡ_Frequency::ਸੰਚਾਲਕ*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  ਵਾਪਸ *ਇਹ;
}

ਖੰਡ_Frequency ਖੰਡ_Frequency::ਸੰਚਾਲਕ*(BranchProbability Prob) ਅਚਲ {
  ਖੰਡ_Frequency Freq(Frequency);
  Freq *= Prob;
  ਵਾਪਸ Freq;
}

ਖੰਡ_Frequency &ਖੰਡ_Frequency::ਸੰਚਾਲਕ/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  ਵਾਪਸ *ਇਹ;
}

ਖੰਡ_Frequency ਖੰਡ_Frequency::ਸੰਚਾਲਕ/(BranchProbability Prob) ਅਚਲ {
  ਖੰਡ_Frequency Freq(Frequency);
  Freq /= Prob;
  ਵਾਪਸ Freq;
}

std::optional<ਖੰਡ_Frequency> ਖੰਡ_Frequency::mul(uint64_t ਗੁਣਕ) ਅਚਲ {
  ਤਾਰਕਿਕ ਉਛਾਲ;
  uint64_t ਨਤੀਜਾ_Frequency = SaturatingMultiply(Frequency, ਗੁਣਕ, &ਉਛਾਲ);
  ਜੇ (ਉਛਾਲ)
    ਵਾਪਸ {};
  ਵਾਪਸ ਖੰਡ_Frequency(ਨਤੀਜਾ_Frequency);
}

raw_ostream &llvm::ਸੰਚਾਲਕ<<(raw_ostream &OS, ਖੰਡ_Frequency Freq) {
  OS << Freq.getFrequency();
  ਵਾਪਸ OS;
}

ਖਾਲੀ llvm::print_Relative_ਖੰਡ_Freq(raw_ostream &OS, ਖੰਡ_Frequency ਇੰਦਰਾਜ_Freq,
                                  ਖੰਡ_Frequency Freq) {
  ਜੇ (Freq == ਖੰਡ_Frequency(0)) {
    OS << "0";
    ਵਾਪਸ;
  }
  ਜੇ (ਇੰਦਰਾਜ_Freq == ਖੰਡ_Frequency(0)) {
    OS << "<invalid BFI>";
    ਵਾਪਸ;
  }
  Scaled_ਸੰਖਿਆ<uint64_t> ਖੰਡ_2(Freq.getFrequency(), 0);
  Scaled_ਸੰਖਿਆ<uint64_t> ਇੰਦਰਾਜ(ਇੰਦਰਾਜ_Freq.getFrequency(), 0);
  OS << ਖੰਡ_2 / ਇੰਦਰਾਜ;
}
