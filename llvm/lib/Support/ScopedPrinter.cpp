//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/ScopedPrinter.h"
#અન્તર્ભાવ "llvm/Support/Format.h"

using નામાકાશ llvm;

raw_ostream &llvm::operator<<(raw_ostream &OS, અચલ Hex_સંખ્યા &કિંમત) {
  OS << "0x" << utohexstr(કિંમત.કિંમત);
  પાછા OS;
}

શૂન્ય ScopedPrinter::print_દ્વિઅંકી_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> Data, તાર્કિક Block,
                                    uint32_t StartOffset) {
  જો (Data.size() > 16)
    Block = સાચું;

  જો (Block) {
    start_લીટી() << Label;
    જો (!Str.empty())
      OS << ": " << Str;
    OS << " (\n";
    જો (!Data.empty())
      OS << format_bytes_with_ascii(Data, StartOffset, 16, 4,
                                    (IndentLevel + 1) * 2, સાચું)
         << "\n";
    start_લીટી() << ")\n";
  } નહીંતર {
    start_લીટી() << Label << ":";
    જો (!Str.empty())
      OS << " " << Str;
    OS << " (" << format_bytes(Data, std::nullopt, Data.size(), 1, 0, સાચું)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, તાર્કિક PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::ScopedPrinterKind::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  જો (આ->OuterScope)
    આ->OuterScope->setPrinter(*આ);
}
