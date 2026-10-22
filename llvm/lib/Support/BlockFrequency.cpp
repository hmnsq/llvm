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

#അന്തര്ഭാവ "llvm/Support/BlockFrequency.h"
#അന്തര്ഭാവ "llvm/Support/BranchProbability.h"
#അന്തര്ഭാവ "llvm/Support/MathExtras.h"
#അന്തര്ഭാവ "llvm/Support/ScaledNumber.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"

using നാമാകാശ llvm;

BlockFrequency &BlockFrequency::operator*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  തിരികെ *ഇത്;
}

BlockFrequency BlockFrequency::operator*(BranchProbability Prob) സ്ഥിരം {
  BlockFrequency Freq(Frequency);
  Freq *= Prob;
  തിരികെ Freq;
}

BlockFrequency &BlockFrequency::operator/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  തിരികെ *ഇത്;
}

BlockFrequency BlockFrequency::operator/(BranchProbability Prob) സ്ഥിരം {
  BlockFrequency Freq(Frequency);
  Freq /= Prob;
  തിരികെ Freq;
}

std::optional<BlockFrequency> BlockFrequency::mul(uint64_t Factor) സ്ഥിരം {
  തർക്കം Overflow;
  uint64_t ResultFrequency = SaturatingMultiply(Frequency, Factor, &Overflow);
  എങ്കിൽ (Overflow)
    തിരികെ {};
  തിരികെ BlockFrequency(ResultFrequency);
}

raw_ostream &llvm::operator<<(raw_ostream &OS, BlockFrequency Freq) {
  OS << Freq.getFrequency();
  തിരികെ OS;
}

ശൂന്യം llvm::printRelativeBlockFreq(raw_ostream &OS, BlockFrequency EntryFreq,
                                  BlockFrequency Freq) {
  എങ്കിൽ (Freq == BlockFrequency(0)) {
    OS << "0";
    തിരികെ;
  }
  എങ്കിൽ (EntryFreq == BlockFrequency(0)) {
    OS << "<invalid BFI>";
    തിരികെ;
  }
  Scaled_സംഖ്യ<uint64_t> Block(Freq.getFrequency(), 0);
  Scaled_സംഖ്യ<uint64_t> Entry(EntryFreq.getFrequency(), 0);
  OS << Block / Entry;
}
