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

#அந்தர்பாவ "llvm/Support/VirtualOutputConfig.h"
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/FileSystem.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::vfs;

OutputConfig &OutputConfig::setOpenFlags(மாறிலி sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  using நாமாகாஶ llvm::sys::fs;
  திரும்பு Flags & OF_Text
             ? setText().setCRLF(Flags & OF_CRLF).setAppend(Flags & OF_Append)
             : set_இருமம்().setAppend(Flags & OF_Append);
}

வெறுமை OutputConfig::print(raw_ostream &OS) மாறிலி {
  OS << "{";
  தருக்க IsFirst = உண்மை;
  auto printFlag = [&](StringRef FlagName, தருக்க மதிப்பு) {
    எனில் (IsFirst)
      IsFirst = பொய்;
    இல்லையெனில்
      OS << ",";
    எனில் (!மதிப்பு)
      OS << "No";
    OS << FlagName;
  };

#define HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  எனில் (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#அந்தர்பாவ "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_METHOD வெறுமை OutputConfig::dump() மாறிலி { print(dbgs()); }

raw_ostream &llvm::operator<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  திரும்பு OS;
}
