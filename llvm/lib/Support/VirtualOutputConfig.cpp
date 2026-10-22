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

کارول نطاق llvm;
کارول نطاق llvm::vfs;

OutputConfig &OutputConfig::سېټ_Open_Flags(ثابت sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  کارول نطاق llvm::sys::fs;
  أرجع Flags & د_متن
             ? سېټ_متن().سېټ_CRLF(Flags & د_CRLF).سېټ_Append(Flags & د_Append)
             : سېټ_دوګونی().سېټ_Append(Flags & د_Append);
}

فراغ OutputConfig::print(raw_ostream &OS) ثابت {
  OS << "{";
  منطقي Is_لومړی = ریښتیا;
  خپلکاری printFlag = [&](StringRef FlagName, منطقي ارزښت) {
    إذا (Is_لومړی)
      Is_لومړی = خطأ;
    وإلا
      OS << ",";
    إذا (!ارزښت)
      OS << "No";
    OS << FlagName;
  };

#تعریف_کړه HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  إذا (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#ضمن "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_لاره فراغ OutputConfig::dump() ثابت { print(dbgs()); }

raw_ostream &llvm::چلوونکی<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  أرجع OS;
}
