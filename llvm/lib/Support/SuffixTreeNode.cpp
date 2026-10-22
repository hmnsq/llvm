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

#অন্তর্ভাব "llvm/Support/SuffixTreeNode.h"

using নামাকাশ llvm;

unsigned Suffix_বৃক্ষ_গ্রন্থি::getStartIdx() ধ্রুবক { ফেরত StartIdx; }
শূন্য Suffix_বৃক্ষ_গ্রন্থি::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
শূন্য Suffix_বৃক্ষ_গ্রন্থি::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned Suffix_বৃক্ষ_গ্রন্থি::getConcatLen() ধ্রুবক { ফেরত ConcatLen; }

যৌক্তিক Suffix_বৃক্ষ_Internal_গ্রন্থি::is_মূল() ধ্রুবক {
  ফেরত getStartIdx() == EmptyIdx;
}
unsigned Suffix_বৃক্ষ_Internal_গ্রন্থি::getEndIdx() ধ্রুবক { ফেরত EndIdx; }
শূন্য Suffix_বৃক্ষ_Internal_গ্রন্থি::setLink(Suffix_বৃক্ষ_Internal_গ্রন্থি *L) {
  প্রতিজ্ঞা(L && "Cannot set a null link?");
  Link = L;
}
Suffix_বৃক্ষ_Internal_গ্রন্থি *Suffix_বৃক্ষ_Internal_গ্রন্থি::getLink() ধ্রুবক { ফেরত Link; }

unsigned Suffix_বৃক্ষ_Leaf_গ্রন্থি::getEndIdx() ধ্রুবক {
  প্রতিজ্ঞা(EndIdx && "EndIdx is empty?");
  ফেরত *EndIdx;
}

unsigned Suffix_বৃক্ষ_Leaf_গ্রন্থি::getSuffixIdx() ধ্রুবক { ফেরত SuffixIdx; }
শূন্য Suffix_বৃক্ষ_Leaf_গ্রন্থি::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned Suffix_বৃক্ষ_গ্রন্থি::get_বাম_Leaf_Idx() ধ্রুবক { ফেরত বাম_Leaf_Idx; }
unsigned Suffix_বৃক্ষ_গ্রন্থি::get_ডান_Leaf_Idx() ধ্রুবক { ফেরত ডান_Leaf_Idx; }
শূন্য Suffix_বৃক্ষ_গ্রন্থি::set_বাম_Leaf_Idx(unsigned Idx) { বাম_Leaf_Idx = Idx; }
শূন্য Suffix_বৃক্ষ_গ্রন্থি::set_ডান_Leaf_Idx(unsigned Idx) { ডান_Leaf_Idx = Idx; }
