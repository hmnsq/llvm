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

#شامل "llvm/Support/RISCVISAUtils.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل <cassert>

استعمال نامخلا llvm;

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
شمار RankFlags {
  RF_Z_EXTENSION = 1 << 6,
  RF_S_EXTENSION = 2 << 6,
  RF_X_EXTENSION = 3 << 6,
  RF_UNKNOWN_MULTILETTER_EXTENSION = 4 << 6,
};

// Get the rank for single-letter extension, lower value meaning higher
// priority.
ساکن غیرعلامتی singleLetterExtensionRank(حرف Ext) {
  أكد(is_زیریں(Ext));
  انتخاب (Ext) {
  حالت 'i':
    واپس 0;
  حالت 'e':
    واپس 1;
  حالت 'y':
    واپس 2;
  }

  size_t Pos = RISCVISAUtils::AllStdExts.ڈھونڈو(Ext);
  اگر (Pos != StringRef::npos)
    واپس Pos + 3; // Skip 'e', 'i', and 'y' from above.

  // If we got an unknown extension letter, then give it an alphabetical
  // order, but after all known standard extensions.
  واپس 3 + RISCVISAUtils::AllStdExts.سائز_2() + (Ext - 'a');
}

// Get the rank for multi-letter extension, lower value meaning higher
// priority/order in canonical order.
ساکن غیرعلامتی getExtensionRank(ثابت std::string &ExtName) {
  أكد(ExtName.سائز_2() >= 1);
  انتخاب (ExtName[0]) {
  حالت 's':
    واپس RF_S_EXTENSION;
  حالت 'z':
    أكد(ExtName.سائز_2() >= 2);
    // `z` extension must be sorted by canonical order of second letter.
    // e.g. zmx has higher rank than zax.
    واپس RF_Z_EXTENSION | singleLetterExtensionRank(ExtName[1]);
  حالت 'x':
    واپس RF_X_EXTENSION;
  طےشدہ:
    اگر (ExtName.سائز_2() == 1)
      واپس singleLetterExtensionRank(ExtName[0]);
    واپس RF_UNKNOWN_MULTILETTER_EXTENSION;
  }
}

// Compare function for extension.
// Only compare the extension name, ignore version comparison.
منطقی llvm::RISCVISAUtils::compareExtension(ثابت std::string &LHS,
                                           ثابت std::string &RHS) {
  غیرعلامتی LHSRank = getExtensionRank(LHS);
  غیرعلامتی RHSRank = getExtensionRank(RHS);

  // If the ranks differ, pick the lower rank.
  اگر (LHSRank != RHSRank)
    واپس LHSRank < RHSRank;

  // If the rank is same, it must be sorted by lexicographic order.
  واپس LHS < RHS;
}
