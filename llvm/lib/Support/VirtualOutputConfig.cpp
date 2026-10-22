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

#समावेश "llvm/Support/VirtualOutputConfig.h"
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/FileSystem.h"
#समावेश "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::vfs;

OutputConfig &OutputConfig::setOpenFlags(स्थिर sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  using namespace llvm::sys::fs;
  वापस Flags & OF_Text
             ? setText().setCRLF(Flags & OF_CRLF).setAppend(Flags & OF_Append)
             : set_द्विआधारी().setAppend(Flags & OF_Append);
}

व्योम OutputConfig::print(raw_ostream &OS) स्थिर {
  OS << "{";
  bool IsFirst = true;
  स्वतः printFlag = [&](StringRef FlagName, bool मूल्य) {
    अगर (IsFirst)
      IsFirst = false;
    अन्यथा
      OS << ",";
    अगर (!मूल्य)
      OS << "No";
    OS << FlagName;
  };

#घोषणा HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  अगर (लो##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, लो##NAME());
#समावेश "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_METHOD व्योम OutputConfig::dump() स्थिर { print(dbgs()); }

raw_ostream &llvm::चालक<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  वापस OS;
}
