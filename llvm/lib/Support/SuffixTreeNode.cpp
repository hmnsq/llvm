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

#ضمن "llvm/Support/SuffixTreeNode.h"

using نطاق llvm;

unsigned Suffix_درەخت_گرێ::getStartIdx() ثابت { أرجع StartIdx; }
فراغ Suffix_درەخت_گرێ::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
فراغ Suffix_درەخت_گرێ::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned Suffix_درەخت_گرێ::getConcatLen() ثابت { أرجع ConcatLen; }

منطقي Suffix_درەخت_Internal_گرێ::is_ڕەگ() ثابت {
  أرجع getStartIdx() == EmptyIdx;
}
unsigned Suffix_درەخت_Internal_گرێ::getEndIdx() ثابت { أرجع EndIdx; }
فراغ Suffix_درەخت_Internal_گرێ::setLink(Suffix_درەخت_Internal_گرێ *L) {
  أكد(L && "Cannot set a null link?");
  Link = L;
}
Suffix_درەخت_Internal_گرێ *Suffix_درەخت_Internal_گرێ::getLink() ثابت { أرجع Link; }

unsigned Suffix_درەخت_Leaf_گرێ::getEndIdx() ثابت {
  أكد(EndIdx && "EndIdx is empty?");
  أرجع *EndIdx;
}

unsigned Suffix_درەخت_Leaf_گرێ::getSuffixIdx() ثابت { أرجع SuffixIdx; }
فراغ Suffix_درەخت_Leaf_گرێ::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned Suffix_درەخت_گرێ::get_چەپ_Leaf_Idx() ثابت { أرجع چەپ_Leaf_Idx; }
unsigned Suffix_درەخت_گرێ::get_ڕاست_Leaf_Idx() ثابت { أرجع ڕاست_Leaf_Idx; }
فراغ Suffix_درەخت_گرێ::set_چەپ_Leaf_Idx(unsigned Idx) { چەپ_Leaf_Idx = Idx; }
فراغ Suffix_درەخت_گرێ::set_ڕاست_Leaf_Idx(unsigned Idx) { ڕاست_Leaf_Idx = Idx; }
