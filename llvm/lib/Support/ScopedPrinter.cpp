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

raw_ostream &llvm::चालक<<(raw_ostream &OS, स्थिरांक Hex_संख्या &मूल्य) {
  OS << "0x" << utohexstr(मूल्य.मूल्य);
  परत OS;
}

शून्य ScopedPrinter::print_द्विमान_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> Data, तार्किक Block,
                                    uint32_t StartOffset) {
  जर (Data.size() > 16)
    Block = सत्य;

  जर (Block) {
    start_ओळ() << Label;
    जर (!Str.empty())
      OS << ": " << Str;
    OS << " (\n";
    जर (!Data.empty())
      OS << format_bytes_with_ascii(Data, StartOffset, 16, 4,
                                    (IndentLevel + 1) * 2, सत्य)
         << "\n";
    start_ओळ() << ")\n";
  } अन्यथा {
    start_ओळ() << Label << ":";
    जर (!Str.empty())
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
  जर (हे->OuterScope)
    हे->OuterScope->setPrinter(*हे);
}
