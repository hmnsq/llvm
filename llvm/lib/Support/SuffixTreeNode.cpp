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

#অন্তর্ভাৱ "llvm/Support/SuffixTreeNode.h"

using নামাকাশ llvm;

unsigned Suffix_বৃক্ষ_গ্ৰন্থি::getStartIdx() ধ্রুৱ { ঘূৰাই StartIdx; }
শূন্য Suffix_বৃক্ষ_গ্ৰন্থি::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
শূন্য Suffix_বৃক্ষ_গ্ৰন্থি::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned Suffix_বৃক্ষ_গ্ৰন্থি::getConcatLen() ধ্রুৱ { ঘূৰাই ConcatLen; }

তার্কিক Suffix_বৃক্ষ_Internal_গ্ৰন্থি::is_মূল() ধ্রুৱ {
  ঘূৰাই getStartIdx() == EmptyIdx;
}
unsigned Suffix_বৃক্ষ_Internal_গ্ৰন্থি::getEndIdx() ধ্রুৱ { ঘূৰাই EndIdx; }
শূন্য Suffix_বৃক্ষ_Internal_গ্ৰন্থি::setLink(Suffix_বৃক্ষ_Internal_গ্ৰন্থি *L) {
  প্রতিজ্ঞা(L && "Cannot set a null link?");
  Link = L;
}
Suffix_বৃক্ষ_Internal_গ্ৰন্থি *Suffix_বৃক্ষ_Internal_গ্ৰন্থি::getLink() ধ্রুৱ { ঘূৰাই Link; }

unsigned Suffix_বৃক্ষ_Leaf_গ্ৰন্থি::getEndIdx() ধ্রুৱ {
  প্রতিজ্ঞা(EndIdx && "EndIdx is empty?");
  ঘূৰাই *EndIdx;
}

unsigned Suffix_বৃক্ষ_Leaf_গ্ৰন্থি::getSuffixIdx() ধ্রুৱ { ঘূৰাই SuffixIdx; }
শূন্য Suffix_বৃক্ষ_Leaf_গ্ৰন্থি::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned Suffix_বৃক্ষ_গ্ৰন্থি::get_বাম_Leaf_Idx() ধ্রুৱ { ঘূৰাই বাম_Leaf_Idx; }
unsigned Suffix_বৃক্ষ_গ্ৰন্থি::get_ডান_Leaf_Idx() ধ্রুৱ { ঘূৰাই ডান_Leaf_Idx; }
শূন্য Suffix_বৃক্ষ_গ্ৰন্থি::set_বাম_Leaf_Idx(unsigned Idx) { বাম_Leaf_Idx = Idx; }
শূন্য Suffix_বৃক্ষ_গ্ৰন্থি::set_ডান_Leaf_Idx(unsigned Idx) { ডান_Leaf_Idx = Idx; }
