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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/RISCVISAUtils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

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
ਗਿਣਤੀ RankFlags {
  RF_Z_EXTENSION = 1 << 6,
  RF_S_EXTENSION = 2 << 6,
  RF_X_EXTENSION = 3 << 6,
  RF_UNKNOWN_MULTILETTER_EXTENSION = 4 << 6,
};

// Get the rank for single-letter extension, lower value meaning higher
// priority.
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ singleLetterExtensionRank(ਅੱਖਰ Ext) {
  ਪ੍ਰਤਿਜ੍ਞਾ(is_ਹੇਠਲਾ(Ext));
  ਚੋਣ (Ext) {
  ਹਾਲਤ 'i':
    ਵਾਪਸ 0;
  ਹਾਲਤ 'e':
    ਵਾਪਸ 1;
  ਹਾਲਤ 'y':
    ਵਾਪਸ 2;
  }

  size_t Pos = RISCVISAUtils::AllStdExts.ਲੱਭੋ(Ext);
  ਜੇ (Pos != StringRef::npos)
    ਵਾਪਸ Pos + 3; // Skip 'e', 'i', and 'y' from above.

  // If we got an unknown extension letter, then give it an alphabetical
  // order, but after all known standard extensions.
  ਵਾਪਸ 3 + RISCVISAUtils::AllStdExts.ਮਾਪ_2() + (Ext - 'a');
}

// Get the rank for multi-letter extension, lower value meaning higher
// priority/order in canonical order.
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ getExtensionRank(ਅਚਲ std::string &ExtName) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ExtName.ਮਾਪ_2() >= 1);
  ਚੋਣ (ExtName[0]) {
  ਹਾਲਤ 's':
    ਵਾਪਸ RF_S_EXTENSION;
  ਹਾਲਤ 'z':
    ਪ੍ਰਤਿਜ੍ਞਾ(ExtName.ਮਾਪ_2() >= 2);
    // `z` extension must be sorted by canonical order of second letter.
    // e.g. zmx has higher rank than zax.
    ਵਾਪਸ RF_Z_EXTENSION | singleLetterExtensionRank(ExtName[1]);
  ਹਾਲਤ 'x':
    ਵਾਪਸ RF_X_EXTENSION;
  ਮੂਲ:
    ਜੇ (ExtName.ਮਾਪ_2() == 1)
      ਵਾਪਸ singleLetterExtensionRank(ExtName[0]);
    ਵਾਪਸ RF_UNKNOWN_MULTILETTER_EXTENSION;
  }
}

// Compare function for extension.
// Only compare the extension name, ignore version comparison.
ਤਾਰਕਿਕ llvm::RISCVISAUtils::compareExtension(ਅਚਲ std::string &LHS,
                                           ਅਚਲ std::string &RHS) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ LHSRank = getExtensionRank(LHS);
  ਬਿਨਾਨਿਸ਼ਾਨੀ RHSRank = getExtensionRank(RHS);

  // If the ranks differ, pick the lower rank.
  ਜੇ (LHSRank != RHSRank)
    ਵਾਪਸ LHSRank < RHSRank;

  // If the rank is same, it must be sorted by lexicographic order.
  ਵਾਪਸ LHS < RHS;
}
