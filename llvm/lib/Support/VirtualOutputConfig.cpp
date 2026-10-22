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

#অন্তর্ভাব "llvm/Support/VirtualOutputConfig.h"
#অন্তর্ভাব "llvm/Support/Debug.h"
#অন্তর্ভাব "llvm/Support/FileSystem.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"

using নামাকাশ llvm;
using নামাকাশ llvm::vfs;

OutputConfig &OutputConfig::setOpenFlags(ধ্রুবক sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  using নামাকাশ llvm::sys::fs;
  ফেরত Flags & OF_Text
             ? setText().setCRLF(Flags & OF_CRLF).setAppend(Flags & OF_Append)
             : set_দ্বিমিক().setAppend(Flags & OF_Append);
}

শূন্য OutputConfig::print(raw_ostream &OS) ধ্রুবক {
  OS << "{";
  যৌক্তিক IsFirst = সত্য;
  auto printFlag = [&](StringRef FlagName, যৌক্তিক মান) {
    যদি (IsFirst)
      IsFirst = মিথ্যা;
    অন্যথা
      OS << ",";
    যদি (!মান)
      OS << "No";
    OS << FlagName;
  };

#define HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  যদি (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#অন্তর্ভাব "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_METHOD শূন্য OutputConfig::dump() ধ্রুবক { print(dbgs()); }

raw_ostream &llvm::operator<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  ফেরত OS;
}
