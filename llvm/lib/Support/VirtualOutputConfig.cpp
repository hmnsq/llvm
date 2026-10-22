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

#അന്തര്ഭാവ "llvm/Support/VirtualOutputConfig.h"
#അന്തര്ഭാവ "llvm/Support/Debug.h"
#അന്തര്ഭാവ "llvm/Support/FileSystem.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"

using നാമാകാശ llvm;
using നാമാകാശ llvm::vfs;

OutputConfig &OutputConfig::setOpenFlags(സ്ഥിരം sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  using നാമാകാശ llvm::sys::fs;
  തിരികെ Flags & OF_Text
             ? setText().setCRLF(Flags & OF_CRLF).setAppend(Flags & OF_Append)
             : set_ദ്വിമാന().setAppend(Flags & OF_Append);
}

ശൂന്യം OutputConfig::print(raw_ostream &OS) സ്ഥിരം {
  OS << "{";
  തർക്കം IsFirst = സത്യം;
  auto printFlag = [&](StringRef FlagName, തർക്കം മൂല്യം) {
    എങ്കിൽ (IsFirst)
      IsFirst = അസത്യം;
    അല്ലെങ്കിൽ
      OS << ",";
    എങ്കിൽ (!മൂല്യം)
      OS << "No";
    OS << FlagName;
  };

#define HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  എങ്കിൽ (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#അന്തര്ഭാവ "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_METHOD ശൂന്യം OutputConfig::dump() സ്ഥിരം { print(dbgs()); }

raw_ostream &llvm::operator<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  തിരികെ OS;
}
