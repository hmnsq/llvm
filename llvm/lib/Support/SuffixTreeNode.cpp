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

unsigned SuffixTreeNode::getStartIdx() const { ត្រឡប់ StartIdx; }
void SuffixTreeNode::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
void SuffixTreeNode::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned SuffixTreeNode::getConcatLen() const { ត្រឡប់ ConcatLen; }

bool SuffixTreeInternalNode::isRoot() const {
  ត្រឡប់ getStartIdx() == EmptyIdx;
}
unsigned SuffixTreeInternalNode::getEndIdx() const { ត្រឡប់ EndIdx; }
void SuffixTreeInternalNode::setLink(SuffixTreeInternalNode *L) {
  assert(L && "Cannot set a null link?");
  Link = L;
}
SuffixTreeInternalNode *SuffixTreeInternalNode::getLink() const { ត្រឡប់ Link; }

unsigned SuffixTreeLeafNode::getEndIdx() const {
  assert(EndIdx && "EndIdx is empty?");
  ត្រឡប់ *EndIdx;
}

unsigned SuffixTreeLeafNode::getSuffixIdx() const { ត្រឡប់ SuffixIdx; }
void SuffixTreeLeafNode::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned SuffixTreeNode::getLeftLeafIdx() const { ត្រឡប់ LeftLeafIdx; }
unsigned SuffixTreeNode::getRightLeafIdx() const { ត្រឡប់ RightLeafIdx; }
void SuffixTreeNode::setLeftLeafIdx(unsigned Idx) { LeftLeafIdx = Idx; }
void SuffixTreeNode::setRightLeafIdx(unsigned Idx) { RightLeafIdx = Idx; }
