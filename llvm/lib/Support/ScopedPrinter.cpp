//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/ScopedPrinter.h"
#समावेश "llvm/Support/Format.h"

using namespace llvm;

raw_ostream &llvm::चालक<<(raw_ostream &OS, स्थिर Hex_संख्या &मूल्य) {
  OS << "0x" << utohexstr(मूल्य.मूल्य);
  वापस OS;
}

व्योम ScopedPrinter::print_द्विआधारी_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> तथ्य_2, bool Block,
                                    uint32_t StartOffset) {
  अगर (तथ्य_2.आकार_2() > 16)
    Block = true;

  अगर (Block) {
    start_पंक्ति() << Label;
    अगर (!Str.empty())
      OS << ": " << Str;
    OS << " (\n";
    अगर (!तथ्य_2.empty())
      OS << format_bytes_with_ascii(तथ्य_2, StartOffset, 16, 4,
                                    (IndentLevel + 1) * 2, true)
         << "\n";
    start_पंक्ति() << ")\n";
  } अन्यथा {
    start_पंक्ति() << Label << ":";
    अगर (!Str.empty())
      OS << " " << Str;
    OS << " (" << format_bytes(तथ्य_2, std::nullopt, तथ्य_2.आकार_2(), 1, 0, true)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, bool PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::ScopedPrinterKind::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  अगर (यह->OuterScope)
    यह->OuterScope->setPrinter(*यह);
}
