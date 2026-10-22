//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/ScopedPrinter.h"
#అన్తర్భావ "llvm/Support/Format.h"

ఉపయోగించి నామాకాశ llvm;

raw_ostream &llvm::క్రియాకారకం<<(raw_ostream &OS, స్థిరరాశి Hex_సంఖ్య &విలువ) {
  OS << "0x" << utohexstr(విలువ.విలువ);
  తిరిగి OS;
}

శూన్య ScopedPrinter::print_ద్విమాన_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> దత్తాంశం_2, తార్కిక ఖండం_2,
                                    uint32_t ప్రారంభం_స్థానభ్రంశం) {
  ఒకవేళ (దత్తాంశం_2.కొలత_2() > 16)
    ఖండం_2 = నిజం;

  ఒకవేళ (ఖండం_2) {
    ప్రారంభం_పంక్తి() << Label;
    ఒకవేళ (!Str.ఖాళీలు())
      OS << ": " << Str;
    OS << " (\n";
    ఒకవేళ (!దత్తాంశం_2.ఖాళీలు())
      OS << format_బైట్లు_with_ascii(దత్తాంశం_2, ప్రారంభం_స్థానభ్రంశం, 16, 4,
                                    (IndentLevel + 1) * 2, నిజం)
         << "\n";
    ప్రారంభం_పంక్తి() << ")\n";
  } లేకపోతే {
    ప్రారంభం_పంక్తి() << Label << ":";
    ఒకవేళ (!Str.ఖాళీలు())
      OS << " " << Str;
    OS << " (" << format_బైట్లు(దత్తాంశం_2, std::nullopt, దత్తాంశం_2.కొలత_2(), 1, 0, నిజం)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, తార్కిక PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::Scoped_Printer_రకం::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  ఒకవేళ (ఇది->OuterScope)
    ఇది->OuterScope->సమితి_Printer(*ఇది);
}
