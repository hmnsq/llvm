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

अचिन्हित Suffix_वृक्ष_गाठ::getStartIdx() स्थिरांक { परत StartIdx; }
शून्य Suffix_वृक्ष_गाठ::incrementStartIdx(अचिन्हित Inc) { StartIdx += Inc; }
शून्य Suffix_वृक्ष_गाठ::setConcatLen(अचिन्हित Len) { ConcatLen = Len; }
अचिन्हित Suffix_वृक्ष_गाठ::getConcatLen() स्थिरांक { परत ConcatLen; }

तार्किक Suffix_वृक्ष_Internal_गाठ::is_मूळ() स्थिरांक {
  परत getStartIdx() == EmptyIdx;
}
अचिन्हित Suffix_वृक्ष_Internal_गाठ::getEndIdx() स्थिरांक { परत EndIdx; }
शून्य Suffix_वृक्ष_Internal_गाठ::setLink(Suffix_वृक्ष_Internal_गाठ *L) {
  प्रतिज्ञा(L && "Cannot set a null link?");
  Link = L;
}
Suffix_वृक्ष_Internal_गाठ *Suffix_वृक्ष_Internal_गाठ::getLink() स्थिरांक { परत Link; }

अचिन्हित Suffix_वृक्ष_Leaf_गाठ::getEndIdx() स्थिरांक {
  प्रतिज्ञा(EndIdx && "EndIdx is empty?");
  परत *EndIdx;
}

अचिन्हित Suffix_वृक्ष_Leaf_गाठ::getSuffixIdx() स्थिरांक { परत SuffixIdx; }
शून्य Suffix_वृक्ष_Leaf_गाठ::setSuffixIdx(अचिन्हित Idx) { SuffixIdx = Idx; }

अचिन्हित Suffix_वृक्ष_गाठ::get_डावा_Leaf_Idx() स्थिरांक { परत डावा_Leaf_Idx; }
अचिन्हित Suffix_वृक्ष_गाठ::get_उजवा_Leaf_Idx() स्थिरांक { परत उजवा_Leaf_Idx; }
शून्य Suffix_वृक्ष_गाठ::set_डावा_Leaf_Idx(अचिन्हित Idx) { डावा_Leaf_Idx = Idx; }
शून्य Suffix_वृक्ष_गाठ::set_उजवा_Leaf_Idx(अचिन्हित Idx) { उजवा_Leaf_Idx = Idx; }
