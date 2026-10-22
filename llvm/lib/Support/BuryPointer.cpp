//===- BuryPointer.cpp - Memory Manipulation/Leak ---------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/BuryPointer.h"
#समावेश "llvm/Support/Compiler.h"
#समावेश <atomic>

namespace llvm {

व्योम BuryPointer(स्थिर व्योम *Ptr) {
  // This function may be called only a small fixed amount of times per each
  // invocation, otherwise we do actually have a leak which we want to report.
  // If this function is called more than kGraveYardMaxSize times, the pointers
  // will not be properly buried and a leak detector will report a leak, which
  // is what we want in such case.
  अटल स्थिर माप_प्रकार k_Grave_Yard_Max_आकार = 16;
  LLVM_ATTRIBUTE_USED अटल स्थिर व्योम *GraveYard[k_Grave_Yard_Max_आकार];
  अटल std::atomic<अचिन्हित> Grave_Yard_आकार;
  अचिन्हित Idx = Grave_Yard_आकार++;
  अगर (Idx >= k_Grave_Yard_Max_आकार)
    वापस;
  GraveYard[Idx] = Ptr;
}

} // namespace llvm
