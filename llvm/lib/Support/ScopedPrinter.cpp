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

raw_ostream &llvm::operator<<(raw_ostream &OS, ثابت HexNumber &Value) {
  OS << "0x" << utohexstr(Value.Value);
  أرجع OS;
}

فراغ ScopedPrinter::printBinaryImpl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> Data, منطقي Block,
                                    uint32_t StartOffset) {
  𐡄𐡍 (Data.size() > 16)
    Block = 𐡒𐡔𐡈;

  𐡄𐡍 (Block) {
    startLine() << Label;
    𐡄𐡍 (!Str.empty())
      OS << ": " << Str;
    OS << " (\n";
    𐡄𐡍 (!Data.empty())
      OS << format_bytes_with_ascii(Data, StartOffset, 16, 4,
                                    (IndentLevel + 1) * 2, 𐡒𐡔𐡈)
         << "\n";
    startLine() << ")\n";
  } وإلا {
    startLine() << Label << ":";
    𐡄𐡍 (!Str.empty())
      OS << " " << Str;
    OS << " (" << format_bytes(Data, std::nullopt, Data.size(), 1, 0, 𐡒𐡔𐡈)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, منطقي PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::ScopedPrinterKind::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  𐡄𐡍 (هذا->OuterScope)
    هذا->OuterScope->setPrinter(*هذا);
}
