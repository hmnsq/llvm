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

#अन्तर्भाव "llvm/Support/VirtualOutputConfig.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::vfs;

OutputConfig &OutputConfig::समुच्चयः_Open_Flags(ध्रुव sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  उपयुज्य नामाकाश llvm::sys::fs;
  प्रत्यागम Flags & सम्बन्धि_Text
             ? समुच्चयः_Text().समुच्चयः_CRLF(Flags & सम्बन्धि_CRLF).समुच्चयः_Append(Flags & सम्बन्धि_Append)
             : समुच्चयः_द्विमान().समुच्चयः_Append(Flags & सम्बन्धि_Append);
}

शून्य OutputConfig::print(raw_ostream &OS) ध्रुव {
  OS << "{";
  तार्किक IsFirst = सत्य;
  स्वचालित printFlag = [&](StringRef FlagName, तार्किक मानम्) {
    यदि (IsFirst)
      IsFirst = असत्य;
    अन्यथा
      OS << ",";
    यदि (!मानम्)
      OS << "No";
    OS << FlagName;
  };

#परिभाषय HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  यदि (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#अन्तर्भाव "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_विधानम् शून्य OutputConfig::dump() ध्रुव { print(dbgs()); }

raw_ostream &llvm::संक्रियकः<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  प्रत्यागम OS;
}
