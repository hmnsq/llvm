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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/VirtualOutputConfig.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::vfs;

OutputConfig &OutputConfig::ਸੈੱਟ_Open_Flags(ਅਚਲ sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::sys::fs;
  ਵਾਪਸ Flags & ਦਾ_ਪਾਠ
             ? ਸੈੱਟ_ਪਾਠ().ਸੈੱਟ_CRLF(Flags & ਦਾ_CRLF).ਸੈੱਟ_Append(Flags & ਦਾ_Append)
             : ਸੈੱਟ_ਦੁਆਧਾਰੀ().ਸੈੱਟ_Append(Flags & ਦਾ_Append);
}

ਖਾਲੀ OutputConfig::print(raw_ostream &OS) ਅਚਲ {
  OS << "{";
  ਤਾਰਕਿਕ Is_ਪਹਿਲਾ = ਸੱਚ;
  ਸਵੈਚਾਲਿਤ printFlag = [&](StringRef FlagName, ਤਾਰਕਿਕ ਮੁੱਲ) {
    ਜੇ (Is_ਪਹਿਲਾ)
      Is_ਪਹਿਲਾ = ਝੂਠ;
    ਨਹੀਂਤਾਂ
      OS << ",";
    ਜੇ (!ਮੁੱਲ)
      OS << "No";
    OS << FlagName;
  };

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  ਜੇ (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_ਵਿਧੀ ਖਾਲੀ OutputConfig::dump() ਅਚਲ { print(dbgs()); }

raw_ostream &llvm::ਸੰਚਾਲਕ<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  ਵਾਪਸ OS;
}
