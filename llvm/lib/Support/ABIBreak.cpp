//===----- lib/Support/ABIBreak.cpp - EnableABIBreakingChecks -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Config/abi-breaking.h"

#अगर_अघोषित _MSC_VER
namespace llvm {

// One of these two variables will be referenced by a symbol defined in
// llvm-config.h. We provide a link-time (or load time for DSO) failure when
// there is a mismatch in the build configuration of the API client and LLVM.
#अगर LLVM_ENABLE_ABI_BREAKING_CHECKS
पूर्णांक EnableABIBreakingChecks;
#अन्यथा
पूर्णांक DisableABIBreakingChecks;
#पूर्ण_अगर

} // end namespace llvm
#पूर्ण_अगर
