//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/ScopedPrinter.h"
#अन्तर्भाव "llvm/Support/Format.h"

using नामाकाश llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, ध्रुव Hex_सङ्ख्या &मानम्) {
  OS << "0x" << utohexstr(मानम्.मानम्);
  प्रत्यागम OS;
}

शून्य ScopedPrinter::print_द्विमान_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> Data, तार्किक Block,
                                    uint32_t StartOffset) {
  जइ (Data.size() > 16)
    Block = सच्च;

  जइ (Block) {
    start_पङ्क्तिः() << Label;
    जइ (!Str.empty())
      OS << ": " << Str;
    OS << " (\n";
    जइ (!Data.empty())
      OS << format_bytes_with_ascii(Data, StartOffset, 16, 4,
                                    (IndentLevel + 1) * 2, सच्च)
         << "\n";
    start_पङ्क्तिः() << ")\n";
  } अण्णहा {
    start_पङ्क्तिः() << Label << ":";
    जइ (!Str.empty())
      OS << " " << Str;
    OS << " (" << format_bytes(Data, std::nullopt, Data.size(), 1, 0, सच्च)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, तार्किक PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::ScopedPrinterKind::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  जइ (एतत्->OuterScope)
    एतत्->OuterScope->setPrinter(*एतत्);
}
