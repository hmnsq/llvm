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

उपयुज्य नामाकाश llvm;

अचिह्नित Suffix_वृक्षः_ग्रन्थिः::getStartIdx() ध्रुव { प्रत्यागम StartIdx; }
शून्य Suffix_वृक्षः_ग्रन्थिः::incrementStartIdx(अचिह्नित Inc) { StartIdx += Inc; }
शून्य Suffix_वृक्षः_ग्रन्थिः::समुच्चयः_Concat_Len(अचिह्नित Len) { ConcatLen = Len; }
अचिह्नित Suffix_वृक्षः_ग्रन्थिः::getConcatLen() ध्रुव { प्रत्यागम ConcatLen; }

तार्किक Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः::is_मूलम्() ध्रुव {
  प्रत्यागम getStartIdx() == रिक्तानि_Idx;
}
अचिह्नित Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः::get_अन्तः_Idx() ध्रुव { प्रत्यागम अन्तः_Idx; }
शून्य Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः::समुच्चयः_Link(Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः *L) {
  प्रतिज्ञा(L && "Cannot set a null link?");
  Link = L;
}
Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः *Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः::getLink() ध्रुव { प्रत्यागम Link; }

अचिह्नित Suffix_वृक्षः_पर्णम्_ग्रन्थिः::get_अन्तः_Idx() ध्रुव {
  प्रतिज्ञा(अन्तः_Idx && "EndIdx is empty?");
  प्रत्यागम *अन्तः_Idx;
}

अचिह्नित Suffix_वृक्षः_पर्णम्_ग्रन्थिः::getSuffixIdx() ध्रुव { प्रत्यागम SuffixIdx; }
शून्य Suffix_वृक्षः_पर्णम्_ग्रन्थिः::समुच्चयः_Suffix_Idx(अचिह्नित Idx) { SuffixIdx = Idx; }

अचिह्नित Suffix_वृक्षः_ग्रन्थिः::get_वामः_पर्णम्_Idx() ध्रुव { प्रत्यागम वामः_पर्णम्_Idx; }
अचिह्नित Suffix_वृक्षः_ग्रन्थिः::get_दक्षिणः_पर्णम्_Idx() ध्रुव { प्रत्यागम दक्षिणः_पर्णम्_Idx; }
शून्य Suffix_वृक्षः_ग्रन्थिः::समुच्चयः_वामः_पर्णम्_Idx(अचिह्नित Idx) { वामः_पर्णम्_Idx = Idx; }
शून्य Suffix_वृक्षः_ग्रन्थिः::समुच्चयः_दक्षिणः_पर्णम्_Idx(अचिह्नित Idx) { दक्षिणः_पर्णम्_Idx = Idx; }
