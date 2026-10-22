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
  परत *हे;
}

BlockFrequency BlockFrequency::चालक*(BranchProbability Prob) स्थिरांक {
  BlockFrequency Freq(Frequency);
  Freq *= Prob;
  परत Freq;
}

BlockFrequency &BlockFrequency::चालक/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  परत *हे;
}

BlockFrequency BlockFrequency::चालक/(BranchProbability Prob) स्थिरांक {
  BlockFrequency Freq(Frequency);
  Freq /= Prob;
  परत Freq;
}

std::optional<BlockFrequency> BlockFrequency::mul(uint64_t Factor) स्थिरांक {
  तार्किक Overflow;
  uint64_t ResultFrequency = SaturatingMultiply(Frequency, Factor, &Overflow);
  जर (Overflow)
    परत {};
  परत BlockFrequency(ResultFrequency);
}

raw_ostream &llvm::चालक<<(raw_ostream &OS, BlockFrequency Freq) {
  OS << Freq.getFrequency();
  परत OS;
}

शून्य llvm::printRelativeBlockFreq(raw_ostream &OS, BlockFrequency EntryFreq,
                                  BlockFrequency Freq) {
  जर (Freq == BlockFrequency(0)) {
    OS << "0";
    परत;
  }
  जर (EntryFreq == BlockFrequency(0)) {
    OS << "<invalid BFI>";
    परत;
  }
  Scaled_संख्या<uint64_t> Block(Freq.getFrequency(), 0);
  Scaled_संख्या<uint64_t> Entry(EntryFreq.getFrequency(), 0);
  OS << Block / Entry;
}
