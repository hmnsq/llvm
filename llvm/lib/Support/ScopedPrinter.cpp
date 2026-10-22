//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/ScopedPrinter.h"
#ضمن "llvm/Support/Format.h"

using نطاق llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, ثابت Hex_لمب &درج) {
  OS << "0x" << utohexstr(درج.درج);
  أرجع OS;
}

فراغ ScopedPrinter::بغ_نبيو_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> Data, منطقي Block,
                                    uint32_t StartOffset) {
  إذا (Data.size() > 16)
    Block = true;

  إذا (Block) {
    start_لي() << Label;
    إذا (!Str.empty())
      OS << ": " << Str;
    OS << " (\n";
    إذا (!Data.empty())
      OS << format_bytes_with_ascii(Data, StartOffset, 16, 4,
                                    (IndentLevel + 1) * 2, true)
         << "\n";
    start_لي() << ")\n";
  } وإلا {
    start_لي() << Label << ":";
    إذا (!Str.empty())
      OS << " " << Str;
    OS << " (" << format_bytes(Data, std::nullopt, Data.size(), 1, 0, true)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, منطقي Pretty_بغ,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::ScopedPrinterKind::JSON),
      JOS(OS, /*Indent=*/Pretty_بغ ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  إذا (هذا->OuterScope)
    هذا->OuterScope->setPrinter(*هذا);
}
