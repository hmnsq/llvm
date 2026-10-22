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

#समावेश "llvm/Support/BlockFrequency.h"
#समावेश "llvm/Support/BranchProbability.h"
#समावेश "llvm/Support/MathExtras.h"
#समावेश "llvm/Support/ScaledNumber.h"
#समावेश "llvm/Support/raw_ostream.h"

using namespace llvm;

BlockFrequency &BlockFrequency::चालक*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  वापस *यह;
}

BlockFrequency BlockFrequency::चालक*(BranchProbability Prob) स्थिर {
  BlockFrequency Freq(Frequency);
  Freq *= Prob;
  वापस Freq;
}

BlockFrequency &BlockFrequency::चालक/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  वापस *यह;
}

BlockFrequency BlockFrequency::चालक/(BranchProbability Prob) स्थिर {
  BlockFrequency Freq(Frequency);
  Freq /= Prob;
  वापस Freq;
}

std::optional<BlockFrequency> BlockFrequency::mul(uint64_t Factor) स्थिर {
  bool Overflow;
  uint64_t परिणाम_Frequency = SaturatingMultiply(Frequency, Factor, &Overflow);
  अगर (Overflow)
    वापस {};
  वापस BlockFrequency(परिणाम_Frequency);
}

raw_ostream &llvm::चालक<<(raw_ostream &OS, BlockFrequency Freq) {
  OS << Freq.getFrequency();
  वापस OS;
}

व्योम llvm::printRelativeBlockFreq(raw_ostream &OS, BlockFrequency EntryFreq,
                                  BlockFrequency Freq) {
  अगर (Freq == BlockFrequency(0)) {
    OS << "0";
    वापस;
  }
  अगर (EntryFreq == BlockFrequency(0)) {
    OS << "<invalid BFI>";
    वापस;
  }
  Scaled_संख्या<uint64_t> Block(Freq.getFrequency(), 0);
  Scaled_संख्या<uint64_t> Entry(EntryFreq.getFrequency(), 0);
  OS << Block / Entry;
}
