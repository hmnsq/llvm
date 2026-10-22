//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements \c OutputConfig class methods.
///
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/VirtualOutputConfig.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ llvm::vfs;

OutputConfig &OutputConfig::setOpenFlags(ಧ್ರುವ sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  using ನಾಮಾಕಾಶ llvm::sys::fs;
  ಪ್ರತ್ಯಾಗಮ Flags & OF_Text
             ? setText().setCRLF(Flags & OF_CRLF).setAppend(Flags & OF_Append)
             : set_ದ್ವಿಮಾನ().setAppend(Flags & OF_Append);
}

ಶೂನ್ಯ OutputConfig::print(raw_ostream &OS) ಧ್ರುವ {
  OS << "{";
  ತಾರ್ಕಿಕ IsFirst = ಸತ್ಯ;
  auto printFlag = [&](StringRef FlagName, ತಾರ್ಕಿಕ ಮೌಲ್ಯ) {
    ಯದಿ (IsFirst)
      IsFirst = ಅಸತ್ಯ;
    ಅನ್ಯಥಾ
      OS << ",";
    ಯದಿ (!ಮೌಲ್ಯ)
      OS << "No";
    OS << FlagName;
  };

#define HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  ಯದಿ (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#ಅನ್ತರ್ಭಾವ "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_METHOD ಶೂನ್ಯ OutputConfig::dump() ಧ್ರುವ { print(dbgs()); }

raw_ostream &llvm::operator<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  ಪ್ರತ್ಯಾಗಮ OS;
}
