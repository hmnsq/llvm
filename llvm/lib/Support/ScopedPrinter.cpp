//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/ScopedPrinter.h"
#ضمن "llvm/Support/Format.h"

کارول نطاق llvm;

raw_ostream &llvm::چلوونکی<<(raw_ostream &OS, ثابت Hex_شمېره &ارزښت) {
  OS << "0x" << utohexstr(ارزښت.ارزښت);
  أرجع OS;
}

فراغ ScopedPrinter::print_دوګونی_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> معلومات_2, منطقي بلاک_2,
                                    uint32_t پیل_دننه_واټن) {
  إذا (معلومات_2.اندازه_2() > 16)
    بلاک_2 = ریښتیا;

  إذا (بلاک_2) {
    پیل_کرښه() << Label;
    إذا (!Str.تشه())
      OS << ": " << Str;
    OS << " (\n";
    إذا (!معلومات_2.تشه())
      OS << format_بایټونه_with_ascii(معلومات_2, پیل_دننه_واټن, 16, 4,
                                    (IndentLevel + 1) * 2, ریښتیا)
         << "\n";
    پیل_کرښه() << ")\n";
  } وإلا {
    پیل_کرښه() << Label << ":";
    إذا (!Str.تشه())
      OS << " " << Str;
    OS << " (" << format_بایټونه(معلومات_2, std::nullopt, معلومات_2.اندازه_2(), 1, 0, ریښتیا)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, منطقي PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::Scoped_Printer_ډول::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  إذا (هذا->OuterScope)
    هذا->OuterScope->سېټ_Printer(*هذا);
}
