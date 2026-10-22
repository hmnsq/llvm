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

ورتوں نطاق llvm;

بنانشانی Suffix_درخت_گرہ::get_شروع_Idx() ثابت { أرجع شروع_Idx; }
فراغ Suffix_درخت_گرہ::increment_شروع_Idx(بنانشانی Inc) { شروع_Idx += Inc; }
فراغ Suffix_درخت_گرہ::سیٹ_Concat_Len(بنانشانی Len) { ConcatLen = Len; }
بنانشانی Suffix_درخت_گرہ::getConcatLen() ثابت { أرجع ConcatLen; }

منطقي Suffix_درخت_اندرونی_گرہ::is_جڑ() ثابت {
  أرجع get_شروع_Idx() == خالی_تھاواں_Idx;
}
بنانشانی Suffix_درخت_اندرونی_گرہ::get_سرا_Idx() ثابت { أرجع سرا_Idx; }
فراغ Suffix_درخت_اندرونی_گرہ::سیٹ_Link(Suffix_درخت_اندرونی_گرہ *L) {
  أكد(L && "Cannot set a null link?");
  Link = L;
}
Suffix_درخت_اندرونی_گرہ *Suffix_درخت_اندرونی_گرہ::getLink() ثابت { أرجع Link; }

بنانشانی Suffix_درخت_پتی_گرہ::get_سرا_Idx() ثابت {
  أكد(سرا_Idx && "EndIdx is empty?");
  أرجع *سرا_Idx;
}

بنانشانی Suffix_درخت_پتی_گرہ::getSuffixIdx() ثابت { أرجع SuffixIdx; }
فراغ Suffix_درخت_پتی_گرہ::سیٹ_Suffix_Idx(بنانشانی Idx) { SuffixIdx = Idx; }

بنانشانی Suffix_درخت_گرہ::get_بایاں_پتی_Idx() ثابت { أرجع بایاں_پتی_Idx; }
بنانشانی Suffix_درخت_گرہ::get_دایاں_پتی_Idx() ثابت { أرجع دایاں_پتی_Idx; }
فراغ Suffix_درخت_گرہ::سیٹ_بایاں_پتی_Idx(بنانشانی Idx) { بایاں_پتی_Idx = Idx; }
فراغ Suffix_درخت_گرہ::سیٹ_دایاں_پتی_Idx(بنانشانی Idx) { دایاں_پتی_Idx = Idx; }
