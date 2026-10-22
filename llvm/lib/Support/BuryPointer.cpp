//===- BuryPointer.cpp - Memory Manipulation/Leak ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/BuryPointer.h"
#અન્તર્ભાવ "llvm/Support/Compiler.h"
#અન્તર્ભાવ <atomic>

નામાકાશ llvm {

શૂન્ય BuryPointer(અચલ શૂન્ય *Ptr) {
  // This function may be called only a small fixed amount of times per each
  // invocation, otherwise we do actually have a leak which we want to report.
  // If this function is called more than kGraveYardMaxSize times, the pointers
  // will not be properly buried and a leak detector will report a leak, which
  // is what we want in such case.
  સ્થિર અચલ size_t kGraveYardMaxSize = 16;
  LLVM_ATTRIBUTE_USED સ્થિર અચલ શૂન્ય *GraveYard[kGraveYardMaxSize];
  સ્થિર std::atomic<unsigned> GraveYardSize;
  unsigned Idx = GraveYardSize++;
  જો (Idx >= kGraveYardMaxSize)
    પાછા;
  GraveYard[Idx] = Ptr;
}

} // namespace llvm
