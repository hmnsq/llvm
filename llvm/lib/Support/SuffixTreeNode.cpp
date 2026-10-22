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

#شامل "llvm/Support/SuffixTreeNode.h"

using فضاینام llvm;

unsigned Suffix_درخت_گره::getStartIdx() ثابت { بازگشت StartIdx; }
تهی Suffix_درخت_گره::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
تهی Suffix_درخت_گره::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned Suffix_درخت_گره::getConcatLen() ثابت { بازگشت ConcatLen; }

منطقی Suffix_درخت_Internal_گره::is_ریشه() ثابت {
  بازگشت getStartIdx() == EmptyIdx;
}
unsigned Suffix_درخت_Internal_گره::getEndIdx() ثابت { بازگشت EndIdx; }
تهی Suffix_درخت_Internal_گره::setLink(Suffix_درخت_Internal_گره *L) {
  أكد(L && "Cannot set a null link?");
  Link = L;
}
Suffix_درخت_Internal_گره *Suffix_درخت_Internal_گره::getLink() ثابت { بازگشت Link; }

unsigned Suffix_درخت_Leaf_گره::getEndIdx() ثابت {
  أكد(EndIdx && "EndIdx is empty?");
  بازگشت *EndIdx;
}

unsigned Suffix_درخت_Leaf_گره::getSuffixIdx() ثابت { بازگشت SuffixIdx; }
تهی Suffix_درخت_Leaf_گره::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned Suffix_درخت_گره::get_چپ_Leaf_Idx() ثابت { بازگشت چپ_Leaf_Idx; }
unsigned Suffix_درخت_گره::get_راست_Leaf_Idx() ثابت { بازگشت راست_Leaf_Idx; }
تهی Suffix_درخت_گره::set_چپ_Leaf_Idx(unsigned Idx) { چپ_Leaf_Idx = Idx; }
تهی Suffix_درخت_گره::set_راست_Leaf_Idx(unsigned Idx) { راست_Leaf_Idx = Idx; }
