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

#অন্তর্ভাব "llvm/Support/RISCVISAUtils.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব <cassert>

using নামাকাশ llvm;

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
গণনা RankFlags {
  RF_Z_EXTENSION = 1 << 6,
  RF_S_EXTENSION = 2 << 6,
  RF_X_EXTENSION = 3 << 6,
  RF_UNKNOWN_MULTILETTER_EXTENSION = 4 << 6,
};

// Get the rank for single-letter extension, lower value meaning higher
// priority.
স্থির unsigned singleLetterExtensionRank(বর্ণ Ext) {
  প্রতিজ্ঞা(isLower(Ext));
  বিকল্প (Ext) {
  পক্ষ 'i':
    প্রত্যাগম 0;
  পক্ষ 'e':
    প্রত্যাগম 1;
  পক্ষ 'y':
    প্রত্যাগম 2;
  }

  size_t Pos = RISCVISAUtils::AllStdExts.find(Ext);
  যদি (Pos != StringRef::npos)
    প্রত্যাগম Pos + 3; // Skip 'e', 'i', and 'y' from above.

  // If we got an unknown extension letter, then give it an alphabetical
  // order, but after all known standard extensions.
  প্রত্যাগম 3 + RISCVISAUtils::AllStdExts.size() + (Ext - 'a');
}

// Get the rank for multi-letter extension, lower value meaning higher
// priority/order in canonical order.
স্থির unsigned getExtensionRank(ধ্রুব std::string &ExtName) {
  প্রতিজ্ঞা(ExtName.size() >= 1);
  বিকল্প (ExtName[0]) {
  পক্ষ 's':
    প্রত্যাগম RF_S_EXTENSION;
  পক্ষ 'z':
    প্রতিজ্ঞা(ExtName.size() >= 2);
    // `z` extension must be sorted by canonical order of second letter.
    // e.g. zmx has higher rank than zax.
    প্রত্যাগম RF_Z_EXTENSION | singleLetterExtensionRank(ExtName[1]);
  পক্ষ 'x':
    প্রত্যাগম RF_X_EXTENSION;
  স্বভাব:
    যদি (ExtName.size() == 1)
      প্রত্যাগম singleLetterExtensionRank(ExtName[0]);
    প্রত্যাগম RF_UNKNOWN_MULTILETTER_EXTENSION;
  }
}

// Compare function for extension.
// Only compare the extension name, ignore version comparison.
তার্কিক llvm::RISCVISAUtils::compareExtension(ধ্রুব std::string &LHS,
                                           ধ্রুব std::string &RHS) {
  unsigned LHSRank = getExtensionRank(LHS);
  unsigned RHSRank = getExtensionRank(RHS);

  // If the ranks differ, pick the lower rank.
  যদি (LHSRank != RHSRank)
    প্রত্যাগম LHSRank < RHSRank;

  // If the rank is same, it must be sorted by lexicographic order.
  প্রত্যাগম LHS < RHS;
}
