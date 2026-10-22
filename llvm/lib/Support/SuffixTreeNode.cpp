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

unsigned SuffixTreeNode::getStartIdx() ثابت { أرجع StartIdx; }
فراغ SuffixTreeNode::incrementStartIdx(unsigned Inc) { StartIdx += Inc; }
فراغ SuffixTreeNode::setConcatLen(unsigned Len) { ConcatLen = Len; }
unsigned SuffixTreeNode::getConcatLen() ثابت { أرجع ConcatLen; }

منطقي SuffixTreeInternalNode::isRoot() ثابت {
  أرجع getStartIdx() == EmptyIdx;
}
unsigned SuffixTreeInternalNode::getEndIdx() ثابت { أرجع EndIdx; }
فراغ SuffixTreeInternalNode::setLink(SuffixTreeInternalNode *L) {
  أكد(L && "Cannot set a null link?");
  Link = L;
}
SuffixTreeInternalNode *SuffixTreeInternalNode::getLink() ثابت { أرجع Link; }

unsigned SuffixTreeLeafNode::getEndIdx() ثابت {
  أكد(EndIdx && "EndIdx is empty?");
  أرجع *EndIdx;
}

unsigned SuffixTreeLeafNode::getSuffixIdx() ثابت { أرجع SuffixIdx; }
فراغ SuffixTreeLeafNode::setSuffixIdx(unsigned Idx) { SuffixIdx = Idx; }

unsigned SuffixTreeNode::getLeftLeafIdx() ثابت { أرجع LeftLeafIdx; }
unsigned SuffixTreeNode::getRightLeafIdx() ثابت { أرجع RightLeafIdx; }
فراغ SuffixTreeNode::setLeftLeafIdx(unsigned Idx) { LeftLeafIdx = Idx; }
فراغ SuffixTreeNode::setRightLeafIdx(unsigned Idx) { RightLeafIdx = Idx; }
