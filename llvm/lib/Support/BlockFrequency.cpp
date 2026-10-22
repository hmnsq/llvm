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

#अन्तर्भाव "llvm/Support/BlockFrequency.h"
#अन्तर्भाव "llvm/Support/BranchProbability.h"
#अन्तर्भाव "llvm/Support/MathExtras.h"
#अन्तर्भाव "llvm/Support/ScaledNumber.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

using नामाकाश llvm;

BlockFrequency &BlockFrequency::operator*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  पटिनिवत्त *एतत्;
}

BlockFrequency BlockFrequency::operator*(BranchProbability Prob) ध्रुव {
  BlockFrequency Freq(Frequency);
  Freq *= Prob;
  पटिनिवत्त Freq;
}

BlockFrequency &BlockFrequency::operator/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  पटिनिवत्त *एतत्;
}

BlockFrequency BlockFrequency::operator/(BranchProbability Prob) ध्रुव {
  BlockFrequency Freq(Frequency);
  Freq /= Prob;
  पटिनिवत्त Freq;
}

std::optional<BlockFrequency> BlockFrequency::mul(uint64_t Factor) ध्रुव {
  तार्किक Overflow;
  uint64_t ResultFrequency = SaturatingMultiply(Frequency, Factor, &Overflow);
  सचे (Overflow)
    पटिनिवत्त {};
  पटिनिवत्त BlockFrequency(ResultFrequency);
}

raw_ostream &llvm::operator<<(raw_ostream &OS, BlockFrequency Freq) {
  OS << Freq.getFrequency();
  पटिनिवत्त OS;
}

शून्य llvm::printRelativeBlockFreq(raw_ostream &OS, BlockFrequency EntryFreq,
                                  BlockFrequency Freq) {
  सचे (Freq == BlockFrequency(0)) {
    OS << "0";
    पटिनिवत्त;
  }
  सचे (EntryFreq == BlockFrequency(0)) {
    OS << "<invalid BFI>";
    पटिनिवत्त;
  }
  Scaled_सङ्ख्या<uint64_t> Block(Freq.getFrequency(), 0);
  Scaled_सङ्ख्या<uint64_t> Entry(EntryFreq.getFrequency(), 0);
  OS << Block / Entry;
}
