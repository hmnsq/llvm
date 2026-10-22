//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/ScopedPrinter.h"
#include "llvm/Support/Format.h"

using namespace llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, const HexNumber &Value) {
  OS << "0x" << utohexstr(Value.Value);
  trả_về OS;
}

void ScopedPrinter::printBinaryImpl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> Data, bool Block,
                                    uint32_t StartOffset) {
  nếu (Data.size() > 16)
    Block = true;

  nếu (Block) {
    startLine() << Label;
    nếu (!Str.empty())
      OS << ": " << Str;
    OS << " (\n";
    nếu (!Data.empty())
      OS << format_bytes_with_ascii(Data, StartOffset, 16, 4,
                                    (IndentLevel + 1) * 2, true)
         << "\n";
    startLine() << ")\n";
  } không_thì {
    startLine() << Label << ":";
    nếu (!Str.empty())
      OS << " " << Str;
    OS << " (" << format_bytes(Data, std::nullopt, Data.size(), 1, 0, true)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, bool PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::ScopedPrinterKind::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  nếu (this->OuterScope)
    this->OuterScope->setPrinter(*this);
}
