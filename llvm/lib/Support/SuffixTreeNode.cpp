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

unsigned Suffix_درخت_گرہ::getStartIdx() ثابت { أرجع StartIdx; }
فراغ Suffix_درخت_گرہ::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
فراغ Suffix_درخت_گرہ::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned Suffix_درخت_گرہ::getConcatLen() ثابت { أرجع ConcatLen; }

منطقي Suffix_درخت_Internal_گرہ::is_جڑ() ثابت {
  أرجع getStartIdx() == EmptyIdx;
}
unsigned Suffix_درخت_Internal_گرہ::getEndIdx() ثابت { أرجع EndIdx; }
فراغ Suffix_درخت_Internal_گرہ::setLink(Suffix_درخت_Internal_گرہ *L) {
  أكد(L && "Cannot set a null link?");
  Link = L;
}
Suffix_درخت_Internal_گرہ *Suffix_درخت_Internal_گرہ::getLink() ثابت { أرجع Link; }

unsigned Suffix_درخت_Leaf_گرہ::getEndIdx() ثابت {
  أكد(EndIdx && "EndIdx is empty?");
  أرجع *EndIdx;
}

unsigned Suffix_درخت_Leaf_گرہ::getSuffixIdx() ثابت { أرجع SuffixIdx; }
فراغ Suffix_درخت_Leaf_گرہ::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned Suffix_درخت_گرہ::get_بایاں_Leaf_Idx() ثابت { أرجع بایاں_Leaf_Idx; }
unsigned Suffix_درخت_گرہ::get_دایاں_Leaf_Idx() ثابت { أرجع دایاں_Leaf_Idx; }
فراغ Suffix_درخت_گرہ::set_بایاں_Leaf_Idx(unsigned Idx) { بایاں_Leaf_Idx = Idx; }
فراغ Suffix_درخت_گرہ::set_دایاں_Leaf_Idx(unsigned Idx) { دایاں_Leaf_Idx = Idx; }
