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

#ضمن "llvm/Support/BlockFrequency.h"
#ضمن "llvm/Support/BranchProbability.h"
#ضمن "llvm/Support/MathExtras.h"
#ضمن "llvm/Support/ScaledNumber.h"
#ضمن "llvm/Support/raw_ostream.h"

using نطاق llvm;

BlockFrequency &BlockFrequency::operator*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  ܦܢܐ *هذا;
}

BlockFrequency BlockFrequency::operator*(BranchProbability Prob) ثابت {
  BlockFrequency Freq(Frequency);
  Freq *= Prob;
  ܦܢܐ Freq;
}

BlockFrequency &BlockFrequency::operator/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  ܦܢܐ *هذا;
}

BlockFrequency BlockFrequency::operator/(BranchProbability Prob) ثابت {
  BlockFrequency Freq(Frequency);
  Freq /= Prob;
  ܦܢܐ Freq;
}

std::optional<BlockFrequency> BlockFrequency::mul(uint64_t Factor) ثابت {
  منطقي Overflow;
  uint64_t ResultFrequency = SaturatingMultiply(Frequency, Factor, &Overflow);
  ܐܢ (Overflow)
    ܦܢܐ {};
  ܦܢܐ BlockFrequency(ResultFrequency);
}

raw_ostream &llvm::operator<<(raw_ostream &OS, BlockFrequency Freq) {
  OS << Freq.getFrequency();
  ܦܢܐ OS;
}

فراغ llvm::printRelativeBlockFreq(raw_ostream &OS, BlockFrequency EntryFreq,
                                  BlockFrequency Freq) {
  ܐܢ (Freq == BlockFrequency(0)) {
    OS << "0";
    ܦܢܐ;
  }
  ܐܢ (EntryFreq == BlockFrequency(0)) {
    OS << "<invalid BFI>";
    ܦܢܐ;
  }
  ScaledNumber<uint64_t> Block(Freq.getFrequency(), 0);
  ScaledNumber<uint64_t> Entry(EntryFreq.getFrequency(), 0);
  OS << Block / Entry;
}
