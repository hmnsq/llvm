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

استعمال نطاق llvm;

بنانشان Suffix_وڻ_ڳنڍ::get_شروعات_Idx() ثابت { أرجع شروعات_Idx; }
فراغ Suffix_وڻ_ڳنڍ::increment_شروعات_Idx(بنانشان Inc) { شروعات_Idx += Inc; }
فراغ Suffix_وڻ_ڳنڍ::سيٽ_Concat_Len(بنانشان Len) { ConcatLen = Len; }
بنانشان Suffix_وڻ_ڳنڍ::getConcatLen() ثابت { أرجع ConcatLen; }

منطقي Suffix_وڻ_اندروني_ڳنڍ::is_پاڙ() ثابت {
  أرجع get_شروعات_Idx() == خالي_جايون_Idx;
}
بنانشان Suffix_وڻ_اندروني_ڳنڍ::get_پڇاڙي_Idx() ثابت { أرجع پڇاڙي_Idx; }
فراغ Suffix_وڻ_اندروني_ڳنڍ::سيٽ_Link(Suffix_وڻ_اندروني_ڳنڍ *L) {
  أكد(L && "Cannot set a null link?");
  Link = L;
}
Suffix_وڻ_اندروني_ڳنڍ *Suffix_وڻ_اندروني_ڳنڍ::getLink() ثابت { أرجع Link; }

بنانشان Suffix_وڻ_پن_ڳنڍ::get_پڇاڙي_Idx() ثابت {
  أكد(پڇاڙي_Idx && "EndIdx is empty?");
  أرجع *پڇاڙي_Idx;
}

بنانشان Suffix_وڻ_پن_ڳنڍ::getSuffixIdx() ثابت { أرجع SuffixIdx; }
فراغ Suffix_وڻ_پن_ڳنڍ::سيٽ_Suffix_Idx(بنانشان Idx) { SuffixIdx = Idx; }

بنانشان Suffix_وڻ_ڳنڍ::get_کاٻو_پن_Idx() ثابت { أرجع کاٻو_پن_Idx; }
بنانشان Suffix_وڻ_ڳنڍ::get_ساڄو_پن_Idx() ثابت { أرجع ساڄو_پن_Idx; }
فراغ Suffix_وڻ_ڳنڍ::سيٽ_کاٻو_پن_Idx(بنانشان Idx) { کاٻو_پن_Idx = Idx; }
فراغ Suffix_وڻ_ڳنڍ::سيٽ_ساڄو_پن_Idx(بنانشان Idx) { ساڄو_پن_Idx = Idx; }
