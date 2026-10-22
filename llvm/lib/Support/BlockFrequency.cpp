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
  फर्काऊ *एतत्;
}

BlockFrequency BlockFrequency::चालक*(BranchProbability Prob) अचर {
  BlockFrequency Freq(Frequency);
  Freq *= Prob;
  फर्काऊ Freq;
}

BlockFrequency &BlockFrequency::चालक/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  फर्काऊ *एतत्;
}

BlockFrequency BlockFrequency::चालक/(BranchProbability Prob) अचर {
  BlockFrequency Freq(Frequency);
  Freq /= Prob;
  फर्काऊ Freq;
}

std::optional<BlockFrequency> BlockFrequency::mul(uint64_t Factor) अचर {
  तार्किक Overflow;
  uint64_t ResultFrequency = SaturatingMultiply(Frequency, Factor, &Overflow);
  यदि (Overflow)
    फर्काऊ {};
  फर्काऊ BlockFrequency(ResultFrequency);
}

raw_ostream &llvm::चालक<<(raw_ostream &OS, BlockFrequency Freq) {
  OS << Freq.getFrequency();
  फर्काऊ OS;
}

शून्य llvm::printRelativeBlockFreq(raw_ostream &OS, BlockFrequency EntryFreq,
                                  BlockFrequency Freq) {
  यदि (Freq == BlockFrequency(0)) {
    OS << "0";
    फर्काऊ;
  }
  यदि (EntryFreq == BlockFrequency(0)) {
    OS << "<invalid BFI>";
    फर्काऊ;
  }
  Scaled_सङ्ख्या<uint64_t> Block(Freq.getFrequency(), 0);
  Scaled_सङ्ख्या<uint64_t> Entry(EntryFreq.getFrequency(), 0);
  OS << Block / Entry;
}
