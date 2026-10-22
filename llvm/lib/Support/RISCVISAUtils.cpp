//===-- RISCVISAUtils.cpp - RISC-V ISA Utilities --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Utilities shared by TableGen and RISCVISAInfo.
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/RISCVISAUtils.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن <cassert>

ورتوں نطاق llvm;

// We rank extensions in the following order:
// -Single letter extensions in canonical order.
// -Unknown single letter extensions in alphabetical order.
// -Multi-letter extensions starting with 'z' sorted by canonical order of
//  the second letter then sorted alphabetically.
// -Multi-letter extensions starting with 's' in alphabetical order.
// -(TODO) Multi-letter extensions starting with 'zxm' in alphabetical order.
// -X extensions in alphabetical order.
// -Unknown multi-letter extensions in alphabetical order.
// These flags are used to indicate the category. The first 6 bits store the
// single letter extension rank for single letter and multi-letter extensions
// starting with 'z'.
تعداد RankFlags {
  RF_Z_EXTENSION = 1 << 6,
  RF_S_EXTENSION = 2 << 6,
  RF_X_EXTENSION = 3 << 6,
  RF_UNKNOWN_MULTILETTER_EXTENSION = 4 << 6,
};

// Get the rank for single-letter extension, lower value meaning higher
// priority.
ساكن بنانشانی singleLetterExtensionRank(حرف Ext) {
  أكد(is_ہیٹھلا(Ext));
  بدل (Ext) {
  حالة 'i':
    أرجع 0;
  حالة 'e':
    أرجع 1;
  حالة 'y':
    أرجع 2;
  }

  size_t Pos = RISCVISAUtils::AllStdExts.لبھو(Ext);
  إذا (Pos != StringRef::npos)
    أرجع Pos + 3; // Skip 'e', 'i', and 'y' from above.

  // If we got an unknown extension letter, then give it an alphabetical
  // order, but after all known standard extensions.
  أرجع 3 + RISCVISAUtils::AllStdExts.ناپ_2() + (Ext - 'a');
}

// Get the rank for multi-letter extension, lower value meaning higher
// priority/order in canonical order.
ساكن بنانشانی getExtensionRank(ثابت std::string &ExtName) {
  أكد(ExtName.ناپ_2() >= 1);
  بدل (ExtName[0]) {
  حالة 's':
    أرجع RF_S_EXTENSION;
  حالة 'z':
    أكد(ExtName.ناپ_2() >= 2);
    // `z` extension must be sorted by canonical order of second letter.
    // e.g. zmx has higher rank than zax.
    أرجع RF_Z_EXTENSION | singleLetterExtensionRank(ExtName[1]);
  حالة 'x':
    أرجع RF_X_EXTENSION;
  افتراضي:
    إذا (ExtName.ناپ_2() == 1)
      أرجع singleLetterExtensionRank(ExtName[0]);
    أرجع RF_UNKNOWN_MULTILETTER_EXTENSION;
  }
}

// Compare function for extension.
// Only compare the extension name, ignore version comparison.
منطقي llvm::RISCVISAUtils::compareExtension(ثابت std::string &LHS,
                                           ثابت std::string &RHS) {
  بنانشانی LHSRank = getExtensionRank(LHS);
  بنانشانی RHSRank = getExtensionRank(RHS);

  // If the ranks differ, pick the lower rank.
  إذا (LHSRank != RHSRank)
    أرجع LHSRank < RHSRank;

  // If the rank is same, it must be sorted by lexicographic order.
  أرجع LHS < RHS;
}
