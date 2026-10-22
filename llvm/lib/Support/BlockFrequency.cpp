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

उपयुज्य नामाकाश llvm;

खण्डम्_Frequency &खण्डम्_Frequency::संक्रियकः*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  प्रत्यागम *एतत्;
}

खण्डम्_Frequency खण्डम्_Frequency::संक्रियकः*(BranchProbability Prob) ध्रुव {
  खण्डम्_Frequency Freq(Frequency);
  Freq *= Prob;
  प्रत्यागम Freq;
}

खण्डम्_Frequency &खण्डम्_Frequency::संक्रियकः/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  प्रत्यागम *एतत्;
}

खण्डम्_Frequency खण्डम्_Frequency::संक्रियकः/(BranchProbability Prob) ध्रुव {
  खण्डम्_Frequency Freq(Frequency);
  Freq /= Prob;
  प्रत्यागम Freq;
}

std::optional<खण्डम्_Frequency> खण्डम्_Frequency::mul(uint64_t गुणकः) ध्रुव {
  तार्किक अतिप्रवाहः;
  uint64_t परिणाम_Frequency = SaturatingMultiply(Frequency, गुणकः, &अतिप्रवाहः);
  यदि (अतिप्रवाहः)
    प्रत्यागम {};
  प्रत्यागम खण्डम्_Frequency(परिणाम_Frequency);
}

raw_ostream &llvm::संक्रियकः<<(raw_ostream &OS, खण्डम्_Frequency Freq) {
  OS << Freq.getFrequency();
  प्रत्यागम OS;
}

शून्य llvm::print_Relative_खण्डम्_Freq(raw_ostream &OS, खण्डम्_Frequency प्रविष्टिः_Freq,
                                  खण्डम्_Frequency Freq) {
  यदि (Freq == खण्डम्_Frequency(0)) {
    OS << "0";
    प्रत्यागम;
  }
  यदि (प्रविष्टिः_Freq == खण्डम्_Frequency(0)) {
    OS << "<invalid BFI>";
    प्रत्यागम;
  }
  Scaled_सङ्ख्या<uint64_t> खण्डम्_2(Freq.getFrequency(), 0);
  Scaled_सङ्ख्या<uint64_t> प्रविष्टिः(प्रविष्टिः_Freq.getFrequency(), 0);
  OS << खण्डम्_2 / प्रविष्टिः;
}
