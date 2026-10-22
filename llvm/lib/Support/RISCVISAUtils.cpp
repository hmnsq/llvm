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

#समावेश "llvm/Support/RISCVISAUtils.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश <cassert>

using namespace llvm;

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
क्रमागत RankFlags {
  RF_Z_EXTENSION = 1 << 6,
  RF_S_EXTENSION = 2 << 6,
  RF_X_EXTENSION = 3 << 6,
  RF_UNKNOWN_MULTILETTER_EXTENSION = 4 << 6,
};

// Get the rank for single-letter extension, lower value meaning higher
// priority.
अटल अचिन्हित singleLetterExtensionRank(अक्षर Ext) {
  निश्चित(isLower(Ext));
  चयन (Ext) {
  हाल 'i':
    वापस 0;
  हाल 'e':
    वापस 1;
  हाल 'y':
    वापस 2;
  }

  माप_प्रकार Pos = RISCVISAUtils::AllStdExts.find(Ext);
  अगर (Pos != StringRef::npos)
    वापस Pos + 3; // Skip 'e', 'i', and 'y' from above.

  // If we got an unknown extension letter, then give it an alphabetical
  // order, but after all known standard extensions.
  वापस 3 + RISCVISAUtils::AllStdExts.आकार_2() + (Ext - 'a');
}

// Get the rank for multi-letter extension, lower value meaning higher
// priority/order in canonical order.
अटल अचिन्हित getExtensionRank(स्थिर std::string &ExtName) {
  निश्चित(ExtName.आकार_2() >= 1);
  चयन (ExtName[0]) {
  हाल 's':
    वापस RF_S_EXTENSION;
  हाल 'z':
    निश्चित(ExtName.आकार_2() >= 2);
    // `z` extension must be sorted by canonical order of second letter.
    // e.g. zmx has higher rank than zax.
    वापस RF_Z_EXTENSION | singleLetterExtensionRank(ExtName[1]);
  हाल 'x':
    वापस RF_X_EXTENSION;
  शेष:
    अगर (ExtName.आकार_2() == 1)
      वापस singleLetterExtensionRank(ExtName[0]);
    वापस RF_UNKNOWN_MULTILETTER_EXTENSION;
  }
}

// Compare function for extension.
// Only compare the extension name, ignore version comparison.
bool llvm::RISCVISAUtils::compareExtension(स्थिर std::string &LHS,
                                           स्थिर std::string &RHS) {
  अचिन्हित LHSRank = getExtensionRank(LHS);
  अचिन्हित RHSRank = getExtensionRank(RHS);

  // If the ranks differ, pick the lower rank.
  अगर (LHSRank != RHSRank)
    वापस LHSRank < RHSRank;

  // If the rank is same, it must be sorted by lexicographic order.
  वापस LHS < RHS;
}
