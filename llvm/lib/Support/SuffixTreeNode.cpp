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

using نطاق llvm;

unsigned SuffixTreeNode::getStartIdx() ثابت { החזר StartIdx; }
فراغ SuffixTreeNode::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
فراغ SuffixTreeNode::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned SuffixTreeNode::getConcatLen() ثابت { החזר ConcatLen; }

منطقي SuffixTreeInternalNode::isRoot() ثابت {
  החזר getStartIdx() == EmptyIdx;
}
unsigned SuffixTreeInternalNode::getEndIdx() ثابت { החזר EndIdx; }
فراغ SuffixTreeInternalNode::setLink(SuffixTreeInternalNode *L) {
  أكد(L && "Cannot set a null link?");
  Link = L;
}
SuffixTreeInternalNode *SuffixTreeInternalNode::getLink() ثابت { החזר Link; }

unsigned SuffixTreeLeafNode::getEndIdx() ثابت {
  أكد(EndIdx && "EndIdx is empty?");
  החזר *EndIdx;
}

unsigned SuffixTreeLeafNode::getSuffixIdx() ثابت { החזר SuffixIdx; }
فراغ SuffixTreeLeafNode::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned SuffixTreeNode::getLeftLeafIdx() ثابت { החזר LeftLeafIdx; }
unsigned SuffixTreeNode::getRightLeafIdx() ثابت { החזר RightLeafIdx; }
فراغ SuffixTreeNode::setLeftLeafIdx(unsigned Idx) { LeftLeafIdx = Idx; }
فراغ SuffixTreeNode::setRightLeafIdx(unsigned Idx) { RightLeafIdx = Idx; }
