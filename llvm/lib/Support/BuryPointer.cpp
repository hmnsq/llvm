//===- BuryPointer.cpp - Memory Manipulation/Leak ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/BuryPointer.h"
#অন্তর্ভাব "llvm/Support/Compiler.h"
#অন্তর্ভাব <atomic>

নামাকাশ llvm {

শূন্য BuryPointer(ধ্রুবক শূন্য *Ptr) {
  // This function may be called only a small fixed amount of times per each
  // invocation, otherwise we do actually have a leak which we want to report.
  // If this function is called more than kGraveYardMaxSize times, the pointers
  // will not be properly buried and a leak detector will report a leak, which
  // is what we want in such case.
  স্থির ধ্রুবক size_t k_Grave_Yard_Max_আকার = 16;
  LLVM_ATTRIBUTE_USED স্থির ধ্রুবক শূন্য *GraveYard[k_Grave_Yard_Max_আকার];
  স্থির std::atomic<unsigned> Grave_Yard_আকার;
  unsigned Idx = Grave_Yard_আকার++;
  যদি (Idx >= k_Grave_Yard_Max_আকার)
    ফেরত;
  GraveYard[Idx] = Ptr;
}

} // namespace llvm
