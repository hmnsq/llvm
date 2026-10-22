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

استفاده نطاق llvm;

بیعلامت Suffix_درخت_گره::get_شروع_Idx() ثابت { أرجع شروع_Idx; }
فراغ Suffix_درخت_گره::increment_شروع_Idx(بیعلامت Inc) { شروع_Idx += Inc; }
فراغ Suffix_درخت_گره::مجموعه_Concat_Len(بیعلامت Len) { ConcatLen = Len; }
بیعلامت Suffix_درخت_گره::getConcatLen() ثابت { أرجع ConcatLen; }

منطقي Suffix_درخت_داخلی_گره::is_ریشه() ثابت {
  أرجع get_شروع_Idx() == خالی_Idx;
}
بیعلامت Suffix_درخت_داخلی_گره::get_انتها_Idx() ثابت { أرجع انتها_Idx; }
فراغ Suffix_درخت_داخلی_گره::مجموعه_Link(Suffix_درخت_داخلی_گره *L) {
  أكد(L && "Cannot set a null link?");
  Link = L;
}
Suffix_درخت_داخلی_گره *Suffix_درخت_داخلی_گره::getLink() ثابت { أرجع Link; }

بیعلامت Suffix_درخت_برگ_گره::get_انتها_Idx() ثابت {
  أكد(انتها_Idx && "EndIdx is empty?");
  أرجع *انتها_Idx;
}

بیعلامت Suffix_درخت_برگ_گره::getSuffixIdx() ثابت { أرجع SuffixIdx; }
فراغ Suffix_درخت_برگ_گره::مجموعه_Suffix_Idx(بیعلامت Idx) { SuffixIdx = Idx; }

بیعلامت Suffix_درخت_گره::get_چپ_برگ_Idx() ثابت { أرجع چپ_برگ_Idx; }
بیعلامت Suffix_درخت_گره::get_راست_برگ_Idx() ثابت { أرجع راست_برگ_Idx; }
فراغ Suffix_درخت_گره::مجموعه_چپ_برگ_Idx(بیعلامت Idx) { چپ_برگ_Idx = Idx; }
فراغ Suffix_درخت_گره::مجموعه_راست_برگ_Idx(بیعلامت Idx) { راست_برگ_Idx = Idx; }
