//===- BuryPointer.cpp - Memory Manipulation/Leak ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/BuryPointer.h"
#అన్తర్భావ "llvm/Support/Compiler.h"
#అన్తర్భావ <atomic>

నామాకాశ llvm {

శూన్య Bury_సూచిక(స్థిరరాశి శూన్య *Ptr) {
  // This function may be called only a small fixed amount of times per each
  // invocation, otherwise we do actually have a leak which we want to report.
  // If this function is called more than kGraveYardMaxSize times, the pointers
  // will not be properly buried and a leak detector will report a leak, which
  // is what we want in such case.
  స్థిర స్థిరరాశి size_t k_Grave_Yard_Max_కొలత = 16;
  LLVM_ATTRIBUTE_వాడిన స్థిర స్థిరరాశి శూన్య *GraveYard[k_Grave_Yard_Max_కొలత];
  స్థిర std::atomic<గుర్తులేని> Grave_Yard_కొలత;
  గుర్తులేని Idx = Grave_Yard_కొలత++;
  ఒకవేళ (Idx >= k_Grave_Yard_Max_కొలత)
    తిరిగి;
  GraveYard[Idx] = Ptr;
}

} // namespace llvm
