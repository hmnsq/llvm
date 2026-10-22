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

استعمال نامخلا llvm;

غیرعلامتی Suffix_درخت_گرہ::get_آغاز_Idx() ثابت { واپس آغاز_Idx; }
خالی Suffix_درخت_گرہ::increment_آغاز_Idx(غیرعلامتی Inc) { آغاز_Idx += Inc; }
خالی Suffix_درخت_گرہ::سیٹ_Concat_Len(غیرعلامتی Len) { ConcatLen = Len; }
غیرعلامتی Suffix_درخت_گرہ::getConcatLen() ثابت { واپس ConcatLen; }

منطقی Suffix_درخت_اندرونی_گرہ::is_جڑ() ثابت {
  واپس get_آغاز_Idx() == خالی_خانے_Idx;
}
غیرعلامتی Suffix_درخت_اندرونی_گرہ::get_انتہا_Idx() ثابت { واپس انتہا_Idx; }
خالی Suffix_درخت_اندرونی_گرہ::سیٹ_Link(Suffix_درخت_اندرونی_گرہ *L) {
  أكد(L && "Cannot set a null link?");
  Link = L;
}
Suffix_درخت_اندرونی_گرہ *Suffix_درخت_اندرونی_گرہ::getLink() ثابت { واپس Link; }

غیرعلامتی Suffix_درخت_برگ_گرہ::get_انتہا_Idx() ثابت {
  أكد(انتہا_Idx && "EndIdx is empty?");
  واپس *انتہا_Idx;
}

غیرعلامتی Suffix_درخت_برگ_گرہ::getSuffixIdx() ثابت { واپس SuffixIdx; }
خالی Suffix_درخت_برگ_گرہ::سیٹ_Suffix_Idx(غیرعلامتی Idx) { SuffixIdx = Idx; }

غیرعلامتی Suffix_درخت_گرہ::get_بایاں_برگ_Idx() ثابت { واپس بایاں_برگ_Idx; }
غیرعلامتی Suffix_درخت_گرہ::get_دایاں_برگ_Idx() ثابت { واپس دایاں_برگ_Idx; }
خالی Suffix_درخت_گرہ::سیٹ_بایاں_برگ_Idx(غیرعلامتی Idx) { بایاں_برگ_Idx = Idx; }
خالی Suffix_درخت_گرہ::سیٹ_دایاں_برگ_Idx(غیرعلامتی Idx) { دایاں_برگ_Idx = Idx; }
