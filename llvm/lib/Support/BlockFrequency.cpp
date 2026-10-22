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

#அந்தர்பாவ "llvm/Support/BlockFrequency.h"
#அந்தர்பாவ "llvm/Support/BranchProbability.h"
#அந்தர்பாவ "llvm/Support/MathExtras.h"
#அந்தர்பாவ "llvm/Support/ScaledNumber.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"

using நாமாகாஶ llvm;

BlockFrequency &BlockFrequency::operator*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  திரும்பு *இது;
}

BlockFrequency BlockFrequency::operator*(BranchProbability Prob) மாறிலி {
  BlockFrequency Freq(Frequency);
  Freq *= Prob;
  திரும்பு Freq;
}

BlockFrequency &BlockFrequency::operator/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  திரும்பு *இது;
}

BlockFrequency BlockFrequency::operator/(BranchProbability Prob) மாறிலி {
  BlockFrequency Freq(Frequency);
  Freq /= Prob;
  திரும்பு Freq;
}

std::optional<BlockFrequency> BlockFrequency::mul(uint64_t Factor) மாறிலி {
  தருக்க Overflow;
  uint64_t ResultFrequency = SaturatingMultiply(Frequency, Factor, &Overflow);
  எனில் (Overflow)
    திரும்பு {};
  திரும்பு BlockFrequency(ResultFrequency);
}

raw_ostream &llvm::operator<<(raw_ostream &OS, BlockFrequency Freq) {
  OS << Freq.getFrequency();
  திரும்பு OS;
}

வெறுமை llvm::printRelativeBlockFreq(raw_ostream &OS, BlockFrequency EntryFreq,
                                  BlockFrequency Freq) {
  எனில் (Freq == BlockFrequency(0)) {
    OS << "0";
    திரும்பு;
  }
  எனில் (EntryFreq == BlockFrequency(0)) {
    OS << "<invalid BFI>";
    திரும்பு;
  }
  Scaled_எண்<uint64_t> Block(Freq.getFrequency(), 0);
  Scaled_எண்<uint64_t> Entry(EntryFreq.getFrequency(), 0);
  OS << Block / Entry;
}
