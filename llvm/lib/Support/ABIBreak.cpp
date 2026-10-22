//===----- lib/Support/ABIBreak.cpp - EnableABIBreakingChecks -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/Config/abi-breaking.h"

#ifndef _MSC_VER
নামাকাশ llvm {

// One of these two variables will be referenced by a symbol defined in
// llvm-config.h. We provide a link-time (or load time for DSO) failure when
// there is a mismatch in the build configuration of the API client and LLVM.
#যদি LLVM_ENABLE_ABI_BREAKING_CHECKS
পূর্ণাঙ্ক EnableABIBreakingChecks;
#অন্যথা
পূর্ণাঙ্ক DisableABIBreakingChecks;
#endif

} // end namespace llvm
#endif
