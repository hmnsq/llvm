//===- BuryPointer.cpp - Memory Manipulation/Leak ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/BuryPointer.h"
#अन्तर्भाव "llvm/Support/Compiler.h"
#अन्तर्भाव <atomic>

नामाकाश llvm {

शून्य BuryPointer(ध्रुव शून्य *Ptr) {
  // This function may be called only a small fixed amount of times per each
  // invocation, otherwise we do actually have a leak which we want to report.
  // If this function is called more than kGraveYardMaxSize times, the pointers
  // will not be properly buried and a leak detector will report a leak, which
  // is what we want in such case.
  स्थिर ध्रुव size_t k_Grave_Yard_Max_आकार = 16;
  LLVM_ATTRIBUTE_प्रयुक्तम् स्थिर ध्रुव शून्य *GraveYard[k_Grave_Yard_Max_आकार];
  स्थिर std::atomic<अचिह्नित> Grave_Yard_आकार;
  अचिह्नित Idx = Grave_Yard_आकार++;
  यदि (Idx >= k_Grave_Yard_Max_आकार)
    प्रत्यागम;
  GraveYard[Idx] = Ptr;
}

} // namespace llvm
