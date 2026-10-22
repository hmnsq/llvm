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

#అన్తర్భావ "llvm/Support/RISCVISAUtils.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ <cassert>

ఉపయోగించి నామాకాశ llvm;

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
గణన RankFlags {
  RF_Z_EXTENSION = 1 << 6,
  RF_S_EXTENSION = 2 << 6,
  RF_X_EXTENSION = 3 << 6,
  RF_UNKNOWN_MULTILETTER_EXTENSION = 4 << 6,
};

// Get the rank for single-letter extension, lower value meaning higher
// priority.
స్థిర గుర్తులేని singleLetterExtensionRank(అక్షరం Ext) {
  ప్రతిజ్ఞా(is_క్రింది(Ext));
  ఎంపిక (Ext) {
  స్థితి 'i':
    తిరిగి 0;
  స్థితి 'e':
    తిరిగి 1;
  స్థితి 'y':
    తిరిగి 2;
  }

  size_t Pos = RISCVISAUtils::AllStdExts.కనుగొను(Ext);
  ఒకవేళ (Pos != StringRef::npos)
    తిరిగి Pos + 3; // Skip 'e', 'i', and 'y' from above.

  // If we got an unknown extension letter, then give it an alphabetical
  // order, but after all known standard extensions.
  తిరిగి 3 + RISCVISAUtils::AllStdExts.కొలత_2() + (Ext - 'a');
}

// Get the rank for multi-letter extension, lower value meaning higher
// priority/order in canonical order.
స్థిర గుర్తులేని getExtensionRank(స్థిరరాశి std::string &ExtName) {
  ప్రతిజ్ఞా(ExtName.కొలత_2() >= 1);
  ఎంపిక (ExtName[0]) {
  స్థితి 's':
    తిరిగి RF_S_EXTENSION;
  స్థితి 'z':
    ప్రతిజ్ఞా(ExtName.కొలత_2() >= 2);
    // `z` extension must be sorted by canonical order of second letter.
    // e.g. zmx has higher rank than zax.
    తిరిగి RF_Z_EXTENSION | singleLetterExtensionRank(ExtName[1]);
  స్థితి 'x':
    తిరిగి RF_X_EXTENSION;
  అప్రమేయ:
    ఒకవేళ (ExtName.కొలత_2() == 1)
      తిరిగి singleLetterExtensionRank(ExtName[0]);
    తిరిగి RF_UNKNOWN_MULTILETTER_EXTENSION;
  }
}

// Compare function for extension.
// Only compare the extension name, ignore version comparison.
తార్కిక llvm::RISCVISAUtils::compareExtension(స్థిరరాశి std::string &LHS,
                                           స్థిరరాశి std::string &RHS) {
  గుర్తులేని LHSRank = getExtensionRank(LHS);
  గుర్తులేని RHSRank = getExtensionRank(RHS);

  // If the ranks differ, pick the lower rank.
  ఒకవేళ (LHSRank != RHSRank)
    తిరిగి LHSRank < RHSRank;

  // If the rank is same, it must be sorted by lexicographic order.
  తిరిగి LHS < RHS;
}
