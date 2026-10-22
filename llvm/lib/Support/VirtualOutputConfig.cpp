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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/VirtualOutputConfig.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Debug.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FileSystem.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::vfs;

OutputConfig &OutputConfig::setOpenFlags(ଅଚଳ sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  using ନାମାକାଶ llvm::sys::fs;
  ଫେରନ୍ତୁ Flags & OF_Text
             ? setText().setCRLF(Flags & OF_CRLF).setAppend(Flags & OF_Append)
             : set_ଦ୍ୱିମିକ().setAppend(Flags & OF_Append);
}

ଶୂନ୍ୟ OutputConfig::print(raw_ostream &OS) ଅଚଳ {
  OS << "{";
  ତାର୍କିକ IsFirst = ସତ୍ୟ;
  auto printFlag = [&](StringRef FlagName, ତାର୍କିକ ମୂଲ୍ୟ) {
    ଯଦି (IsFirst)
      IsFirst = ମିଥ୍ୟା;
    ଅନ୍ୟଥା
      OS << ",";
    ଯଦି (!ମୂଲ୍ୟ)
      OS << "No";
    OS << FlagName;
  };

#define HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  ଯଦି (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#ଅନ୍ତର୍ଭାଵ "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_METHOD ଶୂନ୍ୟ OutputConfig::dump() ଅଚଳ { print(dbgs()); }

raw_ostream &llvm::operator<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  ଫେରନ୍ତୁ OS;
}
