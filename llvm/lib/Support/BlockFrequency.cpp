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

#অন্তর্ভাব "llvm/Support/BlockFrequency.h"
#অন্তর্ভাব "llvm/Support/BranchProbability.h"
#অন্তর্ভাব "llvm/Support/MathExtras.h"
#অন্তর্ভাব "llvm/Support/ScaledNumber.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"

using নামাকাশ llvm;

BlockFrequency &BlockFrequency::operator*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  ফেরত *এই;
}

BlockFrequency BlockFrequency::operator*(BranchProbability Prob) ধ্রুবক {
  BlockFrequency Freq(Frequency);
  Freq *= Prob;
  ফেরত Freq;
}

BlockFrequency &BlockFrequency::operator/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  ফেরত *এই;
}

BlockFrequency BlockFrequency::operator/(BranchProbability Prob) ধ্রুবক {
  BlockFrequency Freq(Frequency);
  Freq /= Prob;
  ফেরত Freq;
}

std::optional<BlockFrequency> BlockFrequency::mul(uint64_t Factor) ধ্রুবক {
  যৌক্তিক Overflow;
  uint64_t ফলাফল_Frequency = SaturatingMultiply(Frequency, Factor, &Overflow);
  যদি (Overflow)
    ফেরত {};
  ফেরত BlockFrequency(ফলাফল_Frequency);
}

raw_ostream &llvm::operator<<(raw_ostream &OS, BlockFrequency Freq) {
  OS << Freq.getFrequency();
  ফেরত OS;
}

শূন্য llvm::printRelativeBlockFreq(raw_ostream &OS, BlockFrequency EntryFreq,
                                  BlockFrequency Freq) {
  যদি (Freq == BlockFrequency(0)) {
    OS << "0";
    ফেরত;
  }
  যদি (EntryFreq == BlockFrequency(0)) {
    OS << "<invalid BFI>";
    ফেরত;
  }
  Scaled_সংখ্যা<uint64_t> Block(Freq.getFrequency(), 0);
  Scaled_সংখ্যা<uint64_t> Entry(EntryFreq.getFrequency(), 0);
  OS << Block / Entry;
}
