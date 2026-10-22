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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/BlockFrequency.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/BranchProbability.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MathExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ScaledNumber.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"

using ନାମାକାଶ llvm;

BlockFrequency &BlockFrequency::operator*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  ଫେରନ୍ତୁ *ଏହି;
}

BlockFrequency BlockFrequency::operator*(BranchProbability Prob) ଅଚଳ {
  BlockFrequency Freq(Frequency);
  Freq *= Prob;
  ଫେରନ୍ତୁ Freq;
}

BlockFrequency &BlockFrequency::operator/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  ଫେରନ୍ତୁ *ଏହି;
}

BlockFrequency BlockFrequency::operator/(BranchProbability Prob) ଅଚଳ {
  BlockFrequency Freq(Frequency);
  Freq /= Prob;
  ଫେରନ୍ତୁ Freq;
}

std::optional<BlockFrequency> BlockFrequency::mul(uint64_t Factor) ଅଚଳ {
  ତାର୍କିକ Overflow;
  uint64_t ResultFrequency = SaturatingMultiply(Frequency, Factor, &Overflow);
  ଯଦି (Overflow)
    ଫେରନ୍ତୁ {};
  ଫେରନ୍ତୁ BlockFrequency(ResultFrequency);
}

raw_ostream &llvm::operator<<(raw_ostream &OS, BlockFrequency Freq) {
  OS << Freq.getFrequency();
  ଫେରନ୍ତୁ OS;
}

ଶୂନ୍ୟ llvm::printRelativeBlockFreq(raw_ostream &OS, BlockFrequency EntryFreq,
                                  BlockFrequency Freq) {
  ଯଦି (Freq == BlockFrequency(0)) {
    OS << "0";
    ଫେରନ୍ତୁ;
  }
  ଯଦି (EntryFreq == BlockFrequency(0)) {
    OS << "<invalid BFI>";
    ଫେରନ୍ତୁ;
  }
  Scaled_ସଂଖ୍ୟା<uint64_t> Block(Freq.getFrequency(), 0);
  Scaled_ସଂଖ୍ୟା<uint64_t> Entry(EntryFreq.getFrequency(), 0);
  OS << Block / Entry;
}
