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
  প্রত্যাগম *এতত্;
}

BlockFrequency BlockFrequency::operator*(BranchProbability Prob) ধ্রুব {
  BlockFrequency Freq(Frequency);
  Freq *= Prob;
  প্রত্যাগম Freq;
}

BlockFrequency &BlockFrequency::operator/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  প্রত্যাগম *এতত্;
}

BlockFrequency BlockFrequency::operator/(BranchProbability Prob) ধ্রুব {
  BlockFrequency Freq(Frequency);
  Freq /= Prob;
  প্রত্যাগম Freq;
}

std::optional<BlockFrequency> BlockFrequency::mul(uint64_t Factor) ধ্রুব {
  তার্কিক Overflow;
  uint64_t ResultFrequency = SaturatingMultiply(Frequency, Factor, &Overflow);
  যদি (Overflow)
    প্রত্যাগম {};
  প্রত্যাগম BlockFrequency(ResultFrequency);
}

raw_ostream &llvm::operator<<(raw_ostream &OS, BlockFrequency Freq) {
  OS << Freq.getFrequency();
  প্রত্যাগম OS;
}

শূন্য llvm::printRelativeBlockFreq(raw_ostream &OS, BlockFrequency EntryFreq,
                                  BlockFrequency Freq) {
  যদি (Freq == BlockFrequency(0)) {
    OS << "0";
    প্রত্যাগম;
  }
  যদি (EntryFreq == BlockFrequency(0)) {
    OS << "<invalid BFI>";
    প্রত্যাগম;
  }
  Scaled_সংখ্যা<uint64_t> Block(Freq.getFrequency(), 0);
  Scaled_সংখ্যা<uint64_t> Entry(EntryFreq.getFrequency(), 0);
  OS << Block / Entry;
}
