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

#ضمن "llvm/Support/BlockFrequency.h"
#ضمن "llvm/Support/BranchProbability.h"
#ضمن "llvm/Support/MathExtras.h"
#ضمن "llvm/Support/ScaledNumber.h"
#ضمن "llvm/Support/raw_ostream.h"

استعمال نطاق llvm;

بلاڪ_Frequency &بلاڪ_Frequency::عامل*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  أرجع *هذا;
}

بلاڪ_Frequency بلاڪ_Frequency::عامل*(BranchProbability Prob) ثابت {
  بلاڪ_Frequency Freq(Frequency);
  Freq *= Prob;
  أرجع Freq;
}

بلاڪ_Frequency &بلاڪ_Frequency::عامل/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  أرجع *هذا;
}

بلاڪ_Frequency بلاڪ_Frequency::عامل/(BranchProbability Prob) ثابت {
  بلاڪ_Frequency Freq(Frequency);
  Freq /= Prob;
  أرجع Freq;
}

std::optional<بلاڪ_Frequency> بلاڪ_Frequency::mul(uint64_t ضرب_ڪندڙ) ثابت {
  منطقي حد_کان_مٿي;
  uint64_t نتيجو_Frequency = SaturatingMultiply(Frequency, ضرب_ڪندڙ, &حد_کان_مٿي);
  إذا (حد_کان_مٿي)
    أرجع {};
  أرجع بلاڪ_Frequency(نتيجو_Frequency);
}

raw_ostream &llvm::عامل<<(raw_ostream &OS, بلاڪ_Frequency Freq) {
  OS << Freq.getFrequency();
  أرجع OS;
}

فراغ llvm::print_Relative_بلاڪ_Freq(raw_ostream &OS, بلاڪ_Frequency داخلا_Freq,
                                  بلاڪ_Frequency Freq) {
  إذا (Freq == بلاڪ_Frequency(0)) {
    OS << "0";
    أرجع;
  }
  إذا (داخلا_Freq == بلاڪ_Frequency(0)) {
    OS << "<invalid BFI>";
    أرجع;
  }
  Scaled_عدد<uint64_t> بلاڪ_2(Freq.getFrequency(), 0);
  Scaled_عدد<uint64_t> داخلا_3(داخلا_Freq.getFrequency(), 0);
  OS << بلاڪ_2 / داخلا_3;
}
