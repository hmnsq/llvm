//===----- lib/Support/ABIBreak.cpp - EnableABIBreakingChecks -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Config/abi-breaking.h"

#నిర్వచితం_కాకపోతే _MSC_VER
నామాకాశ llvm {

// One of these two variables will be referenced by a symbol defined in
// llvm-config.h. We provide a link-time (or load time for DSO) failure when
// there is a mismatch in the build configuration of the API client and LLVM.
#ఒకవేళ LLVM_ENABLE_ABI_BREAKING_CHECKS
పూర్ణాంకం EnableABIBreakingChecks;
#లేకపోతే
పూర్ణాంకం DisableABIBreakingChecks;
#అయితే_ముగింపు

} // end namespace llvm
#అయితే_ముగింపు
