//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/ScopedPrinter.h"
#شامل "llvm/Support/Format.h"

using فضاینام llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, ثابت Hex_عدد &مقدار) {
  OS << "0x" << utohexstr(مقدار.مقدار);
  بازگشت OS;
}

تهی ScopedPrinter::print_دودویی_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> Data, منطقی Block,
                                    uint32_t StartOffset) {
  اگر (Data.size() > 16)
    Block = درست;

  اگر (Block) {
    start_سطر() << Label;
    اگر (!Str.empty())
      OS << ": " << Str;
    OS << " (\n";
    اگر (!Data.empty())
      OS << format_bytes_with_ascii(Data, StartOffset, 16, 4,
                                    (IndentLevel + 1) * 2, درست)
         << "\n";
    start_سطر() << ")\n";
  } وگرنه {
    start_سطر() << Label << ":";
    اگر (!Str.empty())
      OS << " " << Str;
    OS << " (" << format_bytes(Data, std::nullopt, Data.size(), 1, 0, درست)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, منطقی PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::ScopedPrinterKind::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  اگر (این->OuterScope)
    این->OuterScope->setPrinter(*این);
}
