//===- BuryPointer.cpp - Memory Manipulation/Leak ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/BuryPointer.h"
#ضمن "llvm/Support/Compiler.h"
#ضمن <atomic>

نطاق llvm {

فراغ Bury_اشاره_ګر(ثابت فراغ *Ptr) {
  // This function may be called only a small fixed amount of times per each
  // invocation, otherwise we do actually have a leak which we want to report.
  // If this function is called more than kGraveYardMaxSize times, the pointers
  // will not be properly buried and a leak detector will report a leak, which
  // is what we want in such case.
  ساكن ثابت size_t k_Grave_Yard_Max_اندازه = 16;
  LLVM_ATTRIBUTE_کارول_شوی ساكن ثابت فراغ *GraveYard[k_Grave_Yard_Max_اندازه];
  ساكن std::atomic<بینښه> Grave_Yard_اندازه;
  بینښه Idx = Grave_Yard_اندازه++;
  إذا (Idx >= k_Grave_Yard_Max_اندازه)
    أرجع;
  GraveYard[Idx] = Ptr;
}

} // namespace llvm
