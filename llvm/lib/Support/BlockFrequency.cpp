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

#અન્તર્ભાવ "llvm/Support/BlockFrequency.h"
#અન્તર્ભાવ "llvm/Support/BranchProbability.h"
#અન્તર્ભાવ "llvm/Support/MathExtras.h"
#અન્તર્ભાવ "llvm/Support/ScaledNumber.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"

using નામાકાશ llvm;

BlockFrequency &BlockFrequency::operator*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  પાછા *આ;
}

BlockFrequency BlockFrequency::operator*(BranchProbability Prob) અચલ {
  BlockFrequency Freq(Frequency);
  Freq *= Prob;
  પાછા Freq;
}

BlockFrequency &BlockFrequency::operator/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  પાછા *આ;
}

BlockFrequency BlockFrequency::operator/(BranchProbability Prob) અચલ {
  BlockFrequency Freq(Frequency);
  Freq /= Prob;
  પાછા Freq;
}

std::optional<BlockFrequency> BlockFrequency::mul(uint64_t Factor) અચલ {
  તાર્કિક Overflow;
  uint64_t ResultFrequency = SaturatingMultiply(Frequency, Factor, &Overflow);
  જો (Overflow)
    પાછા {};
  પાછા BlockFrequency(ResultFrequency);
}

raw_ostream &llvm::operator<<(raw_ostream &OS, BlockFrequency Freq) {
  OS << Freq.getFrequency();
  પાછા OS;
}

શૂન્ય llvm::printRelativeBlockFreq(raw_ostream &OS, BlockFrequency EntryFreq,
                                  BlockFrequency Freq) {
  જો (Freq == BlockFrequency(0)) {
    OS << "0";
    પાછા;
  }
  જો (EntryFreq == BlockFrequency(0)) {
    OS << "<invalid BFI>";
    પાછા;
  }
  Scaled_સંખ્યા<uint64_t> Block(Freq.getFrequency(), 0);
  Scaled_સંખ્યા<uint64_t> Entry(EntryFreq.getFrequency(), 0);
  OS << Block / Entry;
}
