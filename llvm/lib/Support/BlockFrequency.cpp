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

ورتوں نطاق llvm;

بلاک_Frequency &بلاک_Frequency::عامل*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  أرجع *هذا;
}

بلاک_Frequency بلاک_Frequency::عامل*(BranchProbability Prob) ثابت {
  بلاک_Frequency Freq(Frequency);
  Freq *= Prob;
  أرجع Freq;
}

بلاک_Frequency &بلاک_Frequency::عامل/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  أرجع *هذا;
}

بلاک_Frequency بلاک_Frequency::عامل/(BranchProbability Prob) ثابت {
  بلاک_Frequency Freq(Frequency);
  Freq /= Prob;
  أرجع Freq;
}

std::optional<بلاک_Frequency> بلاک_Frequency::mul(uint64_t ضارب) ثابت {
  منطقي حد_توں_ودھ;
  uint64_t نتیجہ_Frequency = SaturatingMultiply(Frequency, ضارب, &حد_توں_ودھ);
  إذا (حد_توں_ودھ)
    أرجع {};
  أرجع بلاک_Frequency(نتیجہ_Frequency);
}

raw_ostream &llvm::عامل<<(raw_ostream &OS, بلاک_Frequency Freq) {
  OS << Freq.getFrequency();
  أرجع OS;
}

فراغ llvm::print_Relative_بلاک_Freq(raw_ostream &OS, بلاک_Frequency اندراج_Freq,
                                  بلاک_Frequency Freq) {
  إذا (Freq == بلاک_Frequency(0)) {
    OS << "0";
    أرجع;
  }
  إذا (اندراج_Freq == بلاک_Frequency(0)) {
    OS << "<invalid BFI>";
    أرجع;
  }
  Scaled_نمبر<uint64_t> بلاک_2(Freq.getFrequency(), 0);
  Scaled_نمبر<uint64_t> اندراج(اندراج_Freq.getFrequency(), 0);
  OS << بلاک_2 / اندراج;
}
