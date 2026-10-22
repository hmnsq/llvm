//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/ScopedPrinter.h"
#অন্তর্ভাব "llvm/Support/Format.h"

using নামাকাশ llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, ধ্রুবক Hex_সংখ্যা &মান) {
  OS << "0x" << utohexstr(মান.মান);
  ফেরত OS;
}

শূন্য ScopedPrinter::print_দ্বিমিক_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> তথ্য_2, যৌক্তিক Block,
                                    uint32_t StartOffset) {
  যদি (তথ্য_2.আকার_3() > 16)
    Block = সত্য;

  যদি (Block) {
    start_সারি() << Label;
    যদি (!Str.empty())
      OS << ": " << Str;
    OS << " (\n";
    যদি (!তথ্য_2.empty())
      OS << format_bytes_with_ascii(তথ্য_2, StartOffset, 16, 4,
                                    (IndentLevel + 1) * 2, সত্য)
         << "\n";
    start_সারি() << ")\n";
  } অন্যথা {
    start_সারি() << Label << ":";
    যদি (!Str.empty())
      OS << " " << Str;
    OS << " (" << format_bytes(তথ্য_2, std::nullopt, তথ্য_2.আকার_3(), 1, 0, সত্য)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, যৌক্তিক PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::ScopedPrinterKind::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  যদি (এই->OuterScope)
    এই->OuterScope->setPrinter(*এই);
}
