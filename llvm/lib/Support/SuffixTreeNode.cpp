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

کارول نطاق llvm;

بینښه Suffix_ونه_غوټه::get_پیل_Idx() ثابت { أرجع پیل_Idx; }
فراغ Suffix_ونه_غوټه::increment_پیل_Idx(بینښه Inc) { پیل_Idx += Inc; }
فراغ Suffix_ونه_غوټه::سېټ_Concat_Len(بینښه Len) { ConcatLen = Len; }
بینښه Suffix_ونه_غوټه::getConcatLen() ثابت { أرجع ConcatLen; }

منطقي Suffix_ونه_دننی_غوټه::is_ریښه() ثابت {
  أرجع get_پیل_Idx() == تشه_Idx;
}
بینښه Suffix_ونه_دننی_غوټه::get_پای_ټکی_Idx() ثابت { أرجع پای_ټکی_Idx; }
فراغ Suffix_ونه_دننی_غوټه::سېټ_Link(Suffix_ونه_دننی_غوټه *L) {
  أكد(L && "Cannot set a null link?");
  Link = L;
}
Suffix_ونه_دننی_غوټه *Suffix_ونه_دننی_غوټه::getLink() ثابت { أرجع Link; }

بینښه Suffix_ونه_ونې_پاڼه_غوټه::get_پای_ټکی_Idx() ثابت {
  أكد(پای_ټکی_Idx && "EndIdx is empty?");
  أرجع *پای_ټکی_Idx;
}

بینښه Suffix_ونه_ونې_پاڼه_غوټه::getSuffixIdx() ثابت { أرجع SuffixIdx; }
فراغ Suffix_ونه_ونې_پاڼه_غوټه::سېټ_Suffix_Idx(بینښه Idx) { SuffixIdx = Idx; }

بینښه Suffix_ونه_غوټه::get_کیڼ_ونې_پاڼه_Idx() ثابت { أرجع کیڼ_ونې_پاڼه_Idx; }
بینښه Suffix_ونه_غوټه::get_ښی_ونې_پاڼه_Idx() ثابت { أرجع ښی_ونې_پاڼه_Idx; }
فراغ Suffix_ونه_غوټه::سېټ_کیڼ_ونې_پاڼه_Idx(بینښه Idx) { کیڼ_ونې_پاڼه_Idx = Idx; }
فراغ Suffix_ونه_غوټه::سېټ_ښی_ونې_پاڼه_Idx(بینښه Idx) { ښی_ونې_پاڼه_Idx = Idx; }
