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
  أرجع *هذا;
}

BlockFrequency BlockFrequency::operator*(BranchProbability Prob) ثابت {
  BlockFrequency Freq(Frequency);
  Freq *= Prob;
  أرجع Freq;
}

BlockFrequency &BlockFrequency::operator/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  أرجع *هذا;
}

BlockFrequency BlockFrequency::operator/(BranchProbability Prob) ثابت {
  BlockFrequency Freq(Frequency);
  Freq /= Prob;
  أرجع Freq;
}

std::optional<BlockFrequency> BlockFrequency::mul(uint64_t Factor) ثابت {
  منطقي Overflow;
  uint64_t ResultFrequency = SaturatingMultiply(Frequency, Factor, &Overflow);
  إذا (Overflow)
    أرجع {};
  أرجع BlockFrequency(ResultFrequency);
}

raw_ostream &llvm::operator<<(raw_ostream &OS, BlockFrequency Freq) {
  OS << Freq.getFrequency();
  أرجع OS;
}

فراغ llvm::printRelativeBlockFreq(raw_ostream &OS, BlockFrequency EntryFreq,
                                  BlockFrequency Freq) {
  إذا (Freq == BlockFrequency(0)) {
    OS << "0";
    أرجع;
  }
  إذا (EntryFreq == BlockFrequency(0)) {
    OS << "<invalid BFI>";
    أرجع;
  }
  Scaled_نمبر<uint64_t> Block(Freq.getFrequency(), 0);
  Scaled_نمبر<uint64_t> Entry(EntryFreq.getFrequency(), 0);
  OS << Block / Entry;
}
