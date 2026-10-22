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

کارول نطاق llvm;

بلاک_Frequency &بلاک_Frequency::چلوونکی*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  أرجع *هذا;
}

بلاک_Frequency بلاک_Frequency::چلوونکی*(BranchProbability Prob) ثابت {
  بلاک_Frequency Freq(Frequency);
  Freq *= Prob;
  أرجع Freq;
}

بلاک_Frequency &بلاک_Frequency::چلوونکی/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  أرجع *هذا;
}

بلاک_Frequency بلاک_Frequency::چلوونکی/(BranchProbability Prob) ثابت {
  بلاک_Frequency Freq(Frequency);
  Freq /= Prob;
  أرجع Freq;
}

std::optional<بلاک_Frequency> بلاک_Frequency::mul(uint64_t ضریب) ثابت {
  منطقي ډکېدنه;
  uint64_t پایله_Frequency = SaturatingMultiply(Frequency, ضریب, &ډکېدنه);
  إذا (ډکېدنه)
    أرجع {};
  أرجع بلاک_Frequency(پایله_Frequency);
}

raw_ostream &llvm::چلوونکی<<(raw_ostream &OS, بلاک_Frequency Freq) {
  OS << Freq.getFrequency();
  أرجع OS;
}

فراغ llvm::print_Relative_بلاک_Freq(raw_ostream &OS, بلاک_Frequency ثبت_Freq,
                                  بلاک_Frequency Freq) {
  إذا (Freq == بلاک_Frequency(0)) {
    OS << "0";
    أرجع;
  }
  إذا (ثبت_Freq == بلاک_Frequency(0)) {
    OS << "<invalid BFI>";
    أرجع;
  }
  Scaled_شمېره<uint64_t> بلاک_2(Freq.getFrequency(), 0);
  Scaled_شمېره<uint64_t> ثبت(ثبت_Freq.getFrequency(), 0);
  OS << بلاک_2 / ثبت;
}
