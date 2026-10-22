//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/ScopedPrinter.h"
#ضمن "llvm/Support/Format.h"

ورتوں نطاق llvm;

raw_ostream &llvm::عامل<<(raw_ostream &OS, ثابت Hex_نمبر &قدر) {
  OS << "0x" << utohexstr(قدر.قدر);
  أرجع OS;
}

فراغ ScopedPrinter::print_ثنائی_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> انکڑے_2, منطقي بلاک_2,
                                    uint32_t شروع_وتھ) {
  إذا (انکڑے_2.ناپ_2() > 16)
    بلاک_2 = سچ;

  إذا (بلاک_2) {
    شروع_سطر() << Label;
    إذا (!Str.خالی_تھاواں())
      OS << ": " << Str;
    OS << " (\n";
    إذا (!انکڑے_2.خالی_تھاواں())
      OS << format_بائٹاں_with_ascii(انکڑے_2, شروع_وتھ, 16, 4,
                                    (IndentLevel + 1) * 2, سچ)
         << "\n";
    شروع_سطر() << ")\n";
  } وإلا {
    شروع_سطر() << Label << ":";
    إذا (!Str.خالی_تھاواں())
      OS << " " << Str;
    OS << " (" << format_بائٹاں(انکڑے_2, std::nullopt, انکڑے_2.ناپ_2(), 1, 0, سچ)
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
    هذا->OuterScope->سیٹ_Printer(*هذا);
}
