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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::vfs;

OutputConfig &OutputConfig::ಗಣ_Open_Flags(ಸ್ಥಿರಾಂಕ sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  ಬಳಸಿ ನಾಮಾಕಾಶ llvm::sys::fs;
  ಹಿಂತಿರುಗಿ Flags & ದ_ಪಠ್ಯ
             ? ಗಣ_ಪಠ್ಯ().ಗಣ_CRLF(Flags & ದ_CRLF).ಗಣ_Append(Flags & ದ_Append)
             : ಗಣ_ದ್ವಿಮಾನ().ಗಣ_Append(Flags & ದ_Append);
}

ಶೂನ್ಯ OutputConfig::print(raw_ostream &OS) ಸ್ಥಿರಾಂಕ {
  OS << "{";
  ತಾರ್ಕಿಕ Is_ಮೊದಲ = ನಿಜ;
  ಸ್ವಯಂಚಾಲಿತ printFlag = [&](StringRef FlagName, ತಾರ್ಕಿಕ ಮೌಲ್ಯ) {
    ಆದರೆ (Is_ಮೊದಲ)
      Is_ಮೊದಲ = ಸುಳ್ಳು;
    ಇಲ್ಲದಿದ್ದರೆ
      OS << ",";
    ಆದರೆ (!ಮೌಲ್ಯ)
      OS << "No";
    OS << FlagName;
  };

#ವ್ಯಾಖ್ಯಾನಿಸು HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  ಆದರೆ (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#ಅನ್ತರ್ಭಾವ "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_ವಿಧಾನ ಶೂನ್ಯ OutputConfig::dump() ಸ್ಥಿರಾಂಕ { print(dbgs()); }

raw_ostream &llvm::ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  ಹಿಂತಿರುಗಿ OS;
}
