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

raw_ostream &llvm::चालक<<(raw_ostream &OS, अचर Hex_सङ्ख्या &मान) {
  OS << "0x" << utohexstr(मान.मान);
  फर्काऊ OS;
}

शून्य ScopedPrinter::print_द्विआधारी_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> Data, तार्किक Block,
                                    uint32_t StartOffset) {
  यदि (Data.size() > 16)
    Block = सत्य;

  यदि (Block) {
    start_पङ्क्ति() << Label;
    यदि (!Str.empty())
      OS << ": " << Str;
    OS << " (\n";
    यदि (!Data.empty())
      OS << format_bytes_with_ascii(Data, StartOffset, 16, 4,
                                    (IndentLevel + 1) * 2, सत्य)
         << "\n";
    start_पङ्क्ति() << ")\n";
  } अन्यथा {
    start_पङ्क्ति() << Label << ":";
    यदि (!Str.empty())
      OS << " " << Str;
    OS << " (" << format_bytes(Data, std::nullopt, Data.size(), 1, 0, सत्य)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, तार्किक PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::ScopedPrinterKind::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  यदि (एतत्->OuterScope)
    एतत्->OuterScope->setPrinter(*एतत्);
}
