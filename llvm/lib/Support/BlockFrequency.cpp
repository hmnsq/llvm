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

#అన్తర్భావ "llvm/Support/BlockFrequency.h"
#అన్తర్భావ "llvm/Support/BranchProbability.h"
#అన్తర్భావ "llvm/Support/MathExtras.h"
#అన్తర్భావ "llvm/Support/ScaledNumber.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"

ఉపయోగించి నామాకాశ llvm;

ఖండం_Frequency &ఖండం_Frequency::క్రియాకారకం*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  తిరిగి *ఇది;
}

ఖండం_Frequency ఖండం_Frequency::క్రియాకారకం*(BranchProbability Prob) స్థిరరాశి {
  ఖండం_Frequency Freq(Frequency);
  Freq *= Prob;
  తిరిగి Freq;
}

ఖండం_Frequency &ఖండం_Frequency::క్రియాకారకం/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  తిరిగి *ఇది;
}

ఖండం_Frequency ఖండం_Frequency::క్రియాకారకం/(BranchProbability Prob) స్థిరరాశి {
  ఖండం_Frequency Freq(Frequency);
  Freq /= Prob;
  తిరిగి Freq;
}

std::optional<ఖండం_Frequency> ఖండం_Frequency::mul(uint64_t గుణకం) స్థిరరాశి {
  తార్కిక అధికప్రవాహం;
  uint64_t ఫలితం_Frequency = SaturatingMultiply(Frequency, గుణకం, &అధికప్రవాహం);
  ఒకవేళ (అధికప్రవాహం)
    తిరిగి {};
  తిరిగి ఖండం_Frequency(ఫలితం_Frequency);
}

raw_ostream &llvm::క్రియాకారకం<<(raw_ostream &OS, ఖండం_Frequency Freq) {
  OS << Freq.getFrequency();
  తిరిగి OS;
}

శూన్య llvm::print_Relative_ఖండం_Freq(raw_ostream &OS, ఖండం_Frequency నమోదు_Freq,
                                  ఖండం_Frequency Freq) {
  ఒకవేళ (Freq == ఖండం_Frequency(0)) {
    OS << "0";
    తిరిగి;
  }
  ఒకవేళ (నమోదు_Freq == ఖండం_Frequency(0)) {
    OS << "<invalid BFI>";
    తిరిగి;
  }
  Scaled_సంఖ్య<uint64_t> ఖండం_2(Freq.getFrequency(), 0);
  Scaled_సంఖ్య<uint64_t> నమోదు_2(నమోదు_Freq.getFrequency(), 0);
  OS << ఖండం_2 / నమోదు_2;
}
