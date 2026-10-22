//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/ScopedPrinter.h"
#ضمن "llvm/Support/Format.h"

استفاده نطاق llvm;

raw_ostream &llvm::عملگر<<(raw_ostream &OS, ثابت Hex_عدد &مقدار) {
  OS << "0x" << utohexstr(مقدار.مقدار);
  أرجع OS;
}

فراغ ScopedPrinter::print_دودویی_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> داده_2, منطقي بلوک_2,
                                    uint32_t شروع_جابه_جایی) {
  إذا (داده_2.اندازه_2() > 16)
    بلوک_2 = درست;

  إذا (بلوک_2) {
    شروع_سطر() << Label;
    إذا (!Str.خالی())
      OS << ": " << Str;
    OS << " (\n";
    إذا (!داده_2.خالی())
      OS << format_بایتها_with_ascii(داده_2, شروع_جابه_جایی, 16, 4,
                                    (IndentLevel + 1) * 2, درست)
         << "\n";
    شروع_سطر() << ")\n";
  } وإلا {
    شروع_سطر() << Label << ":";
    إذا (!Str.خالی())
      OS << " " << Str;
    OS << " (" << format_بایتها(داده_2, std::nullopt, داده_2.اندازه_2(), 1, 0, درست)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, منطقي PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::Scoped_Printer_نوع::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  إذا (هذا->OuterScope)
    هذا->OuterScope->مجموعه_Printer(*هذا);
}
