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

#അന്തര്ഭാവ "llvm/Support/RISCVISAUtils.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ <cassert>

using നാമാകാശ llvm;

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
എണ്ണൽ RankFlags {
  RF_Z_EXTENSION = 1 << 6,
  RF_S_EXTENSION = 2 << 6,
  RF_X_EXTENSION = 3 << 6,
  RF_UNKNOWN_MULTILETTER_EXTENSION = 4 << 6,
};

// Get the rank for single-letter extension, lower value meaning higher
// priority.
സ്ഥിതം unsigned singleLetterExtensionRank(അക്ഷരം Ext) {
  പ്രതിജ്ഞാ(isLower(Ext));
  തിരഞ്ഞെടുപ്പ് (Ext) {
  അവസ്ഥ 'i':
    തിരികെ 0;
  അവസ്ഥ 'e':
    തിരികെ 1;
  അവസ്ഥ 'y':
    തിരികെ 2;
  }

  size_t Pos = RISCVISAUtils::AllStdExts.find(Ext);
  എങ്കിൽ (Pos != StringRef::npos)
    തിരികെ Pos + 3; // Skip 'e', 'i', and 'y' from above.

  // If we got an unknown extension letter, then give it an alphabetical
  // order, but after all known standard extensions.
  തിരികെ 3 + RISCVISAUtils::AllStdExts.size() + (Ext - 'a');
}

// Get the rank for multi-letter extension, lower value meaning higher
// priority/order in canonical order.
സ്ഥിതം unsigned getExtensionRank(സ്ഥിരം std::string &ExtName) {
  പ്രതിജ്ഞാ(ExtName.size() >= 1);
  തിരഞ്ഞെടുപ്പ് (ExtName[0]) {
  അവസ്ഥ 's':
    തിരികെ RF_S_EXTENSION;
  അവസ്ഥ 'z':
    പ്രതിജ്ഞാ(ExtName.size() >= 2);
    // `z` extension must be sorted by canonical order of second letter.
    // e.g. zmx has higher rank than zax.
    തിരികെ RF_Z_EXTENSION | singleLetterExtensionRank(ExtName[1]);
  അവസ്ഥ 'x':
    തിരികെ RF_X_EXTENSION;
  സ്ഥിരസ്ഥിതി:
    എങ്കിൽ (ExtName.size() == 1)
      തിരികെ singleLetterExtensionRank(ExtName[0]);
    തിരികെ RF_UNKNOWN_MULTILETTER_EXTENSION;
  }
}

// Compare function for extension.
// Only compare the extension name, ignore version comparison.
തർക്കം llvm::RISCVISAUtils::compareExtension(സ്ഥിരം std::string &LHS,
                                           സ്ഥിരം std::string &RHS) {
  unsigned LHSRank = getExtensionRank(LHS);
  unsigned RHSRank = getExtensionRank(RHS);

  // If the ranks differ, pick the lower rank.
  എങ്കിൽ (LHSRank != RHSRank)
    തിരികെ LHSRank < RHSRank;

  // If the rank is same, it must be sorted by lexicographic order.
  തിരികെ LHS < RHS;
}
