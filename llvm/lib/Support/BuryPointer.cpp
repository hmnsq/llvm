//===- BuryPointer.cpp - Memory Manipulation/Leak ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/BuryPointer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Compiler.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <atomic>

ਨਾਮਾਕਾਸ਼ llvm {

ਖਾਲੀ Bury_ਸੂਚਕ(ਅਚਲ ਖਾਲੀ *Ptr) {
  // This function may be called only a small fixed amount of times per each
  // invocation, otherwise we do actually have a leak which we want to report.
  // If this function is called more than kGraveYardMaxSize times, the pointers
  // will not be properly buried and a leak detector will report a leak, which
  // is what we want in such case.
  ਸਥਿਰ ਅਚਲ size_t k_Grave_Yard_Max_ਮਾਪ = 16;
  LLVM_ATTRIBUTE_ਵਰਤਿਆ ਸਥਿਰ ਅਚਲ ਖਾਲੀ *GraveYard[k_Grave_Yard_Max_ਮਾਪ];
  ਸਥਿਰ std::atomic<ਬਿਨਾਨਿਸ਼ਾਨੀ> Grave_Yard_ਮਾਪ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ Idx = Grave_Yard_ਮਾਪ++;
  ਜੇ (Idx >= k_Grave_Yard_Max_ਮਾਪ)
    ਵਾਪਸ;
  GraveYard[Idx] = Ptr;
}

} // namespace llvm
