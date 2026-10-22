//===- BuryPointer.cpp - Memory Manipulation/Leak ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/BuryPointer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Compiler.h"
#ಅನ್ತರ್ಭಾವ <atomic>

ನಾಮಾಕಾಶ llvm {

ಶೂನ್ಯ Bury_ಸೂಚಕ(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *Ptr) {
  // This function may be called only a small fixed amount of times per each
  // invocation, otherwise we do actually have a leak which we want to report.
  // If this function is called more than kGraveYardMaxSize times, the pointers
  // will not be properly buried and a leak detector will report a leak, which
  // is what we want in such case.
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ size_t k_Grave_Yard_Max_ಗಾತ್ರ = 16;
  LLVM_ATTRIBUTE_ಬಳಸಿದ ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *GraveYard[k_Grave_Yard_Max_ಗಾತ್ರ];
  ಸ್ಥಿರ std::atomic<ಚಿಹ್ನೆರಹಿತ> Grave_Yard_ಗಾತ್ರ;
  ಚಿಹ್ನೆರಹಿತ Idx = Grave_Yard_ಗಾತ್ರ++;
  ಆದರೆ (Idx >= k_Grave_Yard_Max_ಗಾತ್ರ)
    ಹಿಂತಿರುಗಿ;
  GraveYard[Idx] = Ptr;
}

} // namespace llvm
