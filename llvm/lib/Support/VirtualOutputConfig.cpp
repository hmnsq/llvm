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

ورتوں نطاق llvm;
ورتوں نطاق llvm::vfs;

OutputConfig &OutputConfig::سیٹ_Open_Flags(ثابت sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  ورتوں نطاق llvm::sys::fs;
  أرجع Flags & دا_متن
             ? سیٹ_متن().سیٹ_CRLF(Flags & دا_CRLF).سیٹ_Append(Flags & دا_Append)
             : سیٹ_ثنائی().سیٹ_Append(Flags & دا_Append);
}

فراغ OutputConfig::print(raw_ostream &OS) ثابت {
  OS << "{";
  منطقي Is_پہلا = سچ;
  خودکار printFlag = [&](StringRef FlagName, منطقي قدر) {
    إذا (Is_پہلا)
      Is_پہلا = خطأ;
    وإلا
      OS << ",";
    إذا (!قدر)
      OS << "No";
    OS << FlagName;
  };

#تعریف_کرو HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  إذا (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#ضمن "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_طریقہ فراغ OutputConfig::dump() ثابت { print(dbgs()); }

raw_ostream &llvm::عامل<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  أرجع OS;
}
