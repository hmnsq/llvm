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

#include "llvm/Support/BlockFrequency.h"
#include "llvm/Support/BranchProbability.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/ScaledNumber.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

BlockFrequency &BlockFrequency::operator*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  ꦮꦭꦶ *this;
}

BlockFrequency BlockFrequency::operator*(BranchProbability Prob) const {
  BlockFrequency Freq(Frequency);
  Freq *= Prob;
  ꦮꦭꦶ Freq;
}

BlockFrequency &BlockFrequency::operator/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  ꦮꦭꦶ *this;
}

BlockFrequency BlockFrequency::operator/(BranchProbability Prob) const {
  BlockFrequency Freq(Frequency);
  Freq /= Prob;
  ꦮꦭꦶ Freq;
}

std::optional<BlockFrequency> BlockFrequency::mul(uint64_t Factor) const {
  bool Overflow;
  uint64_t ResultFrequency = SaturatingMultiply(Frequency, Factor, &Overflow);
  ꦪꦼꦤ꧀ (Overflow)
    ꦮꦭꦶ {};
  ꦮꦭꦶ BlockFrequency(ResultFrequency);
}

raw_ostream &llvm::operator<<(raw_ostream &OS, BlockFrequency Freq) {
  OS << Freq.getFrequency();
  ꦮꦭꦶ OS;
}

void llvm::printRelativeBlockFreq(raw_ostream &OS, BlockFrequency EntryFreq,
                                  BlockFrequency Freq) {
  ꦪꦼꦤ꧀ (Freq == BlockFrequency(0)) {
    OS << "0";
    ꦮꦭꦶ;
  }
  ꦪꦼꦤ꧀ (EntryFreq == BlockFrequency(0)) {
    OS << "<invalid BFI>";
    ꦮꦭꦶ;
  }
  ScaledNumber<uint64_t> Block(Freq.getFrequency(), 0);
  ScaledNumber<uint64_t> Entry(EntryFreq.getFrequency(), 0);
  OS << Block / Entry;
}
