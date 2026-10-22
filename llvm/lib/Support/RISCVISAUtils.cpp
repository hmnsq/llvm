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
स्थिर अचिन्हित singleLetterExtensionRank(वर्ण Ext) {
  प्रतिज्ञा(isLower(Ext));
  विकल्प (Ext) {
  पक्ष 'i':
    फर्काऊ 0;
  पक्ष 'e':
    फर्काऊ 1;
  पक्ष 'y':
    फर्काऊ 2;
  }

  माप_प्रकार Pos = RISCVISAUtils::AllStdExts.find(Ext);
  यदि (Pos != StringRef::npos)
    फर्काऊ Pos + 3; // Skip 'e', 'i', and 'y' from above.

  // If we got an unknown extension letter, then give it an alphabetical
  // order, but after all known standard extensions.
  फर्काऊ 3 + RISCVISAUtils::AllStdExts.size() + (Ext - 'a');
}

// Get the rank for multi-letter extension, lower value meaning higher
// priority/order in canonical order.
स्थिर अचिन्हित getExtensionRank(अचर std::string &ExtName) {
  प्रतिज्ञा(ExtName.size() >= 1);
  विकल्प (ExtName[0]) {
  पक्ष 's':
    फर्काऊ RF_S_EXTENSION;
  पक्ष 'z':
    प्रतिज्ञा(ExtName.size() >= 2);
    // `z` extension must be sorted by canonical order of second letter.
    // e.g. zmx has higher rank than zax.
    फर्काऊ RF_Z_EXTENSION | singleLetterExtensionRank(ExtName[1]);
  पक्ष 'x':
    फर्काऊ RF_X_EXTENSION;
  स्वभाव:
    यदि (ExtName.size() == 1)
      फर्काऊ singleLetterExtensionRank(ExtName[0]);
    फर्काऊ RF_UNKNOWN_MULTILETTER_EXTENSION;
  }
}

// Compare function for extension.
// Only compare the extension name, ignore version comparison.
तार्किक llvm::RISCVISAUtils::compareExtension(अचर std::string &LHS,
                                           अचर std::string &RHS) {
  अचिन्हित LHSRank = getExtensionRank(LHS);
  अचिन्हित RHSRank = getExtensionRank(RHS);

  // If the ranks differ, pick the lower rank.
  यदि (LHSRank != RHSRank)
    फर्काऊ LHSRank < RHSRank;

  // If the rank is same, it must be sorted by lexicographic order.
  फर्काऊ LHS < RHS;
}
