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
ساكن size_t عدد_Elements_In_Substring(ثابت Suffix_درخت_گرہ *N) {
  أكد(N && "Got a null node?");
  إذا (auto *Internal = dyn_cast<Suffix_درخت_Internal_گرہ>(N))
    إذا (Internal->is_جڑ())
      أرجع 0;
  أرجع N->getEndIdx() - N->getStartIdx() + 1;
}

Suffix_درخت::Suffix_درخت(ثابت ArrayRef<unsigned> &Str,
                       منطقي OutlinerLeafDescendants)
    : Str(Str), OutlinerLeafDescendants(OutlinerLeafDescendants) {
  جڑ_2 = داخل_کرو_جڑ();
  Active.گرہ = جڑ_2;

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
  أكد(جڑ_2 && "Root node can't be nullptr!");
  setSuffixIndices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  إذا (OutlinerLeafDescendants)
    setLeafNodes();
}

Suffix_درخت_گرہ *Suffix_درخت::داخل_کرو_Leaf(Suffix_درخت_Internal_گرہ &Parent,
                                       unsigned StartIdx, unsigned Edge) {
  أكد(StartIdx <= LeafEndIdx && "String can't start after it ends!");
  auto *N = جديد (Leaf_گرہ_Allocator.Allocate())
      Suffix_درخت_Leaf_گرہ(StartIdx, &LeafEndIdx);
  Parent.Children[Edge] = N;
  أرجع N;
}

Suffix_درخت_Internal_گرہ *
Suffix_درخت::داخل_کرو_Internal_گرہ(Suffix_درخت_Internal_گرہ *Parent,
                               unsigned StartIdx, unsigned EndIdx,
                               unsigned Edge) {
  أكد(StartIdx <= EndIdx && "String can't start after it ends!");
  أكد(!(!Parent && StartIdx != Suffix_درخت_گرہ::EmptyIdx) &&
         "Non-root internal nodes must have parents!");
  auto *N = جديد (Internal_گرہ_Allocator.Allocate())
      Suffix_درخت_Internal_گرہ(StartIdx, EndIdx, جڑ_2);
  إذا (Parent)
    Parent->Children[Edge] = N;
  أرجع N;
}

Suffix_درخت_Internal_گرہ *Suffix_درخت::داخل_کرو_جڑ() {
  أرجع داخل_کرو_Internal_گرہ(/*Parent = */ nullptr, Suffix_درخت_گرہ::EmptyIdx,
                            Suffix_درخت_گرہ::EmptyIdx, /*Edge = */ 0);
}

فراغ Suffix_درخت::setSuffixIndices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  SmallVector<std::pair<Suffix_درخت_گرہ *, unsigned>> ToVisit;

  // Current node being visited.
  Suffix_درخت_گرہ *Curr_گرہ = جڑ_2;

  // Sum of the lengths of the nodes down the path to the current one.
  unsigned Curr_گرہ_Len = 0;
  ToVisit.ڈالو_back({Curr_گرہ, Curr_گرہ_Len});
  طالما (!ToVisit.empty()) {
    std::tie(Curr_گرہ, Curr_گرہ_Len) = ToVisit.نکالو_back_val();
    // Length of the current node from the root down to here.
    Curr_گرہ->setConcatLen(Curr_گرہ_Len);
    إذا (auto *Internal_گرہ = dyn_cast<Suffix_درخت_Internal_گرہ>(Curr_گرہ))
      لكل (auto &ChildPair : Internal_گرہ->Children) {
        أكد(ChildPair.second && "Node had a null child!");
        ToVisit.ڈالو_back(
            {ChildPair.second,
             Curr_گرہ_Len + عدد_Elements_In_Substring(ChildPair.second)});
      }
    // No children, so we are at the end of the string.
    إذا (auto *Leaf_گرہ = dyn_cast<Suffix_درخت_Leaf_گرہ>(Curr_گرہ))
      Leaf_گرہ->setSuffixIdx(Str.size() - Curr_گرہ_Len);
  }
}

فراغ Suffix_درخت::setLeafNodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  SmallVector<Suffix_درخت_گرہ *> ToVisit;
  ToVisit.ڈالو_back(جڑ_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  unsigned LeafCounter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_درخت_Internal_گرہ *,
           std::pair<Suffix_درخت_گرہ *, Suffix_درخت_گرہ *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  طالما (!ToVisit.empty()) {
    Suffix_درخت_گرہ *Curr_گرہ = ToVisit.نکالو_back_val();
    إذا (auto *Curr_Internal_گرہ = dyn_cast<Suffix_درخت_Internal_گرہ>(Curr_گرہ)) {
      // The current node is an internal node.
      auto I = ChildrenMap.find(Curr_Internal_گرہ);
      إذا (I == ChildrenMap.end()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        auto J = Curr_Internal_گرہ->Children.begin();
        إذا (J != Curr_Internal_گرہ->Children.end()) {
          ToVisit.ڈالو_back(Curr_گرہ);
          Suffix_درخت_گرہ *FirstChild = J->second;
          Suffix_درخت_گرہ *LastChild = nullptr;
          لكل (; J != Curr_Internal_گرہ->Children.end(); ++J) {
            LastChild = J->second;
            ToVisit.ڈالو_back(LastChild);
          }
          ChildrenMap[Curr_Internal_گرہ] = {FirstChild, LastChild};
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
        Curr_گرہ->set_دایاں_Leaf_Idx(FirstChild->get_دایاں_Leaf_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_گرہ->set_بایاں_Leaf_Idx(LastChild->get_بایاں_Leaf_Idx());
        أكد(Curr_گرہ->get_بایاں_Leaf_Idx() <= Curr_گرہ->get_دایاں_Leaf_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } وإلا {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_گرہ->set_بایاں_Leaf_Idx(LeafCounter);
      Curr_گرہ->set_دایاں_Leaf_Idx(LeafCounter);
      ++LeafCounter;
      auto *Curr_Leaf_گرہ = cast<Suffix_درخت_Leaf_گرہ>(Curr_گرہ);
      LeafNodes.ڈالو_back(Curr_Leaf_گرہ);
    }
  }
}

unsigned Suffix_درخت::extend(unsigned EndIdx, unsigned SuffixesToAdd) {
  Suffix_درخت_Internal_گرہ *NeedsLink = nullptr;

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
    إذا (auto It = Active.گرہ->Children.find(FirstChar);
        It == Active.گرہ->Children.end()) {
      // If not, then we can just insert a leaf and move to the next step.
      داخل_کرو_Leaf(*Active.گرہ, EndIdx, FirstChar);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      إذا (NeedsLink) {
        NeedsLink->setLink(Active.گرہ);
        NeedsLink = nullptr;
      }
    } وإلا {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_درخت_گرہ *Next_گرہ = It->second;

      unsigned SubstringLen = عدد_Elements_In_Substring(Next_گرہ);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      إذا (Active.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        أكد(isa<Suffix_درخت_Internal_گرہ>(Next_گرہ) &&
               "Expected an internal node?");
        Active.Idx += SubstringLen;
        Active.Len -= SubstringLen;
        Active.گرہ = cast<Suffix_درخت_Internal_گرہ>(Next_گرہ);
        استمر;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      unsigned LastChar = Str[EndIdx];

      // Is the string we're trying to insert a substring of the next node?
      إذا (Str[Next_گرہ->getStartIdx() + Active.Len] == LastChar) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        إذا (NeedsLink && !Active.گرہ->is_جڑ()) {
          NeedsLink->setLink(Active.گرہ);
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
      Suffix_درخت_Internal_گرہ *Split_گرہ = داخل_کرو_Internal_گرہ(
          Active.گرہ, Next_گرہ->getStartIdx(),
          Next_گرہ->getStartIdx() + Active.Len - 1, FirstChar);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      داخل_کرو_Leaf(*Split_گرہ, EndIdx, LastChar);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      Next_گرہ->incrementStartIdx(Active.Len);
      Split_گرہ->Children[Str[Next_گرہ->getStartIdx()]] = Next_گرہ;

      // SplitNode is an internal node, update the suffix link.
      إذا (NeedsLink)
        NeedsLink->setLink(Split_گرہ);

      NeedsLink = Split_گرہ;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    SuffixesToAdd--;

    إذا (Active.گرہ->is_جڑ()) {
      إذا (Active.Len > 0) {
        Active.Len--;
        Active.Idx = EndIdx - SuffixesToAdd + 1;
      }
    } وإلا {
      // Start the next phase at the next smallest suffix.
      Active.گرہ = Active.گرہ->getLink();
    }
  }

  أرجع SuffixesToAdd;
}

فراغ Suffix_درخت::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = nullptr;

  // Each leaf node represents a repeat of a string.
  SmallVector<unsigned> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  طالما (!InternalNodesToVisit.empty()) {
    RepeatedSubstringStarts.clear();
    auto *Curr = InternalNodesToVisit.نکالو_back_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    unsigned Length = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    لكل (auto &ChildPair : Curr->Children)
      // Save all of this node's children for processing.
      إذا (auto *InternalChild =
              dyn_cast<Suffix_درخت_Internal_گرہ>(ChildPair.second))
        InternalNodesToVisit.ڈالو_back(InternalChild);

    // If length of repeated substring is below threshold, then skip it.
    إذا (Length < MinLength)
      استمر;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    إذا (Curr->is_جڑ())
      استمر;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    إذا (OutlinerLeafDescendants) {
      لكل (unsigned I = Curr->get_بایاں_Leaf_Idx(); I <= Curr->get_دایاں_Leaf_Idx();
           ++I)
        RepeatedSubstringStarts.ڈالو_back(LeafNodes[I]->getSuffixIdx());
    } وإلا {
      لكل (auto &ChildPair : Curr->Children)
        إذا (auto *Leaf = dyn_cast<Suffix_درخت_Leaf_گرہ>(ChildPair.second))
          RepeatedSubstringStarts.ڈالو_back(Leaf->getSuffixIdx());
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
