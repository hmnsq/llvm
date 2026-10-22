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

unsigned Suffix_বৃক্ষ_গ্রন্থি::getStartIdx() ধ্রুব { প্রত্যাগম StartIdx; }
শূন্য Suffix_বৃক্ষ_গ্রন্থি::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
শূন্য Suffix_বৃক্ষ_গ্রন্থি::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned Suffix_বৃক্ষ_গ্রন্থি::getConcatLen() ধ্রুব { প্রত্যাগম ConcatLen; }

তার্কিক Suffix_বৃক্ষ_Internal_গ্রন্থি::is_মূল() ধ্রুব {
  প্রত্যাগম getStartIdx() == EmptyIdx;
}
unsigned Suffix_বৃক্ষ_Internal_গ্রন্থি::getEndIdx() ধ্রুব { প্রত্যাগম EndIdx; }
শূন্য Suffix_বৃক্ষ_Internal_গ্রন্থি::setLink(Suffix_বৃক্ষ_Internal_গ্রন্থি *L) {
  প্রতিজ্ঞা(L && "Cannot set a null link?");
  Link = L;
}
Suffix_বৃক্ষ_Internal_গ্রন্থি *Suffix_বৃক্ষ_Internal_গ্রন্থি::getLink() ধ্রুব { প্রত্যাগম Link; }

unsigned Suffix_বৃক্ষ_Leaf_গ্রন্থি::getEndIdx() ধ্রুব {
  প্রতিজ্ঞা(EndIdx && "EndIdx is empty?");
  প্রত্যাগম *EndIdx;
}

unsigned Suffix_বৃক্ষ_Leaf_গ্রন্থি::getSuffixIdx() ধ্রুব { প্রত্যাগম SuffixIdx; }
শূন্য Suffix_বৃক্ষ_Leaf_গ্রন্থি::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned Suffix_বৃক্ষ_গ্রন্থি::get_বাম_Leaf_Idx() ধ্রুব { প্রত্যাগম বাম_Leaf_Idx; }
unsigned Suffix_বৃক্ষ_গ্রন্থি::get_ডান_Leaf_Idx() ধ্রুব { প্রত্যাগম ডান_Leaf_Idx; }
শূন্য Suffix_বৃক্ষ_গ্রন্থি::set_বাম_Leaf_Idx(unsigned Idx) { বাম_Leaf_Idx = Idx; }
শূন্য Suffix_বৃক্ষ_গ্রন্থি::set_ডান_Leaf_Idx(unsigned Idx) { ডান_Leaf_Idx = Idx; }
