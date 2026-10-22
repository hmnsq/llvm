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
  ܦܢܐ OS;
}

فراغ ScopedPrinter::printBinaryImpl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> Data, منطقي Block,
                                    uint32_t StartOffset) {
  ܐܢ (Data.size() > 16)
    Block = ܫܪܪܐ;

  ܐܢ (Block) {
    startLine() << Label;
    ܐܢ (!Str.empty())
      OS << ": " << Str;
    OS << " (\n";
    ܐܢ (!Data.empty())
      OS << format_bytes_with_ascii(Data, StartOffset, 16, 4,
                                    (IndentLevel + 1) * 2, ܫܪܪܐ)
         << "\n";
    startLine() << ")\n";
  } ܐܚܪܢܐ {
    startLine() << Label << ":";
    ܐܢ (!Str.empty())
      OS << " " << Str;
    OS << " (" << format_bytes(Data, std::nullopt, Data.size(), 1, 0, ܫܪܪܐ)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, منطقي PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::ScopedPrinterKind::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  ܐܢ (هذا->OuterScope)
    هذا->OuterScope->setPrinter(*هذا);
}
