//===----- lib/Support/ABIBreak.cpp - EnableABIBreakingChecks -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Config/abi-breaking.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _MSC_VER
ನಾಮಾಕಾಶ llvm {

// One of these two variables will be referenced by a symbol defined in
// llvm-config.h. We provide a link-time (or load time for DSO) failure when
// there is a mismatch in the build configuration of the API client and LLVM.
#ಆದರೆ LLVM_ENABLE_ABI_BREAKING_CHECKS
ಪೂರ್ಣಾಂಕ EnableABIBreakingChecks;
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ DisableABIBreakingChecks;
#ಆದರೆ_ಅಂತ್ಯ

} // end namespace llvm
#ಆದರೆ_ಅಂತ್ಯ
