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

unsigned Suffix_वृक्षः_ग्रन्थिः::getStartIdx() ध्रुव { प्रत्यागम StartIdx; }
शून्य Suffix_वृक्षः_ग्रन्थिः::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
शून्य Suffix_वृक्षः_ग्रन्थिः::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned Suffix_वृक्षः_ग्रन्थिः::getConcatLen() ध्रुव { प्रत्यागम ConcatLen; }

तार्किक Suffix_वृक्षः_Internal_ग्रन्थिः::is_मूलम्() ध्रुव {
  प्रत्यागम getStartIdx() == EmptyIdx;
}
unsigned Suffix_वृक्षः_Internal_ग्रन्थिः::getEndIdx() ध्रुव { प्रत्यागम EndIdx; }
शून्य Suffix_वृक्षः_Internal_ग्रन्थिः::setLink(Suffix_वृक्षः_Internal_ग्रन्थिः *L) {
  प्रतिज्ञा(L && "Cannot set a null link?");
  Link = L;
}
Suffix_वृक्षः_Internal_ग्रन्थिः *Suffix_वृक्षः_Internal_ग्रन्थिः::getLink() ध्रुव { प्रत्यागम Link; }

unsigned Suffix_वृक्षः_Leaf_ग्रन्थिः::getEndIdx() ध्रुव {
  प्रतिज्ञा(EndIdx && "EndIdx is empty?");
  प्रत्यागम *EndIdx;
}

unsigned Suffix_वृक्षः_Leaf_ग्रन्थिः::getSuffixIdx() ध्रुव { प्रत्यागम SuffixIdx; }
शून्य Suffix_वृक्षः_Leaf_ग्रन्थिः::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned Suffix_वृक्षः_ग्रन्थिः::get_वामः_Leaf_Idx() ध्रुव { प्रत्यागम वामः_Leaf_Idx; }
unsigned Suffix_वृक्षः_ग्रन्थिः::get_दक्षिणः_Leaf_Idx() ध्रुव { प्रत्यागम दक्षिणः_Leaf_Idx; }
शून्य Suffix_वृक्षः_ग्रन्थिः::set_वामः_Leaf_Idx(unsigned Idx) { वामः_Leaf_Idx = Idx; }
शून्य Suffix_वृक्षः_ग्रन्थिः::set_दक्षिणः_Leaf_Idx(unsigned Idx) { दक्षिणः_Leaf_Idx = Idx; }
