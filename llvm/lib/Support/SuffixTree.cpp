//===- llvm/Support/SuffixTree.cpp - Implement Suffix Tree ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Suffix Tree class.
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/SuffixTree.h"
#ضمن "llvm/Support/Allocator.h"
#ضمن "llvm/Support/Casting.h"
#ضمن "llvm/Support/SuffixTreeNode.h"

using نطاق llvm;

/// \returns the number of elements in the substring associated with \p N.
ساكن size_t numElementsInSubstring(ثابت SuffixTreeNode *N) {
  أكد(N && "Got a null node?");
  אם (auto *Internal = dyn_cast<SuffixTreeInternalNode>(N))
    אם (Internal->isRoot())
      החזר 0;
  החזר N->getEndIdx() - N->getStartIdx() + 1;
}

SuffixTree::SuffixTree(ثابت ArrayRef<unsigned> &Str,
                       منطقي OutlinerLeafDescendants)
    : Str(Str), OutlinerLeafDescendants(OutlinerLeafDescendants) {
  Root = insertRoot();
  Active.Node = Root;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  unsigned SuffixesToAdd = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  עבור (unsigned PfxEndIdx = 0, End = Str.size(); PfxEndIdx < End; PfxEndIdx++) {
    SuffixesToAdd++;
    LeafEndIdx = PfxEndIdx; // Extend each of the leaves.
    SuffixesToAdd = extend(PfxEndIdx, SuffixesToAdd);
  }

  // Set the suffix indices of each leaf.
  أكد(Root && "Root node can't be nullptr!");
  setSuffixIndices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  אם (OutlinerLeafDescendants)
    setLeafNodes();
}

SuffixTreeNode *SuffixTree::insertLeaf(SuffixTreeInternalNode &Parent,
                                       unsigned StartIdx, unsigned Edge) {
  أكد(StartIdx <= LeafEndIdx && "String can't start after it ends!");
  auto *N = جديد (LeafNodeAllocator.Allocate())
      SuffixTreeLeafNode(StartIdx, &LeafEndIdx);
  Parent.Children[Edge] = N;
  החזר N;
}

SuffixTreeInternalNode *
SuffixTree::insertInternalNode(SuffixTreeInternalNode *Parent,
                               unsigned StartIdx, unsigned EndIdx,
                               unsigned Edge) {
  أكد(StartIdx <= EndIdx && "String can't start after it ends!");
  أكد(!(!Parent && StartIdx != SuffixTreeNode::EmptyIdx) &&
         "Non-root internal nodes must have parents!");
  auto *N = جديد (InternalNodeAllocator.Allocate())
      SuffixTreeInternalNode(StartIdx, EndIdx, Root);
  אם (Parent)
    Parent->Children[Edge] = N;
  החזר N;
}

SuffixTreeInternalNode *SuffixTree::insertRoot() {
  החזר insertInternalNode(/*Parent = */ nullptr, SuffixTreeNode::EmptyIdx,
                            SuffixTreeNode::EmptyIdx, /*Edge = */ 0);
}

فراغ SuffixTree::setSuffixIndices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  SmallVector<std::pair<SuffixTreeNode *, unsigned>> ToVisit;

  // Current node being visited.
  SuffixTreeNode *CurrNode = Root;

  // Sum of the lengths of the nodes down the path to the current one.
  unsigned CurrNodeLen = 0;
  ToVisit.push_back({CurrNode, CurrNodeLen});
  כלעוד (!ToVisit.empty()) {
    std::tie(CurrNode, CurrNodeLen) = ToVisit.pop_back_val();
    // Length of the current node from the root down to here.
    CurrNode->setConcatLen(CurrNodeLen);
    אם (auto *InternalNode = dyn_cast<SuffixTreeInternalNode>(CurrNode))
      עבור (auto &ChildPair : InternalNode->Children) {
        أكد(ChildPair.second && "Node had a null child!");
        ToVisit.push_back(
            {ChildPair.second,
             CurrNodeLen + numElementsInSubstring(ChildPair.second)});
      }
    // No children, so we are at the end of the string.
    אם (auto *LeafNode = dyn_cast<SuffixTreeLeafNode>(CurrNode))
      LeafNode->setSuffixIdx(Str.size() - CurrNodeLen);
  }
}

فراغ SuffixTree::setLeafNodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  SmallVector<SuffixTreeNode *> ToVisit;
  ToVisit.push_back(Root);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  unsigned LeafCounter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<SuffixTreeInternalNode *,
           std::pair<SuffixTreeNode *, SuffixTreeNode *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  כלעוד (!ToVisit.empty()) {
    SuffixTreeNode *CurrNode = ToVisit.pop_back_val();
    אם (auto *CurrInternalNode = dyn_cast<SuffixTreeInternalNode>(CurrNode)) {
      // The current node is an internal node.
      auto I = ChildrenMap.find(CurrInternalNode);
      אם (I == ChildrenMap.end()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        auto J = CurrInternalNode->Children.begin();
        אם (J != CurrInternalNode->Children.end()) {
          ToVisit.push_back(CurrNode);
          SuffixTreeNode *FirstChild = J->second;
          SuffixTreeNode *LastChild = nullptr;
          עבור (; J != CurrInternalNode->Children.end(); ++J) {
            LastChild = J->second;
            ToVisit.push_back(LastChild);
          }
          ChildrenMap[CurrInternalNode] = {FirstChild, LastChild};
        }
      } אחרת {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        auto [FirstChild, LastChild] = I->second;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        CurrNode->setRightLeafIdx(FirstChild->getRightLeafIdx());
        // Get the last child to use its LeftLeafIdx.
        CurrNode->setLeftLeafIdx(LastChild->getLeftLeafIdx());
        أكد(CurrNode->getLeftLeafIdx() <= CurrNode->getRightLeafIdx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } אחרת {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      CurrNode->setLeftLeafIdx(LeafCounter);
      CurrNode->setRightLeafIdx(LeafCounter);
      ++LeafCounter;
      auto *CurrLeafNode = cast<SuffixTreeLeafNode>(CurrNode);
      LeafNodes.push_back(CurrLeafNode);
    }
  }
}

unsigned SuffixTree::extend(unsigned EndIdx, unsigned SuffixesToAdd) {
  SuffixTreeInternalNode *NeedsLink = nullptr;

  כלעוד (SuffixesToAdd > 0) {

    // Are we waiting to add anything other than just the last character?
    אם (Active.Len == 0) {
      // If not, then say the active index is the end index.
      Active.Idx = EndIdx;
    }

    أكد(Active.Idx <= EndIdx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    unsigned FirstChar = Str[Active.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    אם (auto It = Active.Node->Children.find(FirstChar);
        It == Active.Node->Children.end()) {
      // If not, then we can just insert a leaf and move to the next step.
      insertLeaf(*Active.Node, EndIdx, FirstChar);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      אם (NeedsLink) {
        NeedsLink->setLink(Active.Node);
        NeedsLink = nullptr;
      }
    } אחרת {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      SuffixTreeNode *NextNode = It->second;

      unsigned SubstringLen = numElementsInSubstring(NextNode);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      אם (Active.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        أكد(isa<SuffixTreeInternalNode>(NextNode) &&
               "Expected an internal node?");
        Active.Idx += SubstringLen;
        Active.Len -= SubstringLen;
        Active.Node = cast<SuffixTreeInternalNode>(NextNode);
        استمر;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      unsigned LastChar = Str[EndIdx];

      // Is the string we're trying to insert a substring of the next node?
      אם (Str[NextNode->getStartIdx() + Active.Len] == LastChar) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        אם (NeedsLink && !Active.Node->isRoot()) {
          NeedsLink->setLink(Active.Node);
          NeedsLink = nullptr;
        }

        Active.Len++;
        اكسر;
      }

      // The string we're trying to insert isn't a substring of the next node,
      // but matches up to a point. Split the node.
      //
      // For example, say we ended our search at a node n and we're trying to
      // insert ABD. Then we'll create a new node s for AB, reduce n to just
      // representing C, and insert a new leaf node l to represent d. This
      // allows us to ensure that if n was a leaf, it remains a leaf.
      //
      //   | ABC  ---split--->  | AB
      //   n                    s
      //                     C / \ D
      //                      n   l

      // The node s from the diagram
      SuffixTreeInternalNode *SplitNode = insertInternalNode(
          Active.Node, NextNode->getStartIdx(),
          NextNode->getStartIdx() + Active.Len - 1, FirstChar);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      insertLeaf(*SplitNode, EndIdx, LastChar);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      NextNode->incrementStartIdx(Active.Len);
      SplitNode->Children[Str[NextNode->getStartIdx()]] = NextNode;

      // SplitNode is an internal node, update the suffix link.
      אם (NeedsLink)
        NeedsLink->setLink(SplitNode);

      NeedsLink = SplitNode;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    SuffixesToAdd--;

    אם (Active.Node->isRoot()) {
      אם (Active.Len > 0) {
        Active.Len--;
        Active.Idx = EndIdx - SuffixesToAdd + 1;
      }
    } אחרת {
      // Start the next phase at the next smallest suffix.
      Active.Node = Active.Node->getLink();
    }
  }

  החזר SuffixesToAdd;
}

فراغ SuffixTree::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = nullptr;

  // Each leaf node represents a repeat of a string.
  SmallVector<unsigned> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  כלעוד (!InternalNodesToVisit.empty()) {
    RepeatedSubstringStarts.clear();
    auto *Curr = InternalNodesToVisit.pop_back_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    unsigned Length = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    עבור (auto &ChildPair : Curr->Children)
      // Save all of this node's children for processing.
      אם (auto *InternalChild =
              dyn_cast<SuffixTreeInternalNode>(ChildPair.second))
        InternalNodesToVisit.push_back(InternalChild);

    // If length of repeated substring is below threshold, then skip it.
    אם (Length < MinLength)
      استمر;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    אם (Curr->isRoot())
      استمر;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    אם (OutlinerLeafDescendants) {
      עבור (unsigned I = Curr->getLeftLeafIdx(); I <= Curr->getRightLeafIdx();
           ++I)
        RepeatedSubstringStarts.push_back(LeafNodes[I]->getSuffixIdx());
    } אחרת {
      עבור (auto &ChildPair : Curr->Children)
        אם (auto *Leaf = dyn_cast<SuffixTreeLeafNode>(ChildPair.second))
          RepeatedSubstringStarts.push_back(Leaf->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    אם (RepeatedSubstringStarts.size() < 2)
      استمر;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.Length = Length;
    llvm::append_range(RS.StartIndices, RepeatedSubstringStarts);
    اكسر;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
