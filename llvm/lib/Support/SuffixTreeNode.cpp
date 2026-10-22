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

#அந்தர்பாவ "llvm/Support/SuffixTreeNode.h"

using நாமாகாஶ llvm;

unsigned Suffix_மரம்_முனை::getStartIdx() மாறிலி { திரும்பு StartIdx; }
வெறுமை Suffix_மரம்_முனை::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
வெறுமை Suffix_மரம்_முனை::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned Suffix_மரம்_முனை::getConcatLen() மாறிலி { திரும்பு ConcatLen; }

தருக்க Suffix_மரம்_Internal_முனை::is_வேர்() மாறிலி {
  திரும்பு getStartIdx() == EmptyIdx;
}
unsigned Suffix_மரம்_Internal_முனை::getEndIdx() மாறிலி { திரும்பு EndIdx; }
வெறுமை Suffix_மரம்_Internal_முனை::setLink(Suffix_மரம்_Internal_முனை *L) {
  ப்ரதிஜ்ஞா(L && "Cannot set a null link?");
  Link = L;
}
Suffix_மரம்_Internal_முனை *Suffix_மரம்_Internal_முனை::getLink() மாறிலி { திரும்பு Link; }

unsigned Suffix_மரம்_Leaf_முனை::getEndIdx() மாறிலி {
  ப்ரதிஜ்ஞா(EndIdx && "EndIdx is empty?");
  திரும்பு *EndIdx;
}

unsigned Suffix_மரம்_Leaf_முனை::getSuffixIdx() மாறிலி { திரும்பு SuffixIdx; }
வெறுமை Suffix_மரம்_Leaf_முனை::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned Suffix_மரம்_முனை::get_இடது_Leaf_Idx() மாறிலி { திரும்பு இடது_Leaf_Idx; }
unsigned Suffix_மரம்_முனை::get_வலது_Leaf_Idx() மாறிலி { திரும்பு வலது_Leaf_Idx; }
வெறுமை Suffix_மரம்_முனை::set_இடது_Leaf_Idx(unsigned Idx) { இடது_Leaf_Idx = Idx; }
வெறுமை Suffix_மரம்_முனை::set_வலது_Leaf_Idx(unsigned Idx) { வலது_Leaf_Idx = Idx; }
