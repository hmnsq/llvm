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
ساكن size_t اڠک_Elements_In_Substring(ثابت Suffix_ڤوکوق_نود *N) {
  أكد(N && "Got a null node?");
  إذا (auto *Internal = dyn_cast<Suffix_ڤوکوق_Internal_نود>(N))
    إذا (Internal->is_اکر())
      أرجع 0;
  أرجع N->getEndIdx() - N->getStartIdx() + 1;
}

Suffix_ڤوکوق::Suffix_ڤوکوق(ثابت ArrayRef<unsigned> &Str,
                       منطقي OutlinerLeafDescendants)
    : Str(Str), OutlinerLeafDescendants(OutlinerLeafDescendants) {
  اکر_2 = سيسيڤ_اکر();
  Active.نود = اکر_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  unsigned SuffixesToAdd = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  لكل (unsigned PfxEndIdx = 0, End = Str.size(); PfxEndIdx < End; PfxEndIdx++) {
    SuffixesToAdd++;
    LeafEndIdx = PfxEndIdx; // Extend each of the leaves.
    SuffixesToAdd = extend(PfxEndIdx, SuffixesToAdd);
  }

  // Set the suffix indices of each leaf.
  أكد(اکر_2 && "Root node can't be nullptr!");
  setSuffixIndices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  إذا (OutlinerLeafDescendants)
    setLeafNodes();
}

Suffix_ڤوکوق_نود *Suffix_ڤوکوق::سيسيڤ_Leaf(Suffix_ڤوکوق_Internal_نود &Parent,
                                       unsigned StartIdx, unsigned Edge) {
  أكد(StartIdx <= LeafEndIdx && "String can't start after it ends!");
  auto *N = جديد (Leaf_نود_Allocator.Allocate())
      Suffix_ڤوکوق_Leaf_نود(StartIdx, &LeafEndIdx);
  Parent.Children[Edge] = N;
  أرجع N;
}

Suffix_ڤوکوق_Internal_نود *
Suffix_ڤوکوق::سيسيڤ_Internal_نود(Suffix_ڤوکوق_Internal_نود *Parent,
                               unsigned StartIdx, unsigned EndIdx,
                               unsigned Edge) {
  أكد(StartIdx <= EndIdx && "String can't start after it ends!");
  أكد(!(!Parent && StartIdx != Suffix_ڤوکوق_نود::EmptyIdx) &&
         "Non-root internal nodes must have parents!");
  auto *N = جديد (Internal_نود_Allocator.Allocate())
      Suffix_ڤوکوق_Internal_نود(StartIdx, EndIdx, اکر_2);
  إذا (Parent)
    Parent->Children[Edge] = N;
  أرجع N;
}

Suffix_ڤوکوق_Internal_نود *Suffix_ڤوکوق::سيسيڤ_اکر() {
  أرجع سيسيڤ_Internal_نود(/*Parent = */ nullptr, Suffix_ڤوکوق_نود::EmptyIdx,
                            Suffix_ڤوکوق_نود::EmptyIdx, /*Edge = */ 0);
}

فراغ Suffix_ڤوکوق::setSuffixIndices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  SmallVector<std::pair<Suffix_ڤوکوق_نود *, unsigned>> ToVisit;

  // Current node being visited.
  Suffix_ڤوکوق_نود *Curr_نود = اکر_2;

  // Sum of the lengths of the nodes down the path to the current one.
  unsigned Curr_نود_Len = 0;
  ToVisit.تولق_back({Curr_نود, Curr_نود_Len});
  طالما (!ToVisit.empty()) {
    std::tie(Curr_نود, Curr_نود_Len) = ToVisit.کلوار_back_val();
    // Length of the current node from the root down to here.
    Curr_نود->setConcatLen(Curr_نود_Len);
    إذا (auto *Internal_نود = dyn_cast<Suffix_ڤوکوق_Internal_نود>(Curr_نود))
      لكل (auto &ChildPair : Internal_نود->Children) {
        أكد(ChildPair.second && "Node had a null child!");
        ToVisit.تولق_back(
            {ChildPair.second,
             Curr_نود_Len + اڠک_Elements_In_Substring(ChildPair.second)});
      }
    // No children, so we are at the end of the string.
    إذا (auto *Leaf_نود = dyn_cast<Suffix_ڤوکوق_Leaf_نود>(Curr_نود))
      Leaf_نود->setSuffixIdx(Str.size() - Curr_نود_Len);
  }
}

فراغ Suffix_ڤوکوق::setLeafNodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  SmallVector<Suffix_ڤوکوق_نود *> ToVisit;
  ToVisit.تولق_back(اکر_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  unsigned LeafCounter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_ڤوکوق_Internal_نود *,
           std::pair<Suffix_ڤوکوق_نود *, Suffix_ڤوکوق_نود *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  طالما (!ToVisit.empty()) {
    Suffix_ڤوکوق_نود *Curr_نود = ToVisit.کلوار_back_val();
    إذا (auto *Curr_Internal_نود = dyn_cast<Suffix_ڤوکوق_Internal_نود>(Curr_نود)) {
      // The current node is an internal node.
      auto I = ChildrenMap.find(Curr_Internal_نود);
      إذا (I == ChildrenMap.end()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        auto J = Curr_Internal_نود->Children.begin();
        إذا (J != Curr_Internal_نود->Children.end()) {
          ToVisit.تولق_back(Curr_نود);
          Suffix_ڤوکوق_نود *FirstChild = J->second;
          Suffix_ڤوکوق_نود *LastChild = nullptr;
          لكل (; J != Curr_Internal_نود->Children.end(); ++J) {
            LastChild = J->second;
            ToVisit.تولق_back(LastChild);
          }
          ChildrenMap[Curr_Internal_نود] = {FirstChild, LastChild};
        }
      } وإلا {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        auto [FirstChild, LastChild] = I->second;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_نود->set_کانن_Leaf_Idx(FirstChild->get_کانن_Leaf_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_نود->set_کيري_Leaf_Idx(LastChild->get_کيري_Leaf_Idx());
        أكد(Curr_نود->get_کيري_Leaf_Idx() <= Curr_نود->get_کانن_Leaf_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } وإلا {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_نود->set_کيري_Leaf_Idx(LeafCounter);
      Curr_نود->set_کانن_Leaf_Idx(LeafCounter);
      ++LeafCounter;
      auto *Curr_Leaf_نود = cast<Suffix_ڤوکوق_Leaf_نود>(Curr_نود);
      LeafNodes.تولق_back(Curr_Leaf_نود);
    }
  }
}

unsigned Suffix_ڤوکوق::extend(unsigned EndIdx, unsigned SuffixesToAdd) {
  Suffix_ڤوکوق_Internal_نود *NeedsLink = nullptr;

  طالما (SuffixesToAdd > 0) {

    // Are we waiting to add anything other than just the last character?
    إذا (Active.Len == 0) {
      // If not, then say the active index is the end index.
      Active.Idx = EndIdx;
    }

    أكد(Active.Idx <= EndIdx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    unsigned FirstChar = Str[Active.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    إذا (auto It = Active.نود->Children.find(FirstChar);
        It == Active.نود->Children.end()) {
      // If not, then we can just insert a leaf and move to the next step.
      سيسيڤ_Leaf(*Active.نود, EndIdx, FirstChar);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      إذا (NeedsLink) {
        NeedsLink->setLink(Active.نود);
        NeedsLink = nullptr;
      }
    } وإلا {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_ڤوکوق_نود *Next_نود = It->second;

      unsigned SubstringLen = اڠک_Elements_In_Substring(Next_نود);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      إذا (Active.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        أكد(isa<Suffix_ڤوکوق_Internal_نود>(Next_نود) &&
               "Expected an internal node?");
        Active.Idx += SubstringLen;
        Active.Len -= SubstringLen;
        Active.نود = cast<Suffix_ڤوکوق_Internal_نود>(Next_نود);
        استمر;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      unsigned LastChar = Str[EndIdx];

      // Is the string we're trying to insert a substring of the next node?
      إذا (Str[Next_نود->getStartIdx() + Active.Len] == LastChar) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        إذا (NeedsLink && !Active.نود->is_اکر()) {
          NeedsLink->setLink(Active.نود);
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
      Suffix_ڤوکوق_Internal_نود *Split_نود = سيسيڤ_Internal_نود(
          Active.نود, Next_نود->getStartIdx(),
          Next_نود->getStartIdx() + Active.Len - 1, FirstChar);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      سيسيڤ_Leaf(*Split_نود, EndIdx, LastChar);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      Next_نود->incrementStartIdx(Active.Len);
      Split_نود->Children[Str[Next_نود->getStartIdx()]] = Next_نود;

      // SplitNode is an internal node, update the suffix link.
      إذا (NeedsLink)
        NeedsLink->setLink(Split_نود);

      NeedsLink = Split_نود;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    SuffixesToAdd--;

    إذا (Active.نود->is_اکر()) {
      إذا (Active.Len > 0) {
        Active.Len--;
        Active.Idx = EndIdx - SuffixesToAdd + 1;
      }
    } وإلا {
      // Start the next phase at the next smallest suffix.
      Active.نود = Active.نود->getLink();
    }
  }

  أرجع SuffixesToAdd;
}

فراغ Suffix_ڤوکوق::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = nullptr;

  // Each leaf node represents a repeat of a string.
  SmallVector<unsigned> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  طالما (!InternalNodesToVisit.empty()) {
    RepeatedSubstringStarts.clear();
    auto *Curr = InternalNodesToVisit.کلوار_back_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    unsigned Length = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    لكل (auto &ChildPair : Curr->Children)
      // Save all of this node's children for processing.
      إذا (auto *InternalChild =
              dyn_cast<Suffix_ڤوکوق_Internal_نود>(ChildPair.second))
        InternalNodesToVisit.تولق_back(InternalChild);

    // If length of repeated substring is below threshold, then skip it.
    إذا (Length < MinLength)
      استمر;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    إذا (Curr->is_اکر())
      استمر;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    إذا (OutlinerLeafDescendants) {
      لكل (unsigned I = Curr->get_کيري_Leaf_Idx(); I <= Curr->get_کانن_Leaf_Idx();
           ++I)
        RepeatedSubstringStarts.تولق_back(LeafNodes[I]->getSuffixIdx());
    } وإلا {
      لكل (auto &ChildPair : Curr->Children)
        إذا (auto *Leaf = dyn_cast<Suffix_ڤوکوق_Leaf_نود>(ChildPair.second))
          RepeatedSubstringStarts.تولق_back(Leaf->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    إذا (RepeatedSubstringStarts.size() < 2)
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
