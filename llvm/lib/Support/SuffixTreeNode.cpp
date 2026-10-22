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

अचिन्हित Suffix_रूख_गाँठो::getStartIdx() अचर { फर्काऊ StartIdx; }
शून्य Suffix_रूख_गाँठो::incrementStartIdx(अचिन्हित Inc) { StartIdx += Inc; }
शून्य Suffix_रूख_गाँठो::setConcatLen(अचिन्हित Len) { ConcatLen = Len; }
अचिन्हित Suffix_रूख_गाँठो::getConcatLen() अचर { फर्काऊ ConcatLen; }

तार्किक Suffix_रूख_Internal_गाँठो::is_जरा() अचर {
  फर्काऊ getStartIdx() == EmptyIdx;
}
अचिन्हित Suffix_रूख_Internal_गाँठो::getEndIdx() अचर { फर्काऊ EndIdx; }
शून्य Suffix_रूख_Internal_गाँठो::setLink(Suffix_रूख_Internal_गाँठो *L) {
  प्रतिज्ञा(L && "Cannot set a null link?");
  Link = L;
}
Suffix_रूख_Internal_गाँठो *Suffix_रूख_Internal_गाँठो::getLink() अचर { फर्काऊ Link; }

अचिन्हित Suffix_रूख_Leaf_गाँठो::getEndIdx() अचर {
  प्रतिज्ञा(EndIdx && "EndIdx is empty?");
  फर्काऊ *EndIdx;
}

अचिन्हित Suffix_रूख_Leaf_गाँठो::getSuffixIdx() अचर { फर्काऊ SuffixIdx; }
शून्य Suffix_रूख_Leaf_गाँठो::setSuffixIdx(अचिन्हित Idx) { SuffixIdx = Idx; }

अचिन्हित Suffix_रूख_गाँठो::get_बायाँ_Leaf_Idx() अचर { फर्काऊ बायाँ_Leaf_Idx; }
अचिन्हित Suffix_रूख_गाँठो::get_दायाँ_Leaf_Idx() अचर { फर्काऊ दायाँ_Leaf_Idx; }
शून्य Suffix_रूख_गाँठो::set_बायाँ_Leaf_Idx(अचिन्हित Idx) { बायाँ_Leaf_Idx = Idx; }
शून्य Suffix_रूख_गाँठो::set_दायाँ_Leaf_Idx(अचिन्हित Idx) { दायाँ_Leaf_Idx = Idx; }
