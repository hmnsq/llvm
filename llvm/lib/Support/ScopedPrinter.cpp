//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/ScopedPrinter.h"
#അന്തര്ഭാവ "llvm/Support/Format.h"

using നാമാകാശ llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, സ്ഥിരം Hex_സംഖ്യ &മൂല്യം) {
  OS << "0x" << utohexstr(മൂല്യം.മൂല്യം);
  തിരികെ OS;
}

ശൂന്യം ScopedPrinter::print_ദ്വിമാന_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> Data, തർക്കം Block,
                                    uint32_t StartOffset) {
  എങ്കിൽ (Data.size() > 16)
    Block = സത്യം;

  എങ്കിൽ (Block) {
    start_വരി() << Label;
    എങ്കിൽ (!Str.empty())
      OS << ": " << Str;
    OS << " (\n";
    എങ്കിൽ (!Data.empty())
      OS << format_bytes_with_ascii(Data, StartOffset, 16, 4,
                                    (IndentLevel + 1) * 2, സത്യം)
         << "\n";
    start_വരി() << ")\n";
  } അല്ലെങ്കിൽ {
    start_വരി() << Label << ":";
    എങ്കിൽ (!Str.empty())
      OS << " " << Str;
    OS << " (" << format_bytes(Data, std::nullopt, Data.size(), 1, 0, സത്യം)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, തർക്കം PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::ScopedPrinterKind::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  എങ്കിൽ (ഇത്->OuterScope)
    ഇത്->OuterScope->setPrinter(*ഇത്);
}
