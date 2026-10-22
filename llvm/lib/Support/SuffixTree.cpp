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
ساكن size_t ژمارە_Elements_In_Substring(ثابت Suffix_درەخت_گرێ *N) {
  أكد(N && "Got a null node?");
  إذا (auto *Internal = dyn_cast<Suffix_درەخت_Internal_گرێ>(N))
    إذا (Internal->is_ڕەگ())
      أرجع 0;
  أرجع N->getEndIdx() - N->getStartIdx() + 1;
}

Suffix_درەخت::Suffix_درەخت(ثابت ArrayRef<unsigned> &Str,
                       منطقي OutlinerLeafDescendants)
    : Str(Str), OutlinerLeafDescendants(OutlinerLeafDescendants) {
  ڕەگ_2 = زیادکردن_ڕەگ();
  Active.گرێ = ڕەگ_2;

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
  أكد(ڕەگ_2 && "Root node can't be nullptr!");
  setSuffixIndices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  إذا (OutlinerLeafDescendants)
    setLeafNodes();
}

Suffix_درەخت_گرێ *Suffix_درەخت::زیادکردن_Leaf(Suffix_درەخت_Internal_گرێ &Parent,
                                       unsigned StartIdx, unsigned Edge) {
  أكد(StartIdx <= LeafEndIdx && "String can't start after it ends!");
  auto *N = جديد (Leaf_گرێ_Allocator.Allocate())
      Suffix_درەخت_Leaf_گرێ(StartIdx, &LeafEndIdx);
  Parent.Children[Edge] = N;
  أرجع N;
}

Suffix_درەخت_Internal_گرێ *
Suffix_درەخت::زیادکردن_Internal_گرێ(Suffix_درەخت_Internal_گرێ *Parent,
                               unsigned StartIdx, unsigned EndIdx,
                               unsigned Edge) {
  أكد(StartIdx <= EndIdx && "String can't start after it ends!");
  أكد(!(!Parent && StartIdx != Suffix_درەخت_گرێ::EmptyIdx) &&
         "Non-root internal nodes must have parents!");
  auto *N = جديد (Internal_گرێ_Allocator.Allocate())
      Suffix_درەخت_Internal_گرێ(StartIdx, EndIdx, ڕەگ_2);
  إذا (Parent)
    Parent->Children[Edge] = N;
  أرجع N;
}

Suffix_درەخت_Internal_گرێ *Suffix_درەخت::زیادکردن_ڕەگ() {
  أرجع زیادکردن_Internal_گرێ(/*Parent = */ nullptr, Suffix_درەخت_گرێ::EmptyIdx,
                            Suffix_درەخت_گرێ::EmptyIdx, /*Edge = */ 0);
}

فراغ Suffix_درەخت::setSuffixIndices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  SmallVector<std::pair<Suffix_درەخت_گرێ *, unsigned>> ToVisit;

  // Current node being visited.
  Suffix_درەخت_گرێ *Curr_گرێ = ڕەگ_2;

  // Sum of the lengths of the nodes down the path to the current one.
  unsigned Curr_گرێ_Len = 0;
  ToVisit.پاڵنان_back({Curr_گرێ, Curr_گرێ_Len});
  طالما (!ToVisit.empty()) {
    std::tie(Curr_گرێ, Curr_گرێ_Len) = ToVisit.دەرهێنان_back_val();
    // Length of the current node from the root down to here.
    Curr_گرێ->setConcatLen(Curr_گرێ_Len);
    إذا (auto *Internal_گرێ = dyn_cast<Suffix_درەخت_Internal_گرێ>(Curr_گرێ))
      لكل (auto &ChildPair : Internal_گرێ->Children) {
        أكد(ChildPair.second && "Node had a null child!");
        ToVisit.پاڵنان_back(
            {ChildPair.second,
             Curr_گرێ_Len + ژمارە_Elements_In_Substring(ChildPair.second)});
      }
    // No children, so we are at the end of the string.
    إذا (auto *Leaf_گرێ = dyn_cast<Suffix_درەخت_Leaf_گرێ>(Curr_گرێ))
      Leaf_گرێ->setSuffixIdx(Str.size() - Curr_گرێ_Len);
  }
}

فراغ Suffix_درەخت::setLeafNodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  SmallVector<Suffix_درەخت_گرێ *> ToVisit;
  ToVisit.پاڵنان_back(ڕەگ_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  unsigned LeafCounter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_درەخت_Internal_گرێ *,
           std::pair<Suffix_درەخت_گرێ *, Suffix_درەخت_گرێ *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  طالما (!ToVisit.empty()) {
    Suffix_درەخت_گرێ *Curr_گرێ = ToVisit.دەرهێنان_back_val();
    إذا (auto *Curr_Internal_گرێ = dyn_cast<Suffix_درەخت_Internal_گرێ>(Curr_گرێ)) {
      // The current node is an internal node.
      auto I = ChildrenMap.find(Curr_Internal_گرێ);
      إذا (I == ChildrenMap.end()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        auto J = Curr_Internal_گرێ->Children.begin();
        إذا (J != Curr_Internal_گرێ->Children.end()) {
          ToVisit.پاڵنان_back(Curr_گرێ);
          Suffix_درەخت_گرێ *FirstChild = J->second;
          Suffix_درەخت_گرێ *LastChild = nullptr;
          لكل (; J != Curr_Internal_گرێ->Children.end(); ++J) {
            LastChild = J->second;
            ToVisit.پاڵنان_back(LastChild);
          }
          ChildrenMap[Curr_Internal_گرێ] = {FirstChild, LastChild};
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
        Curr_گرێ->set_ڕاست_Leaf_Idx(FirstChild->get_ڕاست_Leaf_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_گرێ->set_چەپ_Leaf_Idx(LastChild->get_چەپ_Leaf_Idx());
        أكد(Curr_گرێ->get_چەپ_Leaf_Idx() <= Curr_گرێ->get_ڕاست_Leaf_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } وإلا {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_گرێ->set_چەپ_Leaf_Idx(LeafCounter);
      Curr_گرێ->set_ڕاست_Leaf_Idx(LeafCounter);
      ++LeafCounter;
      auto *Curr_Leaf_گرێ = cast<Suffix_درەخت_Leaf_گرێ>(Curr_گرێ);
      LeafNodes.پاڵنان_back(Curr_Leaf_گرێ);
    }
  }
}

unsigned Suffix_درەخت::extend(unsigned EndIdx, unsigned SuffixesToAdd) {
  Suffix_درەخت_Internal_گرێ *NeedsLink = nullptr;

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
    إذا (auto It = Active.گرێ->Children.find(FirstChar);
        It == Active.گرێ->Children.end()) {
      // If not, then we can just insert a leaf and move to the next step.
      زیادکردن_Leaf(*Active.گرێ, EndIdx, FirstChar);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      إذا (NeedsLink) {
        NeedsLink->setLink(Active.گرێ);
        NeedsLink = nullptr;
      }
    } وإلا {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_درەخت_گرێ *Next_گرێ = It->second;

      unsigned SubstringLen = ژمارە_Elements_In_Substring(Next_گرێ);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      إذا (Active.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        أكد(isa<Suffix_درەخت_Internal_گرێ>(Next_گرێ) &&
               "Expected an internal node?");
        Active.Idx += SubstringLen;
        Active.Len -= SubstringLen;
        Active.گرێ = cast<Suffix_درەخت_Internal_گرێ>(Next_گرێ);
        استمر;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      unsigned LastChar = Str[EndIdx];

      // Is the string we're trying to insert a substring of the next node?
      إذا (Str[Next_گرێ->getStartIdx() + Active.Len] == LastChar) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        إذا (NeedsLink && !Active.گرێ->is_ڕەگ()) {
          NeedsLink->setLink(Active.گرێ);
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
      Suffix_درەخت_Internal_گرێ *Split_گرێ = زیادکردن_Internal_گرێ(
          Active.گرێ, Next_گرێ->getStartIdx(),
          Next_گرێ->getStartIdx() + Active.Len - 1, FirstChar);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      زیادکردن_Leaf(*Split_گرێ, EndIdx, LastChar);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      Next_گرێ->incrementStartIdx(Active.Len);
      Split_گرێ->Children[Str[Next_گرێ->getStartIdx()]] = Next_گرێ;

      // SplitNode is an internal node, update the suffix link.
      إذا (NeedsLink)
        NeedsLink->setLink(Split_گرێ);

      NeedsLink = Split_گرێ;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    SuffixesToAdd--;

    إذا (Active.گرێ->is_ڕەگ()) {
      إذا (Active.Len > 0) {
        Active.Len--;
        Active.Idx = EndIdx - SuffixesToAdd + 1;
      }
    } وإلا {
      // Start the next phase at the next smallest suffix.
      Active.گرێ = Active.گرێ->getLink();
    }
  }

  أرجع SuffixesToAdd;
}

فراغ Suffix_درەخت::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = nullptr;

  // Each leaf node represents a repeat of a string.
  SmallVector<unsigned> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  طالما (!InternalNodesToVisit.empty()) {
    RepeatedSubstringStarts.clear();
    auto *Curr = InternalNodesToVisit.دەرهێنان_back_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    unsigned Length = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    لكل (auto &ChildPair : Curr->Children)
      // Save all of this node's children for processing.
      إذا (auto *InternalChild =
              dyn_cast<Suffix_درەخت_Internal_گرێ>(ChildPair.second))
        InternalNodesToVisit.پاڵنان_back(InternalChild);

    // If length of repeated substring is below threshold, then skip it.
    إذا (Length < MinLength)
      استمر;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    إذا (Curr->is_ڕەگ())
      استمر;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    إذا (OutlinerLeafDescendants) {
      لكل (unsigned I = Curr->get_چەپ_Leaf_Idx(); I <= Curr->get_ڕاست_Leaf_Idx();
           ++I)
        RepeatedSubstringStarts.پاڵنان_back(LeafNodes[I]->getSuffixIdx());
    } وإلا {
      لكل (auto &ChildPair : Curr->Children)
        إذا (auto *Leaf = dyn_cast<Suffix_درەخت_Leaf_گرێ>(ChildPair.second))
          RepeatedSubstringStarts.پاڵنان_back(Leaf->getSuffixIdx());
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
