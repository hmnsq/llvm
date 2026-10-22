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

#अन्तर्भाव "llvm/Support/RISCVISAUtils.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव <cassert>

using नामाकाश llvm;

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
गणना RankFlags {
  RF_Z_EXTENSION = 1 << 6,
  RF_S_EXTENSION = 2 << 6,
  RF_X_EXTENSION = 3 << 6,
  RF_UNKNOWN_MULTILETTER_EXTENSION = 4 << 6,
};

// Get the rank for single-letter extension, lower value meaning higher
// priority.
स्थिर unsigned singleLetterExtensionRank(वर्ण Ext) {
  प्रतिज्ञा(isLower(Ext));
  विकल्प (Ext) {
  पक्ष 'i':
    प्रत्यागम 0;
  पक्ष 'e':
    प्रत्यागम 1;
  पक्ष 'y':
    प्रत्यागम 2;
  }

  size_t Pos = RISCVISAUtils::AllStdExts.find(Ext);
  यदि (Pos != StringRef::npos)
    प्रत्यागम Pos + 3; // Skip 'e', 'i', and 'y' from above.

  // If we got an unknown extension letter, then give it an alphabetical
  // order, but after all known standard extensions.
  प्रत्यागम 3 + RISCVISAUtils::AllStdExts.size() + (Ext - 'a');
}

// Get the rank for multi-letter extension, lower value meaning higher
// priority/order in canonical order.
स्थिर unsigned getExtensionRank(ध्रुव std::string &ExtName) {
  प्रतिज्ञा(ExtName.size() >= 1);
  विकल्प (ExtName[0]) {
  पक्ष 's':
    प्रत्यागम RF_S_EXTENSION;
  पक्ष 'z':
    प्रतिज्ञा(ExtName.size() >= 2);
    // `z` extension must be sorted by canonical order of second letter.
    // e.g. zmx has higher rank than zax.
    प्रत्यागम RF_Z_EXTENSION | singleLetterExtensionRank(ExtName[1]);
  पक्ष 'x':
    प्रत्यागम RF_X_EXTENSION;
  स्वभाव:
    यदि (ExtName.size() == 1)
      प्रत्यागम singleLetterExtensionRank(ExtName[0]);
    प्रत्यागम RF_UNKNOWN_MULTILETTER_EXTENSION;
  }
}

// Compare function for extension.
// Only compare the extension name, ignore version comparison.
तार्किक llvm::RISCVISAUtils::compareExtension(ध्रुव std::string &LHS,
                                           ध्रुव std::string &RHS) {
  unsigned LHSRank = getExtensionRank(LHS);
  unsigned RHSRank = getExtensionRank(RHS);

  // If the ranks differ, pick the lower rank.
  यदि (LHSRank != RHSRank)
    प्रत्यागम LHSRank < RHSRank;

  // If the rank is same, it must be sorted by lexicographic order.
  प्रत्यागम LHS < RHS;
}
