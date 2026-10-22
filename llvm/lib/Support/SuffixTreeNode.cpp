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

#include "llvm/Support/SuffixTreeNode.h"

using namespace llvm;

unsigned Suffix_पुनो_बुहोल::getStartIdx() const { return StartIdx; }
void Suffix_पुनो_बुहोल::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
void Suffix_पुनो_बुहोल::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned Suffix_पुनो_बुहोल::getConcatLen() const { return ConcatLen; }

bool Suffix_पुनो_Internal_बुहोल::is_उगात() const {
  return getStartIdx() == EmptyIdx;
}
unsigned Suffix_पुनो_Internal_बुहोल::getEndIdx() const { return EndIdx; }
void Suffix_पुनो_Internal_बुहोल::setLink(Suffix_पुनो_Internal_बुहोल *L) {
  assert(L && "Cannot set a null link?");
  Link = L;
}
Suffix_पुनो_Internal_बुहोल *Suffix_पुनो_Internal_बुहोल::getLink() const { return Link; }

unsigned Suffix_पुनो_Leaf_बुहोल::getEndIdx() const {
  assert(EndIdx && "EndIdx is empty?");
  return *EndIdx;
}

unsigned Suffix_पुनो_Leaf_बुहोल::getSuffixIdx() const { return SuffixIdx; }
void Suffix_पुनो_Leaf_बुहोल::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned Suffix_पुनो_बुहोल::get_कालिवा_Leaf_Idx() const { return कालिवा_Leaf_Idx; }
unsigned Suffix_पुनो_बुहोल::get_कानान_Leaf_Idx() const { return कानान_Leaf_Idx; }
void Suffix_पुनो_बुहोल::set_कालिवा_Leaf_Idx(unsigned Idx) { कालिवा_Leaf_Idx = Idx; }
void Suffix_पुनो_बुहोल::set_कानान_Leaf_Idx(unsigned Idx) { कानान_Leaf_Idx = Idx; }
