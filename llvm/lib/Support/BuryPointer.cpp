//===- BuryPointer.cpp - Memory Manipulation/Leak ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/BuryPointer.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Compiler.h"
#ଅନ୍ତର୍ଭାଵ <atomic>

ନାମାକାଶ llvm {

ଶୂନ୍ୟ BuryPointer(ଅଚଳ ଶୂନ୍ୟ *Ptr) {
  // This function may be called only a small fixed amount of times per each
  // invocation, otherwise we do actually have a leak which we want to report.
  // If this function is called more than kGraveYardMaxSize times, the pointers
  // will not be properly buried and a leak detector will report a leak, which
  // is what we want in such case.
  ସ୍ଥିର ଅଚଳ size_t kGraveYardMaxSize = 16;
  LLVM_ATTRIBUTE_USED ସ୍ଥିର ଅଚଳ ଶୂନ୍ୟ *GraveYard[kGraveYardMaxSize];
  ସ୍ଥିର std::atomic<unsigned> GraveYardSize;
  unsigned Idx = GraveYardSize++;
  ଯଦି (Idx >= kGraveYardMaxSize)
    ଫେରନ୍ତୁ;
  GraveYard[Idx] = Ptr;
}

} // namespace llvm
