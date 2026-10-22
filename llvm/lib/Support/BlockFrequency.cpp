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

#ಅನ್ತರ್ಭಾವ "llvm/Support/BlockFrequency.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/BranchProbability.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MathExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ScaledNumber.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಖಂಡ_Frequency &ಖಂಡ_Frequency::ಕ್ರಿಯಾಕಾರಕ*=(BranchProbability Prob) {
  Frequency = Prob.scale(Frequency);
  ಹಿಂತಿರುಗಿ *ಇದು;
}

ಖಂಡ_Frequency ಖಂಡ_Frequency::ಕ್ರಿಯಾಕಾರಕ*(BranchProbability Prob) ಸ್ಥಿರಾಂಕ {
  ಖಂಡ_Frequency Freq(Frequency);
  Freq *= Prob;
  ಹಿಂತಿರುಗಿ Freq;
}

ಖಂಡ_Frequency &ಖಂಡ_Frequency::ಕ್ರಿಯಾಕಾರಕ/=(BranchProbability Prob) {
  Frequency = Prob.scaleByInverse(Frequency);
  ಹಿಂತಿರುಗಿ *ಇದು;
}

ಖಂಡ_Frequency ಖಂಡ_Frequency::ಕ್ರಿಯಾಕಾರಕ/(BranchProbability Prob) ಸ್ಥಿರಾಂಕ {
  ಖಂಡ_Frequency Freq(Frequency);
  Freq /= Prob;
  ಹಿಂತಿರುಗಿ Freq;
}

std::optional<ಖಂಡ_Frequency> ಖಂಡ_Frequency::mul(uint64_t ಗುಣಕ) ಸ್ಥಿರಾಂಕ {
  ತಾರ್ಕಿಕ ಅತಿಪ್ರವಾಹ;
  uint64_t ಫಲಿತಾಂಶ_Frequency = SaturatingMultiply(Frequency, ಗುಣಕ, &ಅತಿಪ್ರವಾಹ);
  ಆದರೆ (ಅತಿಪ್ರವಾಹ)
    ಹಿಂತಿರುಗಿ {};
  ಹಿಂತಿರುಗಿ ಖಂಡ_Frequency(ಫಲಿತಾಂಶ_Frequency);
}

raw_ostream &llvm::ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &OS, ಖಂಡ_Frequency Freq) {
  OS << Freq.getFrequency();
  ಹಿಂತಿರುಗಿ OS;
}

ಶೂನ್ಯ llvm::print_Relative_ಖಂಡ_Freq(raw_ostream &OS, ಖಂಡ_Frequency ನಮೂದು_Freq,
                                  ಖಂಡ_Frequency Freq) {
  ಆದರೆ (Freq == ಖಂಡ_Frequency(0)) {
    OS << "0";
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ (ನಮೂದು_Freq == ಖಂಡ_Frequency(0)) {
    OS << "<invalid BFI>";
    ಹಿಂತಿರುಗಿ;
  }
  Scaled_ಸಂಖ್ಯೆ<uint64_t> ಖಂಡ_2(Freq.getFrequency(), 0);
  Scaled_ಸಂಖ್ಯೆ<uint64_t> ನಮೂದು(ನಮೂದು_Freq.getFrequency(), 0);
  OS << ಖಂಡ_2 / ನಮೂದು;
}
