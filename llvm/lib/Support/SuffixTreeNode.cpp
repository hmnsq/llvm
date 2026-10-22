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

#समावेश "llvm/Support/SuffixTreeNode.h"

using namespace llvm;

अचिन्हित Suffix_वृक्ष_गाँठ::getStartIdx() स्थिर { वापस StartIdx; }
व्योम Suffix_वृक्ष_गाँठ::incrementStartIdx(अचिन्हित Inc) { StartIdx += Inc; }
व्योम Suffix_वृक्ष_गाँठ::setConcatLen(अचिन्हित Len) { ConcatLen = Len; }
अचिन्हित Suffix_वृक्ष_गाँठ::getConcatLen() स्थिर { वापस ConcatLen; }

bool Suffix_वृक्ष_Internal_गाँठ::is_मूल() स्थिर {
  वापस getStartIdx() == EmptyIdx;
}
अचिन्हित Suffix_वृक्ष_Internal_गाँठ::getEndIdx() स्थिर { वापस EndIdx; }
व्योम Suffix_वृक्ष_Internal_गाँठ::setLink(Suffix_वृक्ष_Internal_गाँठ *L) {
  निश्चित(L && "Cannot set a null link?");
  Link = L;
}
Suffix_वृक्ष_Internal_गाँठ *Suffix_वृक्ष_Internal_गाँठ::getLink() स्थिर { वापस Link; }

अचिन्हित Suffix_वृक्ष_Leaf_गाँठ::getEndIdx() स्थिर {
  निश्चित(EndIdx && "EndIdx is empty?");
  वापस *EndIdx;
}

अचिन्हित Suffix_वृक्ष_Leaf_गाँठ::getSuffixIdx() स्थिर { वापस SuffixIdx; }
व्योम Suffix_वृक्ष_Leaf_गाँठ::setSuffixIdx(अचिन्हित Idx) { SuffixIdx = Idx; }

अचिन्हित Suffix_वृक्ष_गाँठ::get_बायाँ_Leaf_Idx() स्थिर { वापस बायाँ_Leaf_Idx; }
अचिन्हित Suffix_वृक्ष_गाँठ::get_दायाँ_Leaf_Idx() स्थिर { वापस दायाँ_Leaf_Idx; }
व्योम Suffix_वृक्ष_गाँठ::set_बायाँ_Leaf_Idx(अचिन्हित Idx) { बायाँ_Leaf_Idx = Idx; }
व्योम Suffix_वृक्ष_गाँठ::set_दायाँ_Leaf_Idx(अचिन्हित Idx) { दायाँ_Leaf_Idx = Idx; }
