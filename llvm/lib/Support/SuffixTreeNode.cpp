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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/SuffixTreeNode.h"

using ନାମାକାଶ llvm;

unsigned Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି::getStartIdx() ଅଚଳ { ଫେରନ୍ତୁ StartIdx; }
ଶୂନ୍ୟ Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
ଶୂନ୍ୟ Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି::getConcatLen() ଅଚଳ { ଫେରନ୍ତୁ ConcatLen; }

ତାର୍କିକ Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି::is_ମୂଳ() ଅଚଳ {
  ଫେରନ୍ତୁ getStartIdx() == EmptyIdx;
}
unsigned Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି::getEndIdx() ଅଚଳ { ଫେରନ୍ତୁ EndIdx; }
ଶୂନ୍ୟ Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି::setLink(Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି *L) {
  ପ୍ରତିଜ୍ଞା(L && "Cannot set a null link?");
  Link = L;
}
Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି *Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି::getLink() ଅଚଳ { ଫେରନ୍ତୁ Link; }

unsigned Suffix_ବୃକ୍ଷ_Leaf_ଗଣ୍ଠି::getEndIdx() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(EndIdx && "EndIdx is empty?");
  ଫେରନ୍ତୁ *EndIdx;
}

unsigned Suffix_ବୃକ୍ଷ_Leaf_ଗଣ୍ଠି::getSuffixIdx() ଅଚଳ { ଫେରନ୍ତୁ SuffixIdx; }
ଶୂନ୍ୟ Suffix_ବୃକ୍ଷ_Leaf_ଗଣ୍ଠି::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି::get_ବାମ_Leaf_Idx() ଅଚଳ { ଫେରନ୍ତୁ ବାମ_Leaf_Idx; }
unsigned Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି::get_ଡାହାଣ_Leaf_Idx() ଅଚଳ { ଫେରନ୍ତୁ ଡାହାଣ_Leaf_Idx; }
ଶୂନ୍ୟ Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି::set_ବାମ_Leaf_Idx(unsigned Idx) { ବାମ_Leaf_Idx = Idx; }
ଶୂନ୍ୟ Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି::set_ଡାହାଣ_Leaf_Idx(unsigned Idx) { ଡାହାଣ_Leaf_Idx = Idx; }
