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

#ಅನ್ತರ್ಭಾವ "llvm/Support/BlockFrequency.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/BranchProbability.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MathExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ScaledNumber.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"

using ನಾಮಾಕಾಶ llvm;

BlockFrequency &BlockFrequency::operator*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

BlockFrequency BlockFrequency::operator*(BranchProbability Prob) ಧ್ರುವ {
  BlockFrequency Freq(Frequency);
  Freq *= Prob;
  ಪ್ರತ್ಯಾಗಮ Freq;
}

BlockFrequency &BlockFrequency::operator/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

BlockFrequency BlockFrequency::operator/(BranchProbability Prob) ಧ್ರುವ {
  BlockFrequency Freq(Frequency);
  Freq /= Prob;
  ಪ್ರತ್ಯಾಗಮ Freq;
}

std::optional<BlockFrequency> BlockFrequency::mul(uint64_t Factor) ಧ್ರುವ {
  ತಾರ್ಕಿಕ Overflow;
  uint64_t ResultFrequency = SaturatingMultiply(Frequency, Factor, &Overflow);
  ಯದಿ (Overflow)
    ಪ್ರತ್ಯಾಗಮ {};
  ಪ್ರತ್ಯಾಗಮ BlockFrequency(ResultFrequency);
}

raw_ostream &llvm::operator<<(raw_ostream &OS, BlockFrequency Freq) {
  OS << Freq.getFrequency();
  ಪ್ರತ್ಯಾಗಮ OS;
}

ಶೂನ್ಯ llvm::printRelativeBlockFreq(raw_ostream &OS, BlockFrequency EntryFreq,
                                  BlockFrequency Freq) {
  ಯದಿ (Freq == BlockFrequency(0)) {
    OS << "0";
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ (EntryFreq == BlockFrequency(0)) {
    OS << "<invalid BFI>";
    ಪ್ರತ್ಯಾಗಮ;
  }
  Scaled_ಸಂಖ್ಯೆ<uint64_t> Block(Freq.getFrequency(), 0);
  Scaled_ಸಂಖ್ಯೆ<uint64_t> Entry(EntryFreq.getFrequency(), 0);
  OS << Block / Entry;
}
