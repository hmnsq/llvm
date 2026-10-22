//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/ScopedPrinter.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Format.h"

using ନାମାକାଶ llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, ଅଚଳ Hex_ସଂଖ୍ୟା &ମୂଲ୍ୟ) {
  OS << "0x" << utohexstr(ମୂଲ୍ୟ.ମୂଲ୍ୟ);
  ଫେରନ୍ତୁ OS;
}

ଶୂନ୍ୟ ScopedPrinter::print_ଦ୍ୱିମିକ_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> Data, ତାର୍କିକ Block,
                                    uint32_t StartOffset) {
  ଯଦି (Data.size() > 16)
    Block = ସତ୍ୟ;

  ଯଦି (Block) {
    start_ଧାଡ଼ି() << Label;
    ଯଦି (!Str.empty())
      OS << ": " << Str;
    OS << " (\n";
    ଯଦି (!Data.empty())
      OS << format_bytes_with_ascii(Data, StartOffset, 16, 4,
                                    (IndentLevel + 1) * 2, ସତ୍ୟ)
         << "\n";
    start_ଧାଡ଼ି() << ")\n";
  } ଅନ୍ୟଥା {
    start_ଧାଡ଼ି() << Label << ":";
    ଯଦି (!Str.empty())
      OS << " " << Str;
    OS << " (" << format_bytes(Data, std::nullopt, Data.size(), 1, 0, ସତ୍ୟ)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, ତାର୍କିକ PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::ScopedPrinterKind::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  ଯଦି (ଏହି->OuterScope)
    ଏହି->OuterScope->setPrinter(*ଏହି);
}
