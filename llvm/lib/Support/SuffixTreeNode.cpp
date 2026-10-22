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

unsigned Suffix_ڤوکوق_نود::getStartIdx() ثابت { أرجع StartIdx; }
فراغ Suffix_ڤوکوق_نود::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
فراغ Suffix_ڤوکوق_نود::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned Suffix_ڤوکوق_نود::getConcatLen() ثابت { أرجع ConcatLen; }

منطقي Suffix_ڤوکوق_Internal_نود::is_اکر() ثابت {
  أرجع getStartIdx() == EmptyIdx;
}
unsigned Suffix_ڤوکوق_Internal_نود::getEndIdx() ثابت { أرجع EndIdx; }
فراغ Suffix_ڤوکوق_Internal_نود::setLink(Suffix_ڤوکوق_Internal_نود *L) {
  أكد(L && "Cannot set a null link?");
  Link = L;
}
Suffix_ڤوکوق_Internal_نود *Suffix_ڤوکوق_Internal_نود::getLink() ثابت { أرجع Link; }

unsigned Suffix_ڤوکوق_Leaf_نود::getEndIdx() ثابت {
  أكد(EndIdx && "EndIdx is empty?");
  أرجع *EndIdx;
}

unsigned Suffix_ڤوکوق_Leaf_نود::getSuffixIdx() ثابت { أرجع SuffixIdx; }
فراغ Suffix_ڤوکوق_Leaf_نود::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned Suffix_ڤوکوق_نود::get_کيري_Leaf_Idx() ثابت { أرجع کيري_Leaf_Idx; }
unsigned Suffix_ڤوکوق_نود::get_کانن_Leaf_Idx() ثابت { أرجع کانن_Leaf_Idx; }
فراغ Suffix_ڤوکوق_نود::set_کيري_Leaf_Idx(unsigned Idx) { کيري_Leaf_Idx = Idx; }
فراغ Suffix_ڤوکوق_نود::set_کانن_Leaf_Idx(unsigned Idx) { کانن_Leaf_Idx = Idx; }
