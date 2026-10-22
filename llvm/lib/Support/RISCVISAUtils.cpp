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

#அந்தர்பாவ "llvm/Support/RISCVISAUtils.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ <cassert>

using நாமாகாஶ llvm;

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
எண்ணிக்கை RankFlags {
  RF_Z_EXTENSION = 1 << 6,
  RF_S_EXTENSION = 2 << 6,
  RF_X_EXTENSION = 3 << 6,
  RF_UNKNOWN_MULTILETTER_EXTENSION = 4 << 6,
};

// Get the rank for single-letter extension, lower value meaning higher
// priority.
நிலையான unsigned singleLetterExtensionRank(எழுத்து Ext) {
  ப்ரதிஜ்ஞா(isLower(Ext));
  தேர்வு (Ext) {
  நிலை 'i':
    திரும்பு 0;
  நிலை 'e':
    திரும்பு 1;
  நிலை 'y':
    திரும்பு 2;
  }

  size_t Pos = RISCVISAUtils::AllStdExts.find(Ext);
  எனில் (Pos != StringRef::npos)
    திரும்பு Pos + 3; // Skip 'e', 'i', and 'y' from above.

  // If we got an unknown extension letter, then give it an alphabetical
  // order, but after all known standard extensions.
  திரும்பு 3 + RISCVISAUtils::AllStdExts.size() + (Ext - 'a');
}

// Get the rank for multi-letter extension, lower value meaning higher
// priority/order in canonical order.
நிலையான unsigned getExtensionRank(மாறிலி std::string &ExtName) {
  ப்ரதிஜ்ஞா(ExtName.size() >= 1);
  தேர்வு (ExtName[0]) {
  நிலை 's':
    திரும்பு RF_S_EXTENSION;
  நிலை 'z':
    ப்ரதிஜ்ஞா(ExtName.size() >= 2);
    // `z` extension must be sorted by canonical order of second letter.
    // e.g. zmx has higher rank than zax.
    திரும்பு RF_Z_EXTENSION | singleLetterExtensionRank(ExtName[1]);
  நிலை 'x':
    திரும்பு RF_X_EXTENSION;
  இயல்பு:
    எனில் (ExtName.size() == 1)
      திரும்பு singleLetterExtensionRank(ExtName[0]);
    திரும்பு RF_UNKNOWN_MULTILETTER_EXTENSION;
  }
}

// Compare function for extension.
// Only compare the extension name, ignore version comparison.
தருக்க llvm::RISCVISAUtils::compareExtension(மாறிலி std::string &LHS,
                                           மாறிலி std::string &RHS) {
  unsigned LHSRank = getExtensionRank(LHS);
  unsigned RHSRank = getExtensionRank(RHS);

  // If the ranks differ, pick the lower rank.
  எனில் (LHSRank != RHSRank)
    திரும்பு LHSRank < RHSRank;

  // If the rank is same, it must be sorted by lexicographic order.
  திரும்பு LHS < RHS;
}
