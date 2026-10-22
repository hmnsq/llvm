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

#અન્તર્ભાવ "llvm/Support/VirtualOutputConfig.h"
#અન્તર્ભાવ "llvm/Support/Debug.h"
#અન્તર્ભાવ "llvm/Support/FileSystem.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"

using નામાકાશ llvm;
using નામાકાશ llvm::vfs;

OutputConfig &OutputConfig::setOpenFlags(અચલ sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  using નામાકાશ llvm::sys::fs;
  પાછા Flags & OF_Text
             ? setText().setCRLF(Flags & OF_CRLF).setAppend(Flags & OF_Append)
             : set_દ્વિઅંકી().setAppend(Flags & OF_Append);
}

શૂન્ય OutputConfig::print(raw_ostream &OS) અચલ {
  OS << "{";
  તાર્કિક IsFirst = સાચું;
  auto printFlag = [&](StringRef FlagName, તાર્કિક કિંમત) {
    જો (IsFirst)
      IsFirst = ખોટું;
    નહીંતર
      OS << ",";
    જો (!કિંમત)
      OS << "No";
    OS << FlagName;
  };

#define HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  જો (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#અન્તર્ભાવ "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_METHOD શૂન્ય OutputConfig::dump() અચલ { print(dbgs()); }

raw_ostream &llvm::operator<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  પાછા OS;
}
