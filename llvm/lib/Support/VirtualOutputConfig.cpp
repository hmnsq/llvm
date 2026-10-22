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

#अन्तर्भाव "llvm/Support/VirtualOutputConfig.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

using नामाकाश llvm;
using नामाकाश llvm::vfs;

OutputConfig &OutputConfig::setOpenFlags(ध्रुव sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  using नामाकाश llvm::sys::fs;
  प्रत्यागम Flags & OF_Text
             ? setText().setCRLF(Flags & OF_CRLF).setAppend(Flags & OF_Append)
             : set_द्विआधारी().setAppend(Flags & OF_Append);
}

शून्य OutputConfig::print(raw_ostream &OS) ध्रुव {
  OS << "{";
  तार्किक IsFirst = सत्य;
  auto printFlag = [&](StringRef FlagName, तार्किक मूल्य) {
    यदि (IsFirst)
      IsFirst = असत्य;
    अन्यथा
      OS << ",";
    यदि (!मूल्य)
      OS << "No";
    OS << FlagName;
  };

#define HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  यदि (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#अन्तर्भाव "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_METHOD शून्य OutputConfig::dump() ध्रुव { print(dbgs()); }

raw_ostream &llvm::operator<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  प्रत्यागम OS;
}
