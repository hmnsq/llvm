//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/ScopedPrinter.h"
#அந்தர்பாவ "llvm/Support/Format.h"

using நாமாகாஶ llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, மாறிலி Hex_எண் &மதிப்பு) {
  OS << "0x" << utohexstr(மதிப்பு.மதிப்பு);
  திரும்பு OS;
}

வெறுமை ScopedPrinter::print_இருமம்_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> Data, தருக்க Block,
                                    uint32_t StartOffset) {
  எனில் (Data.size() > 16)
    Block = உண்மை;

  எனில் (Block) {
    start_வரி() << Label;
    எனில் (!Str.empty())
      OS << ": " << Str;
    OS << " (\n";
    எனில் (!Data.empty())
      OS << format_bytes_with_ascii(Data, StartOffset, 16, 4,
                                    (IndentLevel + 1) * 2, உண்மை)
         << "\n";
    start_வரி() << ")\n";
  } இல்லையெனில் {
    start_வரி() << Label << ":";
    எனில் (!Str.empty())
      OS << " " << Str;
    OS << " (" << format_bytes(Data, std::nullopt, Data.size(), 1, 0, உண்மை)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, தருக்க PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::ScopedPrinterKind::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  எனில் (இது->OuterScope)
    இது->OuterScope->setPrinter(*இது);
}
