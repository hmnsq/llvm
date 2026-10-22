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

BlockFrequency &BlockFrequency::चालक*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  प्रत्यागम *एतत्;
}

BlockFrequency BlockFrequency::चालक*(BranchProbability Prob) ध्रुव {
  BlockFrequency Freq(Frequency);
  Freq *= Prob;
  प्रत्यागम Freq;
}

BlockFrequency &BlockFrequency::चालक/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  प्रत्यागम *एतत्;
}

BlockFrequency BlockFrequency::चालक/(BranchProbability Prob) ध्रुव {
  BlockFrequency Freq(Frequency);
  Freq /= Prob;
  प्रत्यागम Freq;
}

std::optional<BlockFrequency> BlockFrequency::mul(uint64_t Factor) ध्रुव {
  तार्किक Overflow;
  uint64_t ResultFrequency = SaturatingMultiply(Frequency, Factor, &Overflow);
  यदि (Overflow)
    प्रत्यागम {};
  प्रत्यागम BlockFrequency(ResultFrequency);
}

raw_ostream &llvm::चालक<<(raw_ostream &OS, BlockFrequency Freq) {
  OS << Freq.getFrequency();
  प्रत्यागम OS;
}

शून्य llvm::printRelativeBlockFreq(raw_ostream &OS, BlockFrequency EntryFreq,
                                  BlockFrequency Freq) {
  यदि (Freq == BlockFrequency(0)) {
    OS << "0";
    प्रत्यागम;
  }
  यदि (EntryFreq == BlockFrequency(0)) {
    OS << "<invalid BFI>";
    प्रत्यागम;
  }
  Scaled_संख्या<uint64_t> Block(Freq.getFrequency(), 0);
  Scaled_संख्या<uint64_t> Entry(EntryFreq.getFrequency(), 0);
  OS << Block / Entry;
}
