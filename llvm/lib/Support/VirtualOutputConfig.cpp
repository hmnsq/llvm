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

#شامل "llvm/Support/VirtualOutputConfig.h"
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/raw_ostream.h"

using فضاینام llvm;
using فضاینام llvm::vfs;

OutputConfig &OutputConfig::setOpenFlags(ثابت sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  using فضاینام llvm::sys::fs;
  بازگشت Flags & OF_Text
             ? setText().setCRLF(Flags & OF_CRLF).setAppend(Flags & OF_Append)
             : set_دودویی().setAppend(Flags & OF_Append);
}

تهی OutputConfig::print(raw_ostream &OS) ثابت {
  OS << "{";
  منطقی IsFirst = درست;
  auto printFlag = [&](StringRef FlagName, منطقی مقدار) {
    اگر (IsFirst)
      IsFirst = نادرست;
    وگرنه
      OS << ",";
    اگر (!مقدار)
      OS << "No";
    OS << FlagName;
  };

#define HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  اگر (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#شامل "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_METHOD تهی OutputConfig::dump() ثابت { print(dbgs()); }

raw_ostream &llvm::operator<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  بازگشت OS;
}
