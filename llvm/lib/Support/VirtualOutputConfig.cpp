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

#అన్తర్భావ "llvm/Support/VirtualOutputConfig.h"
#అన్తర్భావ "llvm/Support/Debug.h"
#అన్తర్భావ "llvm/Support/FileSystem.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::vfs;

OutputConfig &OutputConfig::సమితి_Open_Flags(స్థిరరాశి sys::fs::OpenFlags &Flags) {
  // Ignore CRLF on its own as invalid.
  ఉపయోగించి నామాకాశ llvm::sys::fs;
  తిరిగి Flags & యొక్క_పాఠ్యం
             ? సమితి_పాఠ్యం().సమితి_CRLF(Flags & యొక్క_CRLF).సమితి_Append(Flags & యొక్క_Append)
             : సమితి_ద్విమాన().సమితి_Append(Flags & యొక్క_Append);
}

శూన్య OutputConfig::print(raw_ostream &OS) స్థిరరాశి {
  OS << "{";
  తార్కిక Is_మొదటి = నిజం;
  స్వయంచాలక printFlag = [&](StringRef FlagName, తార్కిక విలువ) {
    ఒకవేళ (Is_మొదటి)
      Is_మొదటి = అబద్ధం;
    లేకపోతే
      OS << ",";
    ఒకవేళ (!విలువ)
      OS << "No";
    OS << FlagName;
  };

#నిర్వచించు HANDLE_OUTPUT_CONFIG_FLAG(NAME, DEFAULT)                               \
  ఒకవేళ (get##NAME() != DEFAULT)                                                  \
    printFlag(#NAME, get##NAME());
#అన్తర్భావ "llvm/Support/VirtualOutputConfig.def"
  OS << "}";
}

LLVM_DUMP_పద్ధతి శూన్య OutputConfig::dump() స్థిరరాశి { print(dbgs()); }

raw_ostream &llvm::క్రియాకారకం<<(raw_ostream &OS, OutputConfig Config) {
  Config.print(OS);
  తిరిగి OS;
}
