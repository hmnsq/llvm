//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/ScopedPrinter.h"
#شامل "llvm/Support/Format.h"

استعمال نامخلا llvm;

raw_ostream &llvm::عامل<<(raw_ostream &OS, ثابت Hex_نمبر &قدر) {
  OS << "0x" << utohexstr(قدر.قدر);
  واپس OS;
}

خالی ScopedPrinter::print_ثنائی_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> ڈیٹا_2, منطقی بلاک_2,
                                    uint32_t آغاز_انحراف) {
  اگر (ڈیٹا_2.سائز_2() > 16)
    بلاک_2 = سچ;

  اگر (بلاک_2) {
    آغاز_سطر() << Label;
    اگر (!Str.خالی_خانے())
      OS << ": " << Str;
    OS << " (\n";
    اگر (!ڈیٹا_2.خالی_خانے())
      OS << format_بائٹیں_with_ascii(ڈیٹا_2, آغاز_انحراف, 16, 4,
                                    (IndentLevel + 1) * 2, سچ)
         << "\n";
    آغاز_سطر() << ")\n";
  } ورنہ {
    آغاز_سطر() << Label << ":";
    اگر (!Str.خالی_خانے())
      OS << " " << Str;
    OS << " (" << format_بائٹیں(ڈیٹا_2, std::nullopt, ڈیٹا_2.سائز_2(), 1, 0, سچ)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, منطقی PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::Scoped_Printer_قسم::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  اگر (یہ->OuterScope)
    یہ->OuterScope->سیٹ_Printer(*یہ);
}
