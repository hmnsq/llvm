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

unsigned Suffix_دەرەخ_تۈگۈن::getStartIdx() ثابت { أرجع StartIdx; }
فراغ Suffix_دەرەخ_تۈگۈن::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
فراغ Suffix_دەرەخ_تۈگۈن::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned Suffix_دەرەخ_تۈگۈن::getConcatLen() ثابت { أرجع ConcatLen; }

منطقي Suffix_دەرەخ_Internal_تۈگۈن::is_يىلتىز() ثابت {
  أرجع getStartIdx() == EmptyIdx;
}
unsigned Suffix_دەرەخ_Internal_تۈگۈن::getEndIdx() ثابت { أرجع EndIdx; }
فراغ Suffix_دەرەخ_Internal_تۈگۈن::setLink(Suffix_دەرەخ_Internal_تۈگۈن *L) {
  أكد(L && "Cannot set a null link?");
  Link = L;
}
Suffix_دەرەخ_Internal_تۈگۈن *Suffix_دەرەخ_Internal_تۈگۈن::getLink() ثابت { أرجع Link; }

unsigned Suffix_دەرەخ_Leaf_تۈگۈن::getEndIdx() ثابت {
  أكد(EndIdx && "EndIdx is empty?");
  أرجع *EndIdx;
}

unsigned Suffix_دەرەخ_Leaf_تۈگۈن::getSuffixIdx() ثابت { أرجع SuffixIdx; }
فراغ Suffix_دەرەخ_Leaf_تۈگۈن::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned Suffix_دەرەخ_تۈگۈن::get_سول_Leaf_Idx() ثابت { أرجع سول_Leaf_Idx; }
unsigned Suffix_دەرەخ_تۈگۈن::get_ئوڭ_Leaf_Idx() ثابت { أرجع ئوڭ_Leaf_Idx; }
فراغ Suffix_دەرەخ_تۈگۈن::set_سول_Leaf_Idx(unsigned Idx) { سول_Leaf_Idx = Idx; }
فراغ Suffix_دەرەخ_تۈگۈن::set_ئوڭ_Leaf_Idx(unsigned Idx) { ئوڭ_Leaf_Idx = Idx; }
