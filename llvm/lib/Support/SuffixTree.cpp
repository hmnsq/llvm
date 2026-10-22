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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/SuffixTree.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Allocator.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Casting.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/SuffixTreeNode.h"

using ନାମାକାଶ llvm;

/// \returns the number of elements in the substring associated with \p N.
ସ୍ଥିର size_t ଅଙ୍କ_Elements_In_Substring(ଅଚଳ Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି *N) {
  ପ୍ରତିଜ୍ଞା(N && "Got a null node?");
  ଯଦି (auto *Internal = dyn_cast<Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି>(N))
    ଯଦି (Internal->is_ମୂଳ())
      ଫେରନ୍ତୁ 0;
  ଫେରନ୍ତୁ N->getEndIdx() - N->getStartIdx() + 1;
}

Suffix_ବୃକ୍ଷ::Suffix_ବୃକ୍ଷ(ଅଚଳ ArrayRef<unsigned> &Str,
                       ତାର୍କିକ OutlinerLeafDescendants)
    : Str(Str), OutlinerLeafDescendants(OutlinerLeafDescendants) {
  ମୂଳ_2 = ଭର୍ତ୍ତି_ମୂଳ();
  Active.ଗଣ୍ଠି = ମୂଳ_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  unsigned SuffixesToAdd = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  ପାଇଁ (unsigned PfxEndIdx = 0, End = Str.size(); PfxEndIdx < End; PfxEndIdx++) {
    SuffixesToAdd++;
    LeafEndIdx = PfxEndIdx; // Extend each of the leaves.
    SuffixesToAdd = extend(PfxEndIdx, SuffixesToAdd);
  }

  // Set the suffix indices of each leaf.
  ପ୍ରତିଜ୍ଞା(ମୂଳ_2 && "Root node can't be nullptr!");
  setSuffixIndices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  ଯଦି (OutlinerLeafDescendants)
    setLeafNodes();
}

Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି *Suffix_ବୃକ୍ଷ::ଭର୍ତ୍ତି_Leaf(Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି &Parent,
                                       unsigned StartIdx, unsigned Edge) {
  ପ୍ରତିଜ୍ଞା(StartIdx <= LeafEndIdx && "String can't start after it ends!");
  auto *N = ନୂତନ (Leaf_ଗଣ୍ଠି_Allocator.Allocate())
      Suffix_ବୃକ୍ଷ_Leaf_ଗଣ୍ଠି(StartIdx, &LeafEndIdx);
  Parent.Children[Edge] = N;
  ଫେରନ୍ତୁ N;
}

Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି *
Suffix_ବୃକ୍ଷ::ଭର୍ତ୍ତି_Internal_ଗଣ୍ଠି(Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି *Parent,
                               unsigned StartIdx, unsigned EndIdx,
                               unsigned Edge) {
  ପ୍ରତିଜ୍ଞା(StartIdx <= EndIdx && "String can't start after it ends!");
  ପ୍ରତିଜ୍ଞା(!(!Parent && StartIdx != Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି::EmptyIdx) &&
         "Non-root internal nodes must have parents!");
  auto *N = ନୂତନ (Internal_ଗଣ୍ଠି_Allocator.Allocate())
      Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି(StartIdx, EndIdx, ମୂଳ_2);
  ଯଦି (Parent)
    Parent->Children[Edge] = N;
  ଫେରନ୍ତୁ N;
}

Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି *Suffix_ବୃକ୍ଷ::ଭର୍ତ୍ତି_ମୂଳ() {
  ଫେରନ୍ତୁ ଭର୍ତ୍ତି_Internal_ଗଣ୍ଠି(/*Parent = */ nullptr, Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି::EmptyIdx,
                            Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି::EmptyIdx, /*Edge = */ 0);
}

ଶୂନ୍ୟ Suffix_ବୃକ୍ଷ::setSuffixIndices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  SmallVector<std::pair<Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି *, unsigned>> ToVisit;

  // Current node being visited.
  Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି *Curr_ଗଣ୍ଠି = ମୂଳ_2;

  // Sum of the lengths of the nodes down the path to the current one.
  unsigned Curr_ଗଣ୍ଠି_Len = 0;
  ToVisit.ଠେଲ_back({Curr_ଗଣ୍ଠି, Curr_ଗଣ୍ଠି_Len});
  ଯେତେବେଳେ (!ToVisit.empty()) {
    std::tie(Curr_ଗଣ୍ଠି, Curr_ଗଣ୍ଠି_Len) = ToVisit.କାଢ଼_back_val();
    // Length of the current node from the root down to here.
    Curr_ଗଣ୍ଠି->setConcatLen(Curr_ଗଣ୍ଠି_Len);
    ଯଦି (auto *Internal_ଗଣ୍ଠି = dyn_cast<Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି>(Curr_ଗଣ୍ଠି))
      ପାଇଁ (auto &ChildPair : Internal_ଗଣ୍ଠି->Children) {
        ପ୍ରତିଜ୍ଞା(ChildPair.second && "Node had a null child!");
        ToVisit.ଠେଲ_back(
            {ChildPair.second,
             Curr_ଗଣ୍ଠି_Len + ଅଙ୍କ_Elements_In_Substring(ChildPair.second)});
      }
    // No children, so we are at the end of the string.
    ଯଦି (auto *Leaf_ଗଣ୍ଠି = dyn_cast<Suffix_ବୃକ୍ଷ_Leaf_ଗଣ୍ଠି>(Curr_ଗଣ୍ଠି))
      Leaf_ଗଣ୍ଠି->setSuffixIdx(Str.size() - Curr_ଗଣ୍ଠି_Len);
  }
}

ଶୂନ୍ୟ Suffix_ବୃକ୍ଷ::setLeafNodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  SmallVector<Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି *> ToVisit;
  ToVisit.ଠେଲ_back(ମୂଳ_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  unsigned LeafCounter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି *,
           std::pair<Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି *, Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  ଯେତେବେଳେ (!ToVisit.empty()) {
    Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି *Curr_ଗଣ୍ଠି = ToVisit.କାଢ଼_back_val();
    ଯଦି (auto *Curr_Internal_ଗଣ୍ଠି = dyn_cast<Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି>(Curr_ଗଣ୍ଠି)) {
      // The current node is an internal node.
      auto I = ChildrenMap.find(Curr_Internal_ଗଣ୍ଠି);
      ଯଦି (I == ChildrenMap.end()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        auto J = Curr_Internal_ଗଣ୍ଠି->Children.begin();
        ଯଦି (J != Curr_Internal_ଗଣ୍ଠି->Children.end()) {
          ToVisit.ଠେଲ_back(Curr_ଗଣ୍ଠି);
          Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି *FirstChild = J->second;
          Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି *LastChild = nullptr;
          ପାଇଁ (; J != Curr_Internal_ଗଣ୍ଠି->Children.end(); ++J) {
            LastChild = J->second;
            ToVisit.ଠେଲ_back(LastChild);
          }
          ChildrenMap[Curr_Internal_ଗଣ୍ଠି] = {FirstChild, LastChild};
        }
      } ଅନ୍ୟଥା {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        auto [FirstChild, LastChild] = I->second;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_ଗଣ୍ଠି->set_ଡାହାଣ_Leaf_Idx(FirstChild->get_ଡାହାଣ_Leaf_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_ଗଣ୍ଠି->set_ବାମ_Leaf_Idx(LastChild->get_ବାମ_Leaf_Idx());
        ପ୍ରତିଜ୍ଞା(Curr_ଗଣ୍ଠି->get_ବାମ_Leaf_Idx() <= Curr_ଗଣ୍ଠି->get_ଡାହାଣ_Leaf_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } ଅନ୍ୟଥା {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_ଗଣ୍ଠି->set_ବାମ_Leaf_Idx(LeafCounter);
      Curr_ଗଣ୍ଠି->set_ଡାହାଣ_Leaf_Idx(LeafCounter);
      ++LeafCounter;
      auto *Curr_Leaf_ଗଣ୍ଠି = cast<Suffix_ବୃକ୍ଷ_Leaf_ଗଣ୍ଠି>(Curr_ଗଣ୍ଠି);
      LeafNodes.ଠେଲ_back(Curr_Leaf_ଗଣ୍ଠି);
    }
  }
}

unsigned Suffix_ବୃକ୍ଷ::extend(unsigned EndIdx, unsigned SuffixesToAdd) {
  Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି *NeedsLink = nullptr;

  ଯେତେବେଳେ (SuffixesToAdd > 0) {

    // Are we waiting to add anything other than just the last character?
    ଯଦି (Active.Len == 0) {
      // If not, then say the active index is the end index.
      Active.Idx = EndIdx;
    }

    ପ୍ରତିଜ୍ଞା(Active.Idx <= EndIdx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    unsigned FirstChar = Str[Active.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    ଯଦି (auto It = Active.ଗଣ୍ଠି->Children.find(FirstChar);
        It == Active.ଗଣ୍ଠି->Children.end()) {
      // If not, then we can just insert a leaf and move to the next step.
      ଭର୍ତ୍ତି_Leaf(*Active.ଗଣ୍ଠି, EndIdx, FirstChar);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      ଯଦି (NeedsLink) {
        NeedsLink->setLink(Active.ଗଣ୍ଠି);
        NeedsLink = nullptr;
      }
    } ଅନ୍ୟଥା {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_ବୃକ୍ଷ_ଗଣ୍ଠି *Next_ଗଣ୍ଠି = It->second;

      unsigned SubstringLen = ଅଙ୍କ_Elements_In_Substring(Next_ଗଣ୍ଠି);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      ଯଦି (Active.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        ପ୍ରତିଜ୍ଞା(isa<Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି>(Next_ଗଣ୍ଠି) &&
               "Expected an internal node?");
        Active.Idx += SubstringLen;
        Active.Len -= SubstringLen;
        Active.ଗଣ୍ଠି = cast<Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି>(Next_ଗଣ୍ଠି);
        ଜାରି;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      unsigned LastChar = Str[EndIdx];

      // Is the string we're trying to insert a substring of the next node?
      ଯଦି (Str[Next_ଗଣ୍ଠି->getStartIdx() + Active.Len] == LastChar) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        ଯଦି (NeedsLink && !Active.ଗଣ୍ଠି->is_ମୂଳ()) {
          NeedsLink->setLink(Active.ଗଣ୍ଠି);
          NeedsLink = nullptr;
        }

        Active.Len++;
        ଭାଙ୍ଗ;
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
      Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି *Split_ଗଣ୍ଠି = ଭର୍ତ୍ତି_Internal_ଗଣ୍ଠି(
          Active.ଗଣ୍ଠି, Next_ଗଣ୍ଠି->getStartIdx(),
          Next_ଗଣ୍ଠି->getStartIdx() + Active.Len - 1, FirstChar);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      ଭର୍ତ୍ତି_Leaf(*Split_ଗଣ୍ଠି, EndIdx, LastChar);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      Next_ଗଣ୍ଠି->incrementStartIdx(Active.Len);
      Split_ଗଣ୍ଠି->Children[Str[Next_ଗଣ୍ଠି->getStartIdx()]] = Next_ଗଣ୍ଠି;

      // SplitNode is an internal node, update the suffix link.
      ଯଦି (NeedsLink)
        NeedsLink->setLink(Split_ଗଣ୍ଠି);

      NeedsLink = Split_ଗଣ୍ଠି;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    SuffixesToAdd--;

    ଯଦି (Active.ଗଣ୍ଠି->is_ମୂଳ()) {
      ଯଦି (Active.Len > 0) {
        Active.Len--;
        Active.Idx = EndIdx - SuffixesToAdd + 1;
      }
    } ଅନ୍ୟଥା {
      // Start the next phase at the next smallest suffix.
      Active.ଗଣ୍ଠି = Active.ଗଣ୍ଠି->getLink();
    }
  }

  ଫେରନ୍ତୁ SuffixesToAdd;
}

ଶୂନ୍ୟ Suffix_ବୃକ୍ଷ::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = nullptr;

  // Each leaf node represents a repeat of a string.
  SmallVector<unsigned> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  ଯେତେବେଳେ (!InternalNodesToVisit.empty()) {
    RepeatedSubstringStarts.clear();
    auto *Curr = InternalNodesToVisit.କାଢ଼_back_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    unsigned Length = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    ପାଇଁ (auto &ChildPair : Curr->Children)
      // Save all of this node's children for processing.
      ଯଦି (auto *InternalChild =
              dyn_cast<Suffix_ବୃକ୍ଷ_Internal_ଗଣ୍ଠି>(ChildPair.second))
        InternalNodesToVisit.ଠେଲ_back(InternalChild);

    // If length of repeated substring is below threshold, then skip it.
    ଯଦି (Length < MinLength)
      ଜାରି;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    ଯଦି (Curr->is_ମୂଳ())
      ଜାରି;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    ଯଦି (OutlinerLeafDescendants) {
      ପାଇଁ (unsigned I = Curr->get_ବାମ_Leaf_Idx(); I <= Curr->get_ଡାହାଣ_Leaf_Idx();
           ++I)
        RepeatedSubstringStarts.ଠେଲ_back(LeafNodes[I]->getSuffixIdx());
    } ଅନ୍ୟଥା {
      ପାଇଁ (auto &ChildPair : Curr->Children)
        ଯଦି (auto *Leaf = dyn_cast<Suffix_ବୃକ୍ଷ_Leaf_ଗଣ୍ଠି>(ChildPair.second))
          RepeatedSubstringStarts.ଠେଲ_back(Leaf->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    ଯଦି (RepeatedSubstringStarts.size() < 2)
      ଜାରି;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.Length = Length;
    llvm::append_range(RS.StartIndices, RepeatedSubstringStarts);
    ଭାଙ୍ଗ;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
