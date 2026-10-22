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

#অন্তর্ভাৱ "llvm/Support/VirtualOutputConfig.h"
#অন্তর্ভাৱ "llvm/Support/Debug.h"
#অন্তর্ভাৱ "llvm/Support/FileSystem.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"

using নামাকাশ llvm;
using নামাকাশ llvm::vfs;

OutputConfig &OutputConfig::setOpenFlags(ধ্রুৱ sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  using নামাকাশ llvm::sys::fs;
  ঘূৰাই Flags & OF_Text
             ? setText().setCRLF(Flags & OF_CRLF).setAppend(Flags & OF_Append)
             : set_দ্বিমিক().setAppend(Flags & OF_Append);
}

শূন্য OutputConfig::print(raw_ostream &OS) ধ্রুৱ {
  OS << "{";
  তার্কিক IsFirst = সঁচা;
  auto printFlag = [&](StringRef FlagName, তার্কিক মান) {
    যদি (IsFirst)
      IsFirst = মিছা;
    অন্যথা
      OS << ",";
    যদি (!মান)
      OS << "No";
    OS << FlagName;
  };

#define HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  যদি (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#অন্তর্ভাৱ "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_METHOD শূন্য OutputConfig::dump() ধ্রুৱ { print(dbgs()); }

raw_ostream &llvm::operator<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  ঘূৰাই OS;
}
