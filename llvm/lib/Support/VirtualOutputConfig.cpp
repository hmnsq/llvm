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

#ضمن "llvm/Support/VirtualOutputConfig.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/raw_ostream.h"

using نطاق llvm;
using نطاق llvm::vfs;

OutputConfig &OutputConfig::setOpenFlags(ثابت sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  using نطاق llvm::sys::fs;
  أرجع Flags & OF_Text
             ? setText().setCRLF(Flags & OF_CRLF).setAppend(Flags & OF_Append)
             : set_دووانەیی().setAppend(Flags & OF_Append);
}

فراغ OutputConfig::print(raw_ostream &OS) ثابت {
  OS << "{";
  منطقي IsFirst = true;
  auto printFlag = [&](StringRef FlagName, منطقي بەها) {
    إذا (IsFirst)
      IsFirst = خطأ;
    وإلا
      OS << ",";
    إذا (!بەها)
      OS << "No";
    OS << FlagName;
  };

#define HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  إذا (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#ضمن "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_METHOD فراغ OutputConfig::dump() ثابت { print(dbgs()); }

raw_ostream &llvm::operator<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  أرجع OS;
}
