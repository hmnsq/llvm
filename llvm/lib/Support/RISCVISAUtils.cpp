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
स्थिर अचिन्हित singleLetterExtensionRank(अक्षर Ext) {
  प्रतिज्ञा(isLower(Ext));
  निवड (Ext) {
  स्थिती 'i':
    परत 0;
  स्थिती 'e':
    परत 1;
  स्थिती 'y':
    परत 2;
  }

  माप_प्रकार Pos = RISCVISAUtils::AllStdExts.find(Ext);
  जर (Pos != StringRef::npos)
    परत Pos + 3; // Skip 'e', 'i', and 'y' from above.

  // If we got an unknown extension letter, then give it an alphabetical
  // order, but after all known standard extensions.
  परत 3 + RISCVISAUtils::AllStdExts.size() + (Ext - 'a');
}

// Get the rank for multi-letter extension, lower value meaning higher
// priority/order in canonical order.
स्थिर अचिन्हित getExtensionRank(स्थिरांक std::string &ExtName) {
  प्रतिज्ञा(ExtName.size() >= 1);
  निवड (ExtName[0]) {
  स्थिती 's':
    परत RF_S_EXTENSION;
  स्थिती 'z':
    प्रतिज्ञा(ExtName.size() >= 2);
    // `z` extension must be sorted by canonical order of second letter.
    // e.g. zmx has higher rank than zax.
    परत RF_Z_EXTENSION | singleLetterExtensionRank(ExtName[1]);
  स्थिती 'x':
    परत RF_X_EXTENSION;
  पूर्वनिर्धारित:
    जर (ExtName.size() == 1)
      परत singleLetterExtensionRank(ExtName[0]);
    परत RF_UNKNOWN_MULTILETTER_EXTENSION;
  }
}

// Compare function for extension.
// Only compare the extension name, ignore version comparison.
तार्किक llvm::RISCVISAUtils::compareExtension(स्थिरांक std::string &LHS,
                                           स्थिरांक std::string &RHS) {
  अचिन्हित LHSRank = getExtensionRank(LHS);
  अचिन्हित RHSRank = getExtensionRank(RHS);

  // If the ranks differ, pick the lower rank.
  जर (LHSRank != RHSRank)
    परत LHSRank < RHSRank;

  // If the rank is same, it must be sorted by lexicographic order.
  परत LHS < RHS;
}
