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

استعمال نامخلا llvm;
استعمال نامخلا llvm::vfs;

OutputConfig &OutputConfig::سیٹ_Open_Flags(ثابت sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  استعمال نامخلا llvm::sys::fs;
  واپس Flags & کا_متن
             ? سیٹ_متن().سیٹ_CRLF(Flags & کا_CRLF).سیٹ_Append(Flags & کا_Append)
             : سیٹ_ثنائی().سیٹ_Append(Flags & کا_Append);
}

خالی OutputConfig::print(raw_ostream &OS) ثابت {
  OS << "{";
  منطقی Is_پہلا = سچ;
  خودکار printFlag = [&](StringRef FlagName, منطقی قدر) {
    اگر (Is_پہلا)
      Is_پہلا = جھوٹ;
    ورنہ
      OS << ",";
    اگر (!قدر)
      OS << "No";
    OS << FlagName;
  };

#تعریف_کرو HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  اگر (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#شامل "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_طریقہ خالی OutputConfig::dump() ثابت { print(dbgs()); }

raw_ostream &llvm::عامل<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  واپس OS;
}
