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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/RISCVISAUtils.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ <cassert>

using ନାମାକାଶ llvm;

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
ଗଣନା RankFlags {
  RF_Z_EXTENSION = 1 << 6,
  RF_S_EXTENSION = 2 << 6,
  RF_X_EXTENSION = 3 << 6,
  RF_UNKNOWN_MULTILETTER_EXTENSION = 4 << 6,
};

// Get the rank for single-letter extension, lower value meaning higher
// priority.
ସ୍ଥିର unsigned singleLetterExtensionRank(ଅକ୍ଷର Ext) {
  ପ୍ରତିଜ୍ଞା(isLower(Ext));
  ବାଛ (Ext) {
  ସ୍ଥିତି 'i':
    ଫେରନ୍ତୁ 0;
  ସ୍ଥିତି 'e':
    ଫେରନ୍ତୁ 1;
  ସ୍ଥିତି 'y':
    ଫେରନ୍ତୁ 2;
  }

  size_t Pos = RISCVISAUtils::AllStdExts.find(Ext);
  ଯଦି (Pos != StringRef::npos)
    ଫେରନ୍ତୁ Pos + 3; // Skip 'e', 'i', and 'y' from above.

  // If we got an unknown extension letter, then give it an alphabetical
  // order, but after all known standard extensions.
  ଫେରନ୍ତୁ 3 + RISCVISAUtils::AllStdExts.size() + (Ext - 'a');
}

// Get the rank for multi-letter extension, lower value meaning higher
// priority/order in canonical order.
ସ୍ଥିର unsigned getExtensionRank(ଅଚଳ std::string &ExtName) {
  ପ୍ରତିଜ୍ଞା(ExtName.size() >= 1);
  ବାଛ (ExtName[0]) {
  ସ୍ଥିତି 's':
    ଫେରନ୍ତୁ RF_S_EXTENSION;
  ସ୍ଥିତି 'z':
    ପ୍ରତିଜ୍ଞା(ExtName.size() >= 2);
    // `z` extension must be sorted by canonical order of second letter.
    // e.g. zmx has higher rank than zax.
    ଫେରନ୍ତୁ RF_Z_EXTENSION | singleLetterExtensionRank(ExtName[1]);
  ସ୍ଥିତି 'x':
    ଫେରନ୍ତୁ RF_X_EXTENSION;
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ଯଦି (ExtName.size() == 1)
      ଫେରନ୍ତୁ singleLetterExtensionRank(ExtName[0]);
    ଫେରନ୍ତୁ RF_UNKNOWN_MULTILETTER_EXTENSION;
  }
}

// Compare function for extension.
// Only compare the extension name, ignore version comparison.
ତାର୍କିକ llvm::RISCVISAUtils::compareExtension(ଅଚଳ std::string &LHS,
                                           ଅଚଳ std::string &RHS) {
  unsigned LHSRank = getExtensionRank(LHS);
  unsigned RHSRank = getExtensionRank(RHS);

  // If the ranks differ, pick the lower rank.
  ଯଦି (LHSRank != RHSRank)
    ଫେରନ୍ତୁ LHSRank < RHSRank;

  // If the rank is same, it must be sorted by lexicographic order.
  ଫେରନ୍ତୁ LHS < RHS;
}
