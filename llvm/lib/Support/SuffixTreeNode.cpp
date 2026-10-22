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

unsigned Suffix_بشيا_كلي::getStartIdx() ثابت { أرجع StartIdx; }
فراغ Suffix_بشيا_كلي::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
فراغ Suffix_بشيا_كلي::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned Suffix_بشيا_كلي::getConcatLen() ثابت { أرجع ConcatLen; }

منطقي Suffix_بشيا_Internal_كلي::is_سيو() ثابت {
  أرجع getStartIdx() == EmptyIdx;
}
unsigned Suffix_بشيا_Internal_كلي::getEndIdx() ثابت { أرجع EndIdx; }
فراغ Suffix_بشيا_Internal_كلي::setLink(Suffix_بشيا_Internal_كلي *L) {
  أكد(L && "Cannot set a null link?");
  Link = L;
}
Suffix_بشيا_Internal_كلي *Suffix_بشيا_Internal_كلي::getLink() ثابت { أرجع Link; }

unsigned Suffix_بشيا_Leaf_كلي::getEndIdx() ثابت {
  أكد(EndIdx && "EndIdx is empty?");
  أرجع *EndIdx;
}

unsigned Suffix_بشيا_Leaf_كلي::getSuffixIdx() ثابت { أرجع SuffixIdx; }
فراغ Suffix_بشيا_Leaf_كلي::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned Suffix_بشيا_كلي::get_هغو_Leaf_Idx() ثابت { أرجع هغو_Leaf_Idx; }
unsigned Suffix_بشيا_كلي::get_دام_Leaf_Idx() ثابت { أرجع دام_Leaf_Idx; }
فراغ Suffix_بشيا_كلي::set_هغو_Leaf_Idx(unsigned Idx) { هغو_Leaf_Idx = Idx; }
فراغ Suffix_بشيا_كلي::set_دام_Leaf_Idx(unsigned Idx) { دام_Leaf_Idx = Idx; }
