//===-------------- lib/Support/BranchProbability.cpp -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements Branch Probability class.
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/BranchProbability.h"
#अन्तर्भाव "llvm/Config/llvm-config.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/Format.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <cassert>
#अन्तर्भाव <cmath>

using नामाकाश llvm;

raw_ostream &BranchProbability::print(raw_ostream &OS) अचर {
  यदि (isUnknown())
    फर्काऊ OS << "?%";

  // Get a percentage rounded to two decimal digits. This avoids
  // implementation-defined rounding inside printf.
  द्विगुण Percent = rint(((द्विगुण)N / D) * 100.0 * 100.0) / 100.0;
  फर्काऊ OS << format("0x%08" PRIx32 " / 0x%08" PRIx32 " = %.2f%%", N, D,
                      Percent);
}

#यदि !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य BranchProbability::dump() अचर { print(dbgs()) << '\n'; }
#endif

BranchProbability
BranchProbability::getBranchProbability(uint64_t Numerator,
                                        uint64_t Denominator) {
  प्रतिज्ञा(Numerator <= Denominator && "Probability cannot be bigger than 1!");
  // Scale down Denominator to fit in a 32-bit integer.
  पूर्णाङ्क Scale = 0;
  जबसम्म (Denominator > UINT32_MAX) {
    Denominator >>= 1;
    Scale++;
  }
  फर्काऊ BranchProbability(Numerator >> Scale, Denominator);
}

BranchProbability BranchProbability::getBranchProbability(द्विगुण Prob) {
  प्रतिज्ञा(0 <= Prob && Prob <= 1 && "Probability must be between 0 and 1!");
  फर्काऊ BranchProbability(std::round(Prob * D), D);
}

// If ConstD is not zero, then replace D by ConstD so that division and modulo
// operations by D can be optimized, in case this function is not inlined by the
// compiler.
ढाँचा <uint32_t ConstD>
स्थिर uint64_t scale(uint64_t अङ्क, uint32_t N, uint32_t D) {
  यदि (ConstD > 0)
    D = ConstD;

  प्रतिज्ञा(D && "divide by 0");

  // Fast path for multiplying by 1.0.
  यदि (!अङ्क || D == N)
    फर्काऊ अङ्क;

  // Split Num into upper and lower parts to multiply, then recombine.
  uint64_t Product_माथिल्लो = (अङ्क >> 32) * N;
  uint64_t Product_तल्लो = (अङ्क & UINT32_MAX) * N;

  // Split into 32-bit digits.
  uint32_t Upper32 = Product_माथिल्लो >> 32;
  uint32_t Lower32 = Product_तल्लो & UINT32_MAX;
  uint32_t Mid32Partial = Product_माथिल्लो & UINT32_MAX;
  uint32_t Mid32 = Mid32Partial + (Product_तल्लो >> 32);

  // Carry.
  Upper32 += Mid32 < Mid32Partial;

  uint64_t Rem = (uint64_t(Upper32) << 32) | Mid32;
  uint64_t UpperQ = Rem / D;

  // Check for overflow.
  यदि (UpperQ > UINT32_MAX)
    फर्काऊ UINT64_MAX;

  Rem = ((Rem % D) << 32) | Lower32;
  uint64_t LowerQ = Rem / D;
  uint64_t Q = (UpperQ << 32) + LowerQ;

  // Check for overflow.
  फर्काऊ Q < LowerQ ? UINT64_MAX : Q;
}

uint64_t BranchProbability::scale(uint64_t अङ्क) अचर {
  फर्काऊ ::scale<D>(अङ्क, N, D);
}

uint64_t BranchProbability::scaleByInverse(uint64_t अङ्क) अचर {
  फर्काऊ ::scale<0>(अङ्क, D, N);
}

BranchProbability BranchProbability::घात(अचिन्हित N) अचर {
  BranchProbability Res = BranchProbability::getOne();
  लागि (अचिन्हित I = 0; I < N; ++I)
    Res *= *एतत्;
  फर्काऊ Res;
}
