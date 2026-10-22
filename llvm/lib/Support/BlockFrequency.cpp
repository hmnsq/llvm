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

#شامل "llvm/Support/BlockFrequency.h"
#شامل "llvm/Support/BranchProbability.h"
#شامل "llvm/Support/MathExtras.h"
#شامل "llvm/Support/ScaledNumber.h"
#شامل "llvm/Support/raw_ostream.h"

using فضاینام llvm;

BlockFrequency &BlockFrequency::operator*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  بازگشت *این;
}

BlockFrequency BlockFrequency::operator*(BranchProbability Prob) ثابت {
  BlockFrequency Freq(Frequency);
  Freq *= Prob;
  بازگشت Freq;
}

BlockFrequency &BlockFrequency::operator/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  بازگشت *این;
}

BlockFrequency BlockFrequency::operator/(BranchProbability Prob) ثابت {
  BlockFrequency Freq(Frequency);
  Freq /= Prob;
  بازگشت Freq;
}

std::optional<BlockFrequency> BlockFrequency::mul(uint64_t Factor) ثابت {
  منطقی Overflow;
  uint64_t ResultFrequency = SaturatingMultiply(Frequency, Factor, &Overflow);
  اگر (Overflow)
    بازگشت {};
  بازگشت BlockFrequency(ResultFrequency);
}

raw_ostream &llvm::operator<<(raw_ostream &OS, BlockFrequency Freq) {
  OS << Freq.getFrequency();
  بازگشت OS;
}

تهی llvm::printRelativeBlockFreq(raw_ostream &OS, BlockFrequency EntryFreq,
                                  BlockFrequency Freq) {
  اگر (Freq == BlockFrequency(0)) {
    OS << "0";
    بازگشت;
  }
  اگر (EntryFreq == BlockFrequency(0)) {
    OS << "<invalid BFI>";
    بازگشت;
  }
  Scaled_عدد<uint64_t> Block(Freq.getFrequency(), 0);
  Scaled_عدد<uint64_t> Entry(EntryFreq.getFrequency(), 0);
  OS << Block / Entry;
}
