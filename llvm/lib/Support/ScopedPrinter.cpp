//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/ScopedPrinter.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Format.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

raw_ostream &llvm::ಕ್ರಿಯಾಕಾರಕ<<(raw_ostream &OS, ಸ್ಥಿರಾಂಕ Hex_ಸಂಖ್ಯೆ &ಮೌಲ್ಯ) {
  OS << "0x" << utohexstr(ಮೌಲ್ಯ.ಮೌಲ್ಯ);
  ಹಿಂತಿರುಗಿ OS;
}

ಶೂನ್ಯ ScopedPrinter::print_ದ್ವಿಮಾನ_Impl(StringRef Label, StringRef Str,
                                    ArrayRef<uint8_t> ದತ್ತಾಂಶ_2, ತಾರ್ಕಿಕ ಖಂಡ_2,
                                    uint32_t ಆರಂಭ_ಸ್ಥಾನಾಂತರ) {
  ಆದರೆ (ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3() > 16)
    ಖಂಡ_2 = ನಿಜ;

  ಆದರೆ (ಖಂಡ_2) {
    ಆರಂಭ_ಸಾಲು() << Label;
    ಆದರೆ (!Str.ಖಾಲಿಗಳು())
      OS << ": " << Str;
    OS << " (\n";
    ಆದರೆ (!ದತ್ತಾಂಶ_2.ಖಾಲಿಗಳು())
      OS << format_ಬೈಟುಗಳು_with_ascii(ದತ್ತಾಂಶ_2, ಆರಂಭ_ಸ್ಥಾನಾಂತರ, 16, 4,
                                    (IndentLevel + 1) * 2, ನಿಜ)
         << "\n";
    ಆರಂಭ_ಸಾಲು() << ")\n";
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಆರಂಭ_ಸಾಲು() << Label << ":";
    ಆದರೆ (!Str.ಖಾಲಿಗಳು())
      OS << " " << Str;
    OS << " (" << format_ಬೈಟುಗಳು(ದತ್ತಾಂಶ_2, std::nullopt, ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3(), 1, 0, ನಿಜ)
       << ")\n";
  }
}

JSONScopedPrinter::JSONScopedPrinter(
    raw_ostream &OS, ತಾರ್ಕಿಕ PrettyPrint,
    std::unique_ptr<DelimitedScope> &&OuterScope)
    : ScopedPrinter(OS, ScopedPrinter::Scoped_Printer_ಬಗೆ::JSON),
      JOS(OS, /*Indent=*/PrettyPrint ? 2 : 0),
      OuterScope(std::move(OuterScope)) {
  ಆದರೆ (ಇದು->OuterScope)
    ಇದು->OuterScope->ಗಣ_Printer(*ಇದು);
}
