//===- BuryPointer.cpp - Memory Manipulation/Leak ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/BuryPointer.h"
#شامل "llvm/Support/Compiler.h"
#شامل <atomic>

نامخلا llvm {

خالی Bury_اشارہ_گر(ثابت خالی *Ptr) {
  // This function may be called only a small fixed amount of times per each
  // invocation, otherwise we do actually have a leak which we want to report.
  // If this function is called more than kGraveYardMaxSize times, the pointers
  // will not be properly buried and a leak detector will report a leak, which
  // is what we want in such case.
  ساکن ثابت size_t k_Grave_Yard_Max_سائز = 16;
  LLVM_ATTRIBUTE_مستعمل ساکن ثابت خالی *GraveYard[k_Grave_Yard_Max_سائز];
  ساکن std::atomic<غیرعلامتی> Grave_Yard_سائز;
  غیرعلامتی Idx = Grave_Yard_سائز++;
  اگر (Idx >= k_Grave_Yard_Max_سائز)
    واپس;
  GraveYard[Idx] = Ptr;
}

} // namespace llvm
