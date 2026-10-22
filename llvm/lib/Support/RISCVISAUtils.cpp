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

#અન્તર્ભાવ "llvm/Support/RISCVISAUtils.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ <cassert>

using નામાકાશ llvm;

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
ગણના RankFlags {
  RF_Z_EXTENSION = 1 << 6,
  RF_S_EXTENSION = 2 << 6,
  RF_X_EXTENSION = 3 << 6,
  RF_UNKNOWN_MULTILETTER_EXTENSION = 4 << 6,
};

// Get the rank for single-letter extension, lower value meaning higher
// priority.
સ્થિર unsigned singleLetterExtensionRank(અક્ષર Ext) {
  પ્રતિજ્ઞા(isLower(Ext));
  પસંદગી (Ext) {
  સ્થિતિ 'i':
    પાછા 0;
  સ્થિતિ 'e':
    પાછા 1;
  સ્થિતિ 'y':
    પાછા 2;
  }

  size_t Pos = RISCVISAUtils::AllStdExts.find(Ext);
  જો (Pos != StringRef::npos)
    પાછા Pos + 3; // Skip 'e', 'i', and 'y' from above.

  // If we got an unknown extension letter, then give it an alphabetical
  // order, but after all known standard extensions.
  પાછા 3 + RISCVISAUtils::AllStdExts.size() + (Ext - 'a');
}

// Get the rank for multi-letter extension, lower value meaning higher
// priority/order in canonical order.
સ્થિર unsigned getExtensionRank(અચલ std::string &ExtName) {
  પ્રતિજ્ઞા(ExtName.size() >= 1);
  પસંદગી (ExtName[0]) {
  સ્થિતિ 's':
    પાછા RF_S_EXTENSION;
  સ્થિતિ 'z':
    પ્રતિજ્ઞા(ExtName.size() >= 2);
    // `z` extension must be sorted by canonical order of second letter.
    // e.g. zmx has higher rank than zax.
    પાછા RF_Z_EXTENSION | singleLetterExtensionRank(ExtName[1]);
  સ્થિતિ 'x':
    પાછા RF_X_EXTENSION;
  મૂળભૂત:
    જો (ExtName.size() == 1)
      પાછા singleLetterExtensionRank(ExtName[0]);
    પાછા RF_UNKNOWN_MULTILETTER_EXTENSION;
  }
}

// Compare function for extension.
// Only compare the extension name, ignore version comparison.
તાર્કિક llvm::RISCVISAUtils::compareExtension(અચલ std::string &LHS,
                                           અચલ std::string &RHS) {
  unsigned LHSRank = getExtensionRank(LHS);
  unsigned RHSRank = getExtensionRank(RHS);

  // If the ranks differ, pick the lower rank.
  જો (LHSRank != RHSRank)
    પાછા LHSRank < RHSRank;

  // If the rank is same, it must be sorted by lexicographic order.
  પાછા LHS < RHS;
}
