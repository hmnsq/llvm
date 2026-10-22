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

استعمال نطاق llvm;
استعمال نطاق llvm::vfs;

OutputConfig &OutputConfig::سيٽ_Open_Flags(ثابت sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  استعمال نطاق llvm::sys::fs;
  أرجع Flags & جو_متن
             ? سيٽ_متن().سيٽ_CRLF(Flags & جو_CRLF).سيٽ_Append(Flags & جو_Append)
             : سيٽ_ٻٽو().سيٽ_Append(Flags & جو_Append);
}

فراغ OutputConfig::print(raw_ostream &OS) ثابت {
  OS << "{";
  منطقي Is_پهريون = سچ;
  خودڪار printFlag = [&](StringRef FlagName, منطقي قدر) {
    إذا (Is_پهريون)
      Is_پهريون = خطأ;
    وإلا
      OS << ",";
    إذا (!قدر)
      OS << "No";
    OS << FlagName;
  };

#وصف_ڪريو HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  إذا (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#ضمن "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_طريقو فراغ OutputConfig::dump() ثابت { print(dbgs()); }

raw_ostream &llvm::عامل<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  أرجع OS;
}
