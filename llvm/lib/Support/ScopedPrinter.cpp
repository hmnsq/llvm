//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/Support/ScopedPrinter.h"
#অন্তর্ভাৱ "llvm/Support/Format.h"

using নামাকাশ llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, ধ্রুৱ Hex_সংখ্যা &মান) {
  OS << "0x" << utohexstr(মান.মান);
  ঘূৰাই OS;
}

শূন্য ScopedPrinter::print_দ্বিমিক_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> Data, তার্কিক Block,
                                    uint32_t StartOffset) {
  যদি (Data.size() > 16)
    Block = সঁচা;

  যদি (Block) {
    start_সাৰি() << Label;
    যদি (!Str.empty())
      OS << ": " << Str;
    OS << " (\n";
    যদি (!Data.empty())
      OS << format_bytes_with_ascii(Data, StartOffset, 16, 4,
                                    (IndentLevel + 1) * 2, সঁচা)
         << "\n";
    start_সাৰি() << ")\n";
  } অন্যথা {
    start_সাৰি() << Label << ":";
    যদি (!Str.empty())
      OS << " " << Str;
    OS << " (" << format_bytes(Data, std::nullopt, Data.size(), 1, 0, সঁচা)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, তার্কিক PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::ScopedPrinterKind::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  যদি (এতত্->OuterScope)
    এতত্->OuterScope->setPrinter(*এতত্);
}
