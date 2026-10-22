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

ಶೂನ್ಯ BuryPointer(ಧ್ರುವ ಶೂನ್ಯ *Ptr) {
  // This function may be called only a small fixed amount of times per each
  // invocation, otherwise we do actually have a leak which we want to report.
  // If this function is called more than kGraveYardMaxSize times, the pointers
  // will not be properly buried and a leak detector will report a leak, which
  // is what we want in such case.
  ಸ್ಥಿರ ಧ್ರುವ size_t kGraveYardMaxSize = 16;
  LLVM_ATTRIBUTE_USED ಸ್ಥಿರ ಧ್ರುವ ಶೂನ್ಯ *GraveYard[kGraveYardMaxSize];
  ಸ್ಥಿರ std::atomic<unsigned> GraveYardSize;
  unsigned Idx = GraveYardSize++;
  ಯದಿ (Idx >= kGraveYardMaxSize)
    ಪ್ರತ್ಯಾಗಮ;
  GraveYard[Idx] = Ptr;
}

} // namespace llvm
