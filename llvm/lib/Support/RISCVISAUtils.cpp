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

#ಅನ್ತರ್ಭಾವ "llvm/Support/RISCVISAUtils.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ <cassert>

using ನಾಮಾಕಾಶ llvm;

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
ಗಣನಾ RankFlags {
  RF_Z_EXTENSION = 1 << 6,
  RF_S_EXTENSION = 2 << 6,
  RF_X_EXTENSION = 3 << 6,
  RF_UNKNOWN_MULTILETTER_EXTENSION = 4 << 6,
};

// Get the rank for single-letter extension, lower value meaning higher
// priority.
ಸ್ಥಿರ unsigned singleLetterExtensionRank(ವರ್ಣ Ext) {
  ಪ್ರತಿಜ್ಞಾ(isLower(Ext));
  ವಿಕಲ್ಪ (Ext) {
  ಪಕ್ಷ 'i':
    ಪ್ರತ್ಯಾಗಮ 0;
  ಪಕ್ಷ 'e':
    ಪ್ರತ್ಯಾಗಮ 1;
  ಪಕ್ಷ 'y':
    ಪ್ರತ್ಯಾಗಮ 2;
  }

  size_t Pos = RISCVISAUtils::AllStdExts.find(Ext);
  ಯದಿ (Pos != StringRef::npos)
    ಪ್ರತ್ಯಾಗಮ Pos + 3; // Skip 'e', 'i', and 'y' from above.

  // If we got an unknown extension letter, then give it an alphabetical
  // order, but after all known standard extensions.
  ಪ್ರತ್ಯಾಗಮ 3 + RISCVISAUtils::AllStdExts.size() + (Ext - 'a');
}

// Get the rank for multi-letter extension, lower value meaning higher
// priority/order in canonical order.
ಸ್ಥಿರ unsigned getExtensionRank(ಧ್ರುವ std::string &ExtName) {
  ಪ್ರತಿಜ್ಞಾ(ExtName.size() >= 1);
  ವಿಕಲ್ಪ (ExtName[0]) {
  ಪಕ್ಷ 's':
    ಪ್ರತ್ಯಾಗಮ RF_S_EXTENSION;
  ಪಕ್ಷ 'z':
    ಪ್ರತಿಜ್ಞಾ(ExtName.size() >= 2);
    // `z` extension must be sorted by canonical order of second letter.
    // e.g. zmx has higher rank than zax.
    ಪ್ರತ್ಯಾಗಮ RF_Z_EXTENSION | singleLetterExtensionRank(ExtName[1]);
  ಪಕ್ಷ 'x':
    ಪ್ರತ್ಯಾಗಮ RF_X_EXTENSION;
  ಸ್ವಭಾವ:
    ಯದಿ (ExtName.size() == 1)
      ಪ್ರತ್ಯಾಗಮ singleLetterExtensionRank(ExtName[0]);
    ಪ್ರತ್ಯಾಗಮ RF_UNKNOWN_MULTILETTER_EXTENSION;
  }
}

// Compare function for extension.
// Only compare the extension name, ignore version comparison.
ತಾರ್ಕಿಕ llvm::RISCVISAUtils::compareExtension(ಧ್ರುವ std::string &LHS,
                                           ಧ್ರುವ std::string &RHS) {
  unsigned LHSRank = getExtensionRank(LHS);
  unsigned RHSRank = getExtensionRank(RHS);

  // If the ranks differ, pick the lower rank.
  ಯದಿ (LHSRank != RHSRank)
    ಪ್ರತ್ಯಾಗಮ LHSRank < RHSRank;

  // If the rank is same, it must be sorted by lexicographic order.
  ಪ್ರತ್ಯಾಗಮ LHS < RHS;
}
