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

unsigned SuffixTreeNode::getStartIdx() const { 戻る StartIdx; }
void SuffixTreeNode::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
void SuffixTreeNode::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned SuffixTreeNode::getConcatLen() const { 戻る ConcatLen; }

bool SuffixTreeInternalNode::isRoot() const {
  戻る getStartIdx() == EmptyIdx;
}
unsigned SuffixTreeInternalNode::getEndIdx() const { 戻る EndIdx; }
void SuffixTreeInternalNode::setLink(SuffixTreeInternalNode *L) {
  assert(L && "Cannot set a null link?");
  Link = L;
}
SuffixTreeInternalNode *SuffixTreeInternalNode::getLink() const { 戻る Link; }

unsigned SuffixTreeLeafNode::getEndIdx() const {
  assert(EndIdx && "EndIdx is empty?");
  戻る *EndIdx;
}

unsigned SuffixTreeLeafNode::getSuffixIdx() const { 戻る SuffixIdx; }
void SuffixTreeLeafNode::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned SuffixTreeNode::getLeftLeafIdx() const { 戻る LeftLeafIdx; }
unsigned SuffixTreeNode::getRightLeafIdx() const { 戻る RightLeafIdx; }
void SuffixTreeNode::setLeftLeafIdx(unsigned Idx) { LeftLeafIdx = Idx; }
void SuffixTreeNode::setRightLeafIdx(unsigned Idx) { RightLeafIdx = Idx; }
