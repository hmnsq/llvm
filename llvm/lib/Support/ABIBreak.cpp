//===----- lib/Support/ABIBreak.cpp - EnableABIBreakingChecks -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/abi-breaking.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _MSC_VER
ਨਾਮਾਕਾਸ਼ llvm {

// One of these two variables will be referenced by a symbol defined in
// llvm-config.h. We provide a link-time (or load time for DSO) failure when
// there is a mismatch in the build configuration of the API client and LLVM.
#ਜੇ LLVM_ENABLE_ABI_BREAKING_CHECKS
ਪੂਰਨ EnableABIBreakingChecks;
#ਨਹੀਂਤਾਂ
ਪੂਰਨ DisableABIBreakingChecks;
#ਜੇ_ਅੰਤ

} // end namespace llvm
#ਜੇ_ਅੰਤ
