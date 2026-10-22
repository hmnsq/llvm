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

unsigned Suffix_شجرة_عقدة::getStartIdx() ثابت { أرجع StartIdx; }
فراغ Suffix_شجرة_عقدة::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
فراغ Suffix_شجرة_عقدة::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned Suffix_شجرة_عقدة::getConcatLen() ثابت { أرجع ConcatLen; }

منطقي Suffix_شجرة_Internal_عقدة::is_جذر() ثابت {
  أرجع getStartIdx() == EmptyIdx;
}
unsigned Suffix_شجرة_Internal_عقدة::getEndIdx() ثابت { أرجع EndIdx; }
فراغ Suffix_شجرة_Internal_عقدة::setLink(Suffix_شجرة_Internal_عقدة *L) {
  أكد(L && "Cannot set a null link?");
  Link = L;
}
Suffix_شجرة_Internal_عقدة *Suffix_شجرة_Internal_عقدة::getLink() ثابت { أرجع Link; }

unsigned Suffix_شجرة_Leaf_عقدة::getEndIdx() ثابت {
  أكد(EndIdx && "EndIdx is empty?");
  أرجع *EndIdx;
}

unsigned Suffix_شجرة_Leaf_عقدة::getSuffixIdx() ثابت { أرجع SuffixIdx; }
فراغ Suffix_شجرة_Leaf_عقدة::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned Suffix_شجرة_عقدة::get_يسار_Leaf_Idx() ثابت { أرجع يسار_Leaf_Idx; }
unsigned Suffix_شجرة_عقدة::get_يمين_Leaf_Idx() ثابت { أرجع يمين_Leaf_Idx; }
فراغ Suffix_شجرة_عقدة::set_يسار_Leaf_Idx(unsigned Idx) { يسار_Leaf_Idx = Idx; }
فراغ Suffix_شجرة_عقدة::set_يمين_Leaf_Idx(unsigned Idx) { يمين_Leaf_Idx = Idx; }
