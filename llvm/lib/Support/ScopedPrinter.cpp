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

raw_ostream &llvm::operator<<(raw_ostream &OS, const Hex_नुमेरो &हालागा) {
  OS << "0x" << utohexstr(हालागा.हालागा);
  return OS;
}

void ScopedPrinter::इलिम्बाग_दालावाहान_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> Data, bool Block,
                                    uint32_t StartOffset) {
  if (Data.size() > 16)
    Block = true;

  if (Block) {
    start_लिन्या() << Label;
    if (!Str.empty())
      OS << ": " << Str;
    OS << " (\n";
    if (!Data.empty())
      OS << format_bytes_with_ascii(Data, StartOffset, 16, 4,
                                    (IndentLevel + 1) * 2, true)
         << "\n";
    start_लिन्या() << ")\n";
  } else {
    start_लिन्या() << Label << ":";
    if (!Str.empty())
      OS << " " << Str;
    OS << " (" << format_bytes(Data, std::nullopt, Data.size(), 1, 0, true)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, bool Pretty_इलिम्बाग,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::ScopedPrinterKind::JSON),
      JOS(OS, /*Indent=*/Pretty_इलिम्बाग ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  if (this->OuterScope)
    this->OuterScope->setPrinter(*this);
}
