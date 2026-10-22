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

#અન્તર્ભાવ "llvm/Support/SuffixTreeNode.h"

using નામાકાશ llvm;

unsigned Suffix_વૃક્ષ_ગાંઠ::getStartIdx() અચલ { પાછા StartIdx; }
શૂન્ય Suffix_વૃક્ષ_ગાંઠ::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
શૂન્ય Suffix_વૃક્ષ_ગાંઠ::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned Suffix_વૃક્ષ_ગાંઠ::getConcatLen() અચલ { પાછા ConcatLen; }

તાર્કિક Suffix_વૃક્ષ_Internal_ગાંઠ::is_મૂળ() અચલ {
  પાછા getStartIdx() == EmptyIdx;
}
unsigned Suffix_વૃક્ષ_Internal_ગાંઠ::getEndIdx() અચલ { પાછા EndIdx; }
શૂન્ય Suffix_વૃક્ષ_Internal_ગાંઠ::setLink(Suffix_વૃક્ષ_Internal_ગાંઠ *L) {
  પ્રતિજ્ઞા(L && "Cannot set a null link?");
  Link = L;
}
Suffix_વૃક્ષ_Internal_ગાંઠ *Suffix_વૃક્ષ_Internal_ગાંઠ::getLink() અચલ { પાછા Link; }

unsigned Suffix_વૃક્ષ_Leaf_ગાંઠ::getEndIdx() અચલ {
  પ્રતિજ્ઞા(EndIdx && "EndIdx is empty?");
  પાછા *EndIdx;
}

unsigned Suffix_વૃક્ષ_Leaf_ગાંઠ::getSuffixIdx() અચલ { પાછા SuffixIdx; }
શૂન્ય Suffix_વૃક્ષ_Leaf_ગાંઠ::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned Suffix_વૃક્ષ_ગાંઠ::get_ડાબું_Leaf_Idx() અચલ { પાછા ડાબું_Leaf_Idx; }
unsigned Suffix_વૃક્ષ_ગાંઠ::get_જમણું_Leaf_Idx() અચલ { પાછા જમણું_Leaf_Idx; }
શૂન્ય Suffix_વૃક્ષ_ગાંઠ::set_ડાબું_Leaf_Idx(unsigned Idx) { ડાબું_Leaf_Idx = Idx; }
શૂન્ય Suffix_વૃક્ષ_ગાંઠ::set_જમણું_Leaf_Idx(unsigned Idx) { જમણું_Leaf_Idx = Idx; }
