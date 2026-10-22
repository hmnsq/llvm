//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/ScopedPrinter.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Format.h"

using ನಾಮಾಕಾಶ llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, ಧ್ರುವ Hex_ಸಂಖ್ಯೆ &ಮೌಲ್ಯ) {
  OS << "0x" << utohexstr(ಮೌಲ್ಯ.ಮೌಲ್ಯ);
  ಪ್ರತ್ಯಾಗಮ OS;
}

ಶೂನ್ಯ ScopedPrinter::print_ದ್ವಿಮಾನ_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> Data, ತಾರ್ಕಿಕ Block,
                                    uint32_t StartOffset) {
  ಯದಿ (Data.size() > 16)
    Block = ಸತ್ಯ;

  ಯದಿ (Block) {
    start_ಸಾಲು() << Label;
    ಯದಿ (!Str.empty())
      OS << ": " << Str;
    OS << " (\n";
    ಯದಿ (!Data.empty())
      OS << format_bytes_with_ascii(Data, StartOffset, 16, 4,
                                    (IndentLevel + 1) * 2, ಸತ್ಯ)
         << "\n";
    start_ಸಾಲು() << ")\n";
  } ಅನ್ಯಥಾ {
    start_ಸಾಲು() << Label << ":";
    ಯದಿ (!Str.empty())
      OS << " " << Str;
    OS << " (" << format_bytes(Data, std::nullopt, Data.size(), 1, 0, ಸತ್ಯ)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, ತಾರ್ಕಿಕ PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::ScopedPrinterKind::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  ಯದಿ (ಏತತ್->OuterScope)
    ಏತತ್->OuterScope->setPrinter(*ಏತತ್);
}
