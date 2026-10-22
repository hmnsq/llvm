//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ScopedPrinter.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Format.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

raw_ostream &llvm::ਸੰਚਾਲਕ<<(raw_ostream &OS, ਅਚਲ Hex_ਸੰਖਿਆ &ਮੁੱਲ) {
  OS << "0x" << utohexstr(ਮੁੱਲ.ਮੁੱਲ);
  ਵਾਪਸ OS;
}

ਖਾਲੀ ScopedPrinter::print_ਦੁਆਧਾਰੀ_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> ਅੰਕੜੇ_2, ਤਾਰਕਿਕ ਖੰਡ_2,
                                    uint32_t ਸ਼ੁਰੂ_ਵਿੱਥ) {
  ਜੇ (ਅੰਕੜੇ_2.ਮਾਪ_2() > 16)
    ਖੰਡ_2 = ਸੱਚ;

  ਜੇ (ਖੰਡ_2) {
    ਸ਼ੁਰੂ_ਸਤਰ() << Label;
    ਜੇ (!Str.ਖਾਲੀ_ਥਾਵਾਂ())
      OS << ": " << Str;
    OS << " (\n";
    ਜੇ (!ਅੰਕੜੇ_2.ਖਾਲੀ_ਥਾਵਾਂ())
      OS << format_ਬਾਈਟਾਂ_with_ascii(ਅੰਕੜੇ_2, ਸ਼ੁਰੂ_ਵਿੱਥ, 16, 4,
                                    (IndentLevel + 1) * 2, ਸੱਚ)
         << "\n";
    ਸ਼ੁਰੂ_ਸਤਰ() << ")\n";
  } ਨਹੀਂਤਾਂ {
    ਸ਼ੁਰੂ_ਸਤਰ() << Label << ":";
    ਜੇ (!Str.ਖਾਲੀ_ਥਾਵਾਂ())
      OS << " " << Str;
    OS << " (" << format_ਬਾਈਟਾਂ(ਅੰਕੜੇ_2, std::nullopt, ਅੰਕੜੇ_2.ਮਾਪ_2(), 1, 0, ਸੱਚ)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, ਤਾਰਕਿਕ PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::Scoped_Printer_ਕਿਸਮ::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  ਜੇ (ਇਹ->OuterScope)
    ਇਹ->OuterScope->ਸੈੱਟ_Printer(*ਇਹ);
}
