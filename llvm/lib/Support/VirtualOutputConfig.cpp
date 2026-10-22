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

استفاده نطاق llvm;
استفاده نطاق llvm::vfs;

OutputConfig &OutputConfig::مجموعه_Open_Flags(ثابت sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  استفاده نطاق llvm::sys::fs;
  أرجع Flags & از_متن
             ? مجموعه_متن().مجموعه_CRLF(Flags & از_CRLF).مجموعه_Append(Flags & از_Append)
             : مجموعه_دودویی().مجموعه_Append(Flags & از_Append);
}

فراغ OutputConfig::print(raw_ostream &OS) ثابت {
  OS << "{";
  منطقي Is_اولی = درست;
  خودکار printFlag = [&](StringRef FlagName, منطقي مقدار) {
    إذا (Is_اولی)
      Is_اولی = خطأ;
    وإلا
      OS << ",";
    إذا (!مقدار)
      OS << "No";
    OS << FlagName;
  };

#تعریف_کن HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  إذا (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#ضمن "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_روش فراغ OutputConfig::dump() ثابت { print(dbgs()); }

raw_ostream &llvm::عملگر<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  أرجع OS;
}
