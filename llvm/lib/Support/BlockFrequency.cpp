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

استفاده نطاق llvm;

بلوک_Frequency &بلوک_Frequency::عملگر*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  أرجع *هذا;
}

بلوک_Frequency بلوک_Frequency::عملگر*(BranchProbability Prob) ثابت {
  بلوک_Frequency Freq(Frequency);
  Freq *= Prob;
  أرجع Freq;
}

بلوک_Frequency &بلوک_Frequency::عملگر/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  أرجع *هذا;
}

بلوک_Frequency بلوک_Frequency::عملگر/(BranchProbability Prob) ثابت {
  بلوک_Frequency Freq(Frequency);
  Freq /= Prob;
  أرجع Freq;
}

std::optional<بلوک_Frequency> بلوک_Frequency::mul(uint64_t ضریب) ثابت {
  منطقي سرریز;
  uint64_t نتیجه_Frequency = SaturatingMultiply(Frequency, ضریب, &سرریز);
  إذا (سرریز)
    أرجع {};
  أرجع بلوک_Frequency(نتیجه_Frequency);
}

raw_ostream &llvm::عملگر<<(raw_ostream &OS, بلوک_Frequency Freq) {
  OS << Freq.getFrequency();
  أرجع OS;
}

فراغ llvm::print_Relative_بلوک_Freq(raw_ostream &OS, بلوک_Frequency مدخل_Freq,
                                  بلوک_Frequency Freq) {
  إذا (Freq == بلوک_Frequency(0)) {
    OS << "0";
    أرجع;
  }
  إذا (مدخل_Freq == بلوک_Frequency(0)) {
    OS << "<invalid BFI>";
    أرجع;
  }
  Scaled_عدد<uint64_t> بلوک_2(Freq.getFrequency(), 0);
  Scaled_عدد<uint64_t> مدخل(مدخل_Freq.getFrequency(), 0);
  OS << بلوک_2 / مدخل;
}
