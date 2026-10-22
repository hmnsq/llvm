//===- llvm/ADT/SuffixTreeNode.cpp - Nodes for SuffixTrees --------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines nodes for use within a SuffixTree.
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/SuffixTreeNode.h"

using नामाकाश llvm;

unsigned Suffix_वृक्ष_गाँठ::getStartIdx() ध्रुव { प्रत्यागम StartIdx; }
शून्य Suffix_वृक्ष_गाँठ::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
शून्य Suffix_वृक्ष_गाँठ::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned Suffix_वृक्ष_गाँठ::getConcatLen() ध्रुव { प्रत्यागम ConcatLen; }

तार्किक Suffix_वृक्ष_Internal_गाँठ::is_मूल() ध्रुव {
  प्रत्यागम getStartIdx() == EmptyIdx;
}
unsigned Suffix_वृक्ष_Internal_गाँठ::getEndIdx() ध्रुव { प्रत्यागम EndIdx; }
शून्य Suffix_वृक्ष_Internal_गाँठ::setLink(Suffix_वृक्ष_Internal_गाँठ *L) {
  प्रतिज्ञा(L && "Cannot set a null link?");
  Link = L;
}
Suffix_वृक्ष_Internal_गाँठ *Suffix_वृक्ष_Internal_गाँठ::getLink() ध्रुव { प्रत्यागम Link; }

unsigned Suffix_वृक्ष_Leaf_गाँठ::getEndIdx() ध्रुव {
  प्रतिज्ञा(EndIdx && "EndIdx is empty?");
  प्रत्यागम *EndIdx;
}

unsigned Suffix_वृक्ष_Leaf_गाँठ::getSuffixIdx() ध्रुव { प्रत्यागम SuffixIdx; }
शून्य Suffix_वृक्ष_Leaf_गाँठ::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned Suffix_वृक्ष_गाँठ::get_बायाँ_Leaf_Idx() ध्रुव { प्रत्यागम बायाँ_Leaf_Idx; }
unsigned Suffix_वृक्ष_गाँठ::get_दायाँ_Leaf_Idx() ध्रुव { प्रत्यागम दायाँ_Leaf_Idx; }
शून्य Suffix_वृक्ष_गाँठ::set_बायाँ_Leaf_Idx(unsigned Idx) { बायाँ_Leaf_Idx = Idx; }
शून्य Suffix_वृक्ष_गाँठ::set_दायाँ_Leaf_Idx(unsigned Idx) { दायाँ_Leaf_Idx = Idx; }
