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

فراغ Bury_اشارو(ثابت فراغ *Ptr) {
  // This function may be called only a small fixed amount of times per each
  // invocation, otherwise we do actually have a leak which we want to report.
  // If this function is called more than kGraveYardMaxSize times, the pointers
  // will not be properly buried and a leak detector will report a leak, which
  // is what we want in such case.
  ساكن ثابت size_t k_Grave_Yard_Max_سائيز = 16;
  LLVM_ATTRIBUTE_استعمال_ٿيل ساكن ثابت فراغ *GraveYard[k_Grave_Yard_Max_سائيز];
  ساكن std::atomic<بنانشان> Grave_Yard_سائيز;
  بنانشان Idx = Grave_Yard_سائيز++;
  إذا (Idx >= k_Grave_Yard_Max_سائيز)
    أرجع;
  GraveYard[Idx] = Ptr;
}

} // namespace llvm
