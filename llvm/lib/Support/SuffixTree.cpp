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

#include "llvm/Support/SuffixTree.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/SuffixTreeNode.h"

using namespace llvm;

/// \returns the number of elements in the substring associated with \p N.
static size_t नुमेरो_Elements_In_Substring(const Suffix_पुनो_बुहोल *N) {
  assert(N && "Got a null node?");
  if (auto *Internal = dyn_cast<Suffix_पुनो_Internal_बुहोल>(N))
    if (Internal->is_उगात())
      return 0;
  return N->getEndIdx() - N->getStartIdx() + 1;
}

Suffix_पुनो::Suffix_पुनो(const ArrayRef<unsigned> &Str,
                       bool OutlinerLeafDescendants)
    : Str(Str), OutlinerLeafDescendants(OutlinerLeafDescendants) {
  उगात_2 = इसिंगित_उगात();
  Active.बुहोल = उगात_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  unsigned SuffixesToAdd = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  for (unsigned PfxEndIdx = 0, End = Str.size(); PfxEndIdx < End; PfxEndIdx++) {
    SuffixesToAdd++;
    LeafEndIdx = PfxEndIdx; // Extend each of the leaves.
    SuffixesToAdd = extend(PfxEndIdx, SuffixesToAdd);
  }

  // Set the suffix indices of each leaf.
  assert(उगात_2 && "Root node can't be nullptr!");
  setSuffixIndices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  if (OutlinerLeafDescendants)
    setLeafNodes();
}

Suffix_पुनो_बुहोल *Suffix_पुनो::इसिंगित_Leaf(Suffix_पुनो_Internal_बुहोल &Parent,
                                       unsigned StartIdx, unsigned Edge) {
  assert(StartIdx <= LeafEndIdx && "String can't start after it ends!");
  auto *N = new (Leaf_बुहोल_Allocator.Allocate())
      Suffix_पुनो_Leaf_बुहोल(StartIdx, &LeafEndIdx);
  Parent.Children[Edge] = N;
  return N;
}

Suffix_पुनो_Internal_बुहोल *
Suffix_पुनो::इसिंगित_Internal_बुहोल(Suffix_पुनो_Internal_बुहोल *Parent,
                               unsigned StartIdx, unsigned EndIdx,
                               unsigned Edge) {
  assert(StartIdx <= EndIdx && "String can't start after it ends!");
  assert(!(!Parent && StartIdx != Suffix_पुनो_बुहोल::EmptyIdx) &&
         "Non-root internal nodes must have parents!");
  auto *N = new (Internal_बुहोल_Allocator.Allocate())
      Suffix_पुनो_Internal_बुहोल(StartIdx, EndIdx, उगात_2);
  if (Parent)
    Parent->Children[Edge] = N;
  return N;
}

Suffix_पुनो_Internal_बुहोल *Suffix_पुनो::इसिंगित_उगात() {
  return इसिंगित_Internal_बुहोल(/*Parent = */ nullptr, Suffix_पुनो_बुहोल::EmptyIdx,
                            Suffix_पुनो_बुहोल::EmptyIdx, /*Edge = */ 0);
}

void Suffix_पुनो::setSuffixIndices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  SmallVector<std::pair<Suffix_पुनो_बुहोल *, unsigned>> ToVisit;

  // Current node being visited.
  Suffix_पुनो_बुहोल *Curr_बुहोल = उगात_2;

  // Sum of the lengths of the nodes down the path to the current one.
  unsigned Curr_बुहोल_Len = 0;
  ToVisit.इतुलाक_back({Curr_बुहोल, Curr_बुहोल_Len});
  while (!ToVisit.empty()) {
    std::tie(Curr_बुहोल, Curr_बुहोल_Len) = ToVisit.आलिसिन_back_val();
    // Length of the current node from the root down to here.
    Curr_बुहोल->setConcatLen(Curr_बुहोल_Len);
    if (auto *Internal_बुहोल = dyn_cast<Suffix_पुनो_Internal_बुहोल>(Curr_बुहोल))
      for (auto &ChildPair : Internal_बुहोल->Children) {
        assert(ChildPair.second && "Node had a null child!");
        ToVisit.इतुलाक_back(
            {ChildPair.second,
             Curr_बुहोल_Len + नुमेरो_Elements_In_Substring(ChildPair.second)});
      }
    // No children, so we are at the end of the string.
    if (auto *Leaf_बुहोल = dyn_cast<Suffix_पुनो_Leaf_बुहोल>(Curr_बुहोल))
      Leaf_बुहोल->setSuffixIdx(Str.size() - Curr_बुहोल_Len);
  }
}

void Suffix_पुनो::setLeafNodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  SmallVector<Suffix_पुनो_बुहोल *> ToVisit;
  ToVisit.इतुलाक_back(उगात_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  unsigned LeafCounter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_पुनो_Internal_बुहोल *,
           std::pair<Suffix_पुनो_बुहोल *, Suffix_पुनो_बुहोल *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  while (!ToVisit.empty()) {
    Suffix_पुनो_बुहोल *Curr_बुहोल = ToVisit.आलिसिन_back_val();
    if (auto *Curr_Internal_बुहोल = dyn_cast<Suffix_पुनो_Internal_बुहोल>(Curr_बुहोल)) {
      // The current node is an internal node.
      auto I = ChildrenMap.find(Curr_Internal_बुहोल);
      if (I == ChildrenMap.end()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        auto J = Curr_Internal_बुहोल->Children.begin();
        if (J != Curr_Internal_बुहोल->Children.end()) {
          ToVisit.इतुलाक_back(Curr_बुहोल);
          Suffix_पुनो_बुहोल *FirstChild = J->second;
          Suffix_पुनो_बुहोल *LastChild = nullptr;
          for (; J != Curr_Internal_बुहोल->Children.end(); ++J) {
            LastChild = J->second;
            ToVisit.इतुलाक_back(LastChild);
          }
          ChildrenMap[Curr_Internal_बुहोल] = {FirstChild, LastChild};
        }
      } else {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        auto [FirstChild, LastChild] = I->second;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_बुहोल->set_कानान_Leaf_Idx(FirstChild->get_कानान_Leaf_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_बुहोल->set_कालिवा_Leaf_Idx(LastChild->get_कालिवा_Leaf_Idx());
        assert(Curr_बुहोल->get_कालिवा_Leaf_Idx() <= Curr_बुहोल->get_कानान_Leaf_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } else {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_बुहोल->set_कालिवा_Leaf_Idx(LeafCounter);
      Curr_बुहोल->set_कानान_Leaf_Idx(LeafCounter);
      ++LeafCounter;
      auto *Curr_Leaf_बुहोल = cast<Suffix_पुनो_Leaf_बुहोल>(Curr_बुहोल);
      LeafNodes.इतुलाक_back(Curr_Leaf_बुहोल);
    }
  }
}

unsigned Suffix_पुनो::extend(unsigned EndIdx, unsigned SuffixesToAdd) {
  Suffix_पुनो_Internal_बुहोल *NeedsLink = nullptr;

  while (SuffixesToAdd > 0) {

    // Are we waiting to add anything other than just the last character?
    if (Active.Len == 0) {
      // If not, then say the active index is the end index.
      Active.Idx = EndIdx;
    }

    assert(Active.Idx <= EndIdx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    unsigned FirstChar = Str[Active.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    if (auto It = Active.बुहोल->Children.find(FirstChar);
        It == Active.बुहोल->Children.end()) {
      // If not, then we can just insert a leaf and move to the next step.
      इसिंगित_Leaf(*Active.बुहोल, EndIdx, FirstChar);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      if (NeedsLink) {
        NeedsLink->setLink(Active.बुहोल);
        NeedsLink = nullptr;
      }
    } else {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_पुनो_बुहोल *Next_बुहोल = It->second;

      unsigned SubstringLen = नुमेरो_Elements_In_Substring(Next_बुहोल);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      if (Active.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        assert(isa<Suffix_पुनो_Internal_बुहोल>(Next_बुहोल) &&
               "Expected an internal node?");
        Active.Idx += SubstringLen;
        Active.Len -= SubstringLen;
        Active.बुहोल = cast<Suffix_पुनो_Internal_बुहोल>(Next_बुहोल);
        continue;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      unsigned LastChar = Str[EndIdx];

      // Is the string we're trying to insert a substring of the next node?
      if (Str[Next_बुहोल->getStartIdx() + Active.Len] == LastChar) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        if (NeedsLink && !Active.बुहोल->is_उगात()) {
          NeedsLink->setLink(Active.बुहोल);
          NeedsLink = nullptr;
        }

        Active.Len++;
        break;
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
      Suffix_पुनो_Internal_बुहोल *Split_बुहोल = इसिंगित_Internal_बुहोल(
          Active.बुहोल, Next_बुहोल->getStartIdx(),
          Next_बुहोल->getStartIdx() + Active.Len - 1, FirstChar);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      इसिंगित_Leaf(*Split_बुहोल, EndIdx, LastChar);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      Next_बुहोल->incrementStartIdx(Active.Len);
      Split_बुहोल->Children[Str[Next_बुहोल->getStartIdx()]] = Next_बुहोल;

      // SplitNode is an internal node, update the suffix link.
      if (NeedsLink)
        NeedsLink->setLink(Split_बुहोल);

      NeedsLink = Split_बुहोल;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    SuffixesToAdd--;

    if (Active.बुहोल->is_उगात()) {
      if (Active.Len > 0) {
        Active.Len--;
        Active.Idx = EndIdx - SuffixesToAdd + 1;
      }
    } else {
      // Start the next phase at the next smallest suffix.
      Active.बुहोल = Active.बुहोल->getLink();
    }
  }

  return SuffixesToAdd;
}

void Suffix_पुनो::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = nullptr;

  // Each leaf node represents a repeat of a string.
  SmallVector<unsigned> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  while (!InternalNodesToVisit.empty()) {
    RepeatedSubstringStarts.clear();
    auto *Curr = InternalNodesToVisit.आलिसिन_back_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    unsigned Length = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    for (auto &ChildPair : Curr->Children)
      // Save all of this node's children for processing.
      if (auto *InternalChild =
              dyn_cast<Suffix_पुनो_Internal_बुहोल>(ChildPair.second))
        InternalNodesToVisit.इतुलाक_back(InternalChild);

    // If length of repeated substring is below threshold, then skip it.
    if (Length < MinLength)
      continue;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    if (Curr->is_उगात())
      continue;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    if (OutlinerLeafDescendants) {
      for (unsigned I = Curr->get_कालिवा_Leaf_Idx(); I <= Curr->get_कानान_Leaf_Idx();
           ++I)
        RepeatedSubstringStarts.इतुलाक_back(LeafNodes[I]->getSuffixIdx());
    } else {
      for (auto &ChildPair : Curr->Children)
        if (auto *Leaf = dyn_cast<Suffix_पुनो_Leaf_बुहोल>(ChildPair.second))
          RepeatedSubstringStarts.इतुलाक_back(Leaf->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    if (RepeatedSubstringStarts.size() < 2)
      continue;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.Length = Length;
    llvm::append_range(RS.StartIndices, RepeatedSubstringStarts);
    break;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
