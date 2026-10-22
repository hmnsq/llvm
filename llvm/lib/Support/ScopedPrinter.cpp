//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/ScopedPrinter.h"
#अन्तर्भाव "llvm/Support/Format.h"

उपयुज्य नामाकाश llvm;

raw_ostream &llvm::संक्रियकः<<(raw_ostream &OS, ध्रुव Hex_सङ्ख्या &मानम्) {
  OS << "0x" << utohexstr(मानम्.मानम्);
  प्रत्यागम OS;
}

शून्य ScopedPrinter::print_द्विमान_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> दत्त_2, तार्किक खण्डम्_2,
                                    uint32_t Start_विस्थापनम्) {
  यदि (दत्त_2.आकार_2() > 16)
    खण्डम्_2 = सत्य;

  यदि (खण्डम्_2) {
    start_पङ्क्तिः() << Label;
    यदि (!Str.रिक्तानि())
      OS << ": " << Str;
    OS << " (\n";
    यदि (!दत्त_2.रिक्तानि())
      OS << format_अष्टकानि_with_ascii(दत्त_2, Start_विस्थापनम्, 16, 4,
                                    (IndentLevel + 1) * 2, सत्य)
         << "\n";
    start_पङ्क्तिः() << ")\n";
  } अन्यथा {
    start_पङ्क्तिः() << Label << ":";
    यदि (!Str.रिक्तानि())
      OS << " " << Str;
    OS << " (" << format_अष्टकानि(दत्त_2, std::nullopt, दत्त_2.आकार_2(), 1, 0, सत्य)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, तार्किक PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::Scoped_Printer_विधा::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  यदि (एतत्->OuterScope)
    एतत्->OuterScope->समुच्चयः_Printer(*एतत्);
}
