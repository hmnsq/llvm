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

unsigned SuffixTreeNode::getStartIdx() const { ᨀᨘᨒ StartIdx; }
void SuffixTreeNode::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
void SuffixTreeNode::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned SuffixTreeNode::getConcatLen() const { ᨀᨘᨒ ConcatLen; }

bool SuffixTreeInternalNode::isRoot() const {
  ᨀᨘᨒ getStartIdx() == EmptyIdx;
}
unsigned SuffixTreeInternalNode::getEndIdx() const { ᨀᨘᨒ EndIdx; }
void SuffixTreeInternalNode::setLink(SuffixTreeInternalNode *L) {
  assert(L && "Cannot set a null link?");
  Link = L;
}
SuffixTreeInternalNode *SuffixTreeInternalNode::getLink() const { ᨀᨘᨒ Link; }

unsigned SuffixTreeLeafNode::getEndIdx() const {
  assert(EndIdx && "EndIdx is empty?");
  ᨀᨘᨒ *EndIdx;
}

unsigned SuffixTreeLeafNode::getSuffixIdx() const { ᨀᨘᨒ SuffixIdx; }
void SuffixTreeLeafNode::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned SuffixTreeNode::getLeftLeafIdx() const { ᨀᨘᨒ LeftLeafIdx; }
unsigned SuffixTreeNode::getRightLeafIdx() const { ᨀᨘᨒ RightLeafIdx; }
void SuffixTreeNode::setLeftLeafIdx(unsigned Idx) { LeftLeafIdx = Idx; }
void SuffixTreeNode::setRightLeafIdx(unsigned Idx) { RightLeafIdx = Idx; }
