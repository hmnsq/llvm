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

#شامل "llvm/Support/BlockFrequency.h"
#شامل "llvm/Support/BranchProbability.h"
#شامل "llvm/Support/MathExtras.h"
#شامل "llvm/Support/ScaledNumber.h"
#شامل "llvm/Support/raw_ostream.h"

استعمال نامخلا llvm;

بلاک_Frequency &بلاک_Frequency::عامل*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  واپس *یہ;
}

بلاک_Frequency بلاک_Frequency::عامل*(BranchProbability Prob) ثابت {
  بلاک_Frequency Freq(Frequency);
  Freq *= Prob;
  واپس Freq;
}

بلاک_Frequency &بلاک_Frequency::عامل/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  واپس *یہ;
}

بلاک_Frequency بلاک_Frequency::عامل/(BranchProbability Prob) ثابت {
  بلاک_Frequency Freq(Frequency);
  Freq /= Prob;
  واپس Freq;
}

std::optional<بلاک_Frequency> بلاک_Frequency::mul(uint64_t ضربی_عدد) ثابت {
  منطقی حد_سے_تجاوز;
  uint64_t نتیجہ_Frequency = SaturatingMultiply(Frequency, ضربی_عدد, &حد_سے_تجاوز);
  اگر (حد_سے_تجاوز)
    واپس {};
  واپس بلاک_Frequency(نتیجہ_Frequency);
}

raw_ostream &llvm::عامل<<(raw_ostream &OS, بلاک_Frequency Freq) {
  OS << Freq.getFrequency();
  واپس OS;
}

خالی llvm::print_Relative_بلاک_Freq(raw_ostream &OS, بلاک_Frequency اندراج_Freq,
                                  بلاک_Frequency Freq) {
  اگر (Freq == بلاک_Frequency(0)) {
    OS << "0";
    واپس;
  }
  اگر (اندراج_Freq == بلاک_Frequency(0)) {
    OS << "<invalid BFI>";
    واپس;
  }
  Scaled_نمبر<uint64_t> بلاک_2(Freq.getFrequency(), 0);
  Scaled_نمبر<uint64_t> اندراج_2(اندراج_Freq.getFrequency(), 0);
  OS << بلاک_2 / اندراج_2;
}
