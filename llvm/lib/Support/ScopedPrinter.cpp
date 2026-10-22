//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/ScopedPrinter.h"
#ضمن "llvm/Support/Format.h"

استعمال نطاق llvm;

raw_ostream &llvm::عامل<<(raw_ostream &OS, ثابت Hex_عدد &قدر) {
  OS << "0x" << utohexstr(قدر.قدر);
  أرجع OS;
}

فراغ ScopedPrinter::print_ٻٽو_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> معلومات_2, منطقي بلاڪ_2,
                                    uint32_t شروعات_وٿي) {
  إذا (معلومات_2.سائيز_2() > 16)
    بلاڪ_2 = سچ;

  إذا (بلاڪ_2) {
    شروعات_سٽ() << Label;
    إذا (!Str.خالي_جايون())
      OS << ": " << Str;
    OS << " (\n";
    إذا (!معلومات_2.خالي_جايون())
      OS << format_بائيٽون_with_ascii(معلومات_2, شروعات_وٿي, 16, 4,
                                    (IndentLevel + 1) * 2, سچ)
         << "\n";
    شروعات_سٽ() << ")\n";
  } وإلا {
    شروعات_سٽ() << Label << ":";
    إذا (!Str.خالي_جايون())
      OS << " " << Str;
    OS << " (" << format_بائيٽون(معلومات_2, std::nullopt, معلومات_2.سائيز_2(), 1, 0, سچ)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, منطقي PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::Scoped_Printer_قسم::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  إذا (هذا->OuterScope)
    هذا->OuterScope->سيٽ_Printer(*هذا);
}
