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

#অন্তর্ভাৱ "llvm/Support/SuffixTree.h"
#অন্তর্ভাৱ "llvm/Support/Allocator.h"
#অন্তর্ভাৱ "llvm/Support/Casting.h"
#অন্তর্ভাৱ "llvm/Support/SuffixTreeNode.h"

using নামাকাশ llvm;

/// \returns the number of elements in the substring associated with \p N.
স্থির size_t অঙ্ক_Elements_In_Substring(ধ্রুৱ Suffix_বৃক্ষ_গ্ৰন্থি *N) {
  প্রতিজ্ঞা(N && "Got a null node?");
  যদি (auto *Internal = dyn_cast<Suffix_বৃক্ষ_Internal_গ্ৰন্থি>(N))
    যদি (Internal->is_মূল())
      ঘূৰাই 0;
  ঘূৰাই N->getEndIdx() - N->getStartIdx() + 1;
}

Suffix_বৃক্ষ::Suffix_বৃক্ষ(ধ্রুৱ ArrayRef<unsigned> &Str,
                       তার্কিক OutlinerLeafDescendants)
    : Str(Str), OutlinerLeafDescendants(OutlinerLeafDescendants) {
  মূল_2 = ঢোকাও_মূল();
  Active.গ্ৰন্থি = মূল_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  unsigned SuffixesToAdd = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  বাবে (unsigned PfxEndIdx = 0, End = Str.size(); PfxEndIdx < End; PfxEndIdx++) {
    SuffixesToAdd++;
    LeafEndIdx = PfxEndIdx; // Extend each of the leaves.
    SuffixesToAdd = extend(PfxEndIdx, SuffixesToAdd);
  }

  // Set the suffix indices of each leaf.
  প্রতিজ্ঞা(মূল_2 && "Root node can't be nullptr!");
  setSuffixIndices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  যদি (OutlinerLeafDescendants)
    setLeafNodes();
}

Suffix_বৃক্ষ_গ্ৰন্থি *Suffix_বৃক্ষ::ঢোকাও_Leaf(Suffix_বৃক্ষ_Internal_গ্ৰন্থি &Parent,
                                       unsigned StartIdx, unsigned Edge) {
  প্রতিজ্ঞা(StartIdx <= LeafEndIdx && "String can't start after it ends!");
  auto *N = নূতন (Leaf_গ্ৰন্থি_Allocator.Allocate())
      Suffix_বৃক্ষ_Leaf_গ্ৰন্থি(StartIdx, &LeafEndIdx);
  Parent.Children[Edge] = N;
  ঘূৰাই N;
}

Suffix_বৃক্ষ_Internal_গ্ৰন্থি *
Suffix_বৃক্ষ::ঢোকাও_Internal_গ্ৰন্থি(Suffix_বৃক্ষ_Internal_গ্ৰন্থি *Parent,
                               unsigned StartIdx, unsigned EndIdx,
                               unsigned Edge) {
  প্রতিজ্ঞা(StartIdx <= EndIdx && "String can't start after it ends!");
  প্রতিজ্ঞা(!(!Parent && StartIdx != Suffix_বৃক্ষ_গ্ৰন্থি::EmptyIdx) &&
         "Non-root internal nodes must have parents!");
  auto *N = নূতন (Internal_গ্ৰন্থি_Allocator.Allocate())
      Suffix_বৃক্ষ_Internal_গ্ৰন্থি(StartIdx, EndIdx, মূল_2);
  যদি (Parent)
    Parent->Children[Edge] = N;
  ঘূৰাই N;
}

Suffix_বৃক্ষ_Internal_গ্ৰন্থি *Suffix_বৃক্ষ::ঢোকাও_মূল() {
  ঘূৰাই ঢোকাও_Internal_গ্ৰন্থি(/*Parent = */ nullptr, Suffix_বৃক্ষ_গ্ৰন্থি::EmptyIdx,
                            Suffix_বৃক্ষ_গ্ৰন্থি::EmptyIdx, /*Edge = */ 0);
}

শূন্য Suffix_বৃক্ষ::setSuffixIndices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  SmallVector<std::pair<Suffix_বৃক্ষ_গ্ৰন্থি *, unsigned>> ToVisit;

  // Current node being visited.
  Suffix_বৃক্ষ_গ্ৰন্থি *Curr_গ্ৰন্থি = মূল_2;

  // Sum of the lengths of the nodes down the path to the current one.
  unsigned Curr_গ্ৰন্থি_Len = 0;
  ToVisit.ঠেলো_back({Curr_গ্ৰন্থি, Curr_গ্ৰন্থি_Len});
  যেতিয়ালৈ (!ToVisit.empty()) {
    std::tie(Curr_গ্ৰন্থি, Curr_গ্ৰন্থি_Len) = ToVisit.বেৰ_কৰো_back_val();
    // Length of the current node from the root down to here.
    Curr_গ্ৰন্থি->setConcatLen(Curr_গ্ৰন্থি_Len);
    যদি (auto *Internal_গ্ৰন্থি = dyn_cast<Suffix_বৃক্ষ_Internal_গ্ৰন্থি>(Curr_গ্ৰন্থি))
      বাবে (auto &ChildPair : Internal_গ্ৰন্থি->Children) {
        প্রতিজ্ঞা(ChildPair.second && "Node had a null child!");
        ToVisit.ঠেলো_back(
            {ChildPair.second,
             Curr_গ্ৰন্থি_Len + অঙ্ক_Elements_In_Substring(ChildPair.second)});
      }
    // No children, so we are at the end of the string.
    যদি (auto *Leaf_গ্ৰন্থি = dyn_cast<Suffix_বৃক্ষ_Leaf_গ্ৰন্থি>(Curr_গ্ৰন্থি))
      Leaf_গ্ৰন্থি->setSuffixIdx(Str.size() - Curr_গ্ৰন্থি_Len);
  }
}

শূন্য Suffix_বৃক্ষ::setLeafNodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  SmallVector<Suffix_বৃক্ষ_গ্ৰন্থি *> ToVisit;
  ToVisit.ঠেলো_back(মূল_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  unsigned LeafCounter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_বৃক্ষ_Internal_গ্ৰন্থি *,
           std::pair<Suffix_বৃক্ষ_গ্ৰন্থি *, Suffix_বৃক্ষ_গ্ৰন্থি *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  যেতিয়ালৈ (!ToVisit.empty()) {
    Suffix_বৃক্ষ_গ্ৰন্থি *Curr_গ্ৰন্থি = ToVisit.বেৰ_কৰো_back_val();
    যদি (auto *Curr_Internal_গ্ৰন্থি = dyn_cast<Suffix_বৃক্ষ_Internal_গ্ৰন্থি>(Curr_গ্ৰন্থি)) {
      // The current node is an internal node.
      auto I = ChildrenMap.find(Curr_Internal_গ্ৰন্থি);
      যদি (I == ChildrenMap.end()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        auto J = Curr_Internal_গ্ৰন্থি->Children.begin();
        যদি (J != Curr_Internal_গ্ৰন্থি->Children.end()) {
          ToVisit.ঠেলো_back(Curr_গ্ৰন্থি);
          Suffix_বৃক্ষ_গ্ৰন্থি *FirstChild = J->second;
          Suffix_বৃক্ষ_গ্ৰন্থি *LastChild = nullptr;
          বাবে (; J != Curr_Internal_গ্ৰন্থি->Children.end(); ++J) {
            LastChild = J->second;
            ToVisit.ঠেলো_back(LastChild);
          }
          ChildrenMap[Curr_Internal_গ্ৰন্থি] = {FirstChild, LastChild};
        }
      } অন্যথা {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        auto [FirstChild, LastChild] = I->second;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_গ্ৰন্থি->set_ডান_Leaf_Idx(FirstChild->get_ডান_Leaf_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_গ্ৰন্থি->set_বাম_Leaf_Idx(LastChild->get_বাম_Leaf_Idx());
        প্রতিজ্ঞা(Curr_গ্ৰন্থি->get_বাম_Leaf_Idx() <= Curr_গ্ৰন্থি->get_ডান_Leaf_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } অন্যথা {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_গ্ৰন্থি->set_বাম_Leaf_Idx(LeafCounter);
      Curr_গ্ৰন্থি->set_ডান_Leaf_Idx(LeafCounter);
      ++LeafCounter;
      auto *Curr_Leaf_গ্ৰন্থি = cast<Suffix_বৃক্ষ_Leaf_গ্ৰন্থি>(Curr_গ্ৰন্থি);
      LeafNodes.ঠেলো_back(Curr_Leaf_গ্ৰন্থি);
    }
  }
}

unsigned Suffix_বৃক্ষ::extend(unsigned EndIdx, unsigned SuffixesToAdd) {
  Suffix_বৃক্ষ_Internal_গ্ৰন্থি *NeedsLink = nullptr;

  যেতিয়ালৈ (SuffixesToAdd > 0) {

    // Are we waiting to add anything other than just the last character?
    যদি (Active.Len == 0) {
      // If not, then say the active index is the end index.
      Active.Idx = EndIdx;
    }

    প্রতিজ্ঞা(Active.Idx <= EndIdx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    unsigned FirstChar = Str[Active.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    যদি (auto It = Active.গ্ৰন্থি->Children.find(FirstChar);
        It == Active.গ্ৰন্থি->Children.end()) {
      // If not, then we can just insert a leaf and move to the next step.
      ঢোকাও_Leaf(*Active.গ্ৰন্থি, EndIdx, FirstChar);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      যদি (NeedsLink) {
        NeedsLink->setLink(Active.গ্ৰন্থি);
        NeedsLink = nullptr;
      }
    } অন্যথা {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_বৃক্ষ_গ্ৰন্থি *Next_গ্ৰন্থি = It->second;

      unsigned SubstringLen = অঙ্ক_Elements_In_Substring(Next_গ্ৰন্থি);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      যদি (Active.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        প্রতিজ্ঞা(isa<Suffix_বৃক্ষ_Internal_গ্ৰন্থি>(Next_গ্ৰন্থি) &&
               "Expected an internal node?");
        Active.Idx += SubstringLen;
        Active.Len -= SubstringLen;
        Active.গ্ৰন্থি = cast<Suffix_বৃক্ষ_Internal_গ্ৰন্থি>(Next_গ্ৰন্থি);
        অনুৱর্ত;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      unsigned LastChar = Str[EndIdx];

      // Is the string we're trying to insert a substring of the next node?
      যদি (Str[Next_গ্ৰন্থি->getStartIdx() + Active.Len] == LastChar) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        যদি (NeedsLink && !Active.গ্ৰন্থি->is_মূল()) {
          NeedsLink->setLink(Active.গ্ৰন্থি);
          NeedsLink = nullptr;
        }

        Active.Len++;
        ভঙ্গ;
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
      Suffix_বৃক্ষ_Internal_গ্ৰন্থি *Split_গ্ৰন্থি = ঢোকাও_Internal_গ্ৰন্থি(
          Active.গ্ৰন্থি, Next_গ্ৰন্থি->getStartIdx(),
          Next_গ্ৰন্থি->getStartIdx() + Active.Len - 1, FirstChar);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      ঢোকাও_Leaf(*Split_গ্ৰন্থি, EndIdx, LastChar);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      Next_গ্ৰন্থি->incrementStartIdx(Active.Len);
      Split_গ্ৰন্থি->Children[Str[Next_গ্ৰন্থি->getStartIdx()]] = Next_গ্ৰন্থি;

      // SplitNode is an internal node, update the suffix link.
      যদি (NeedsLink)
        NeedsLink->setLink(Split_গ্ৰন্থি);

      NeedsLink = Split_গ্ৰন্থি;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    SuffixesToAdd--;

    যদি (Active.গ্ৰন্থি->is_মূল()) {
      যদি (Active.Len > 0) {
        Active.Len--;
        Active.Idx = EndIdx - SuffixesToAdd + 1;
      }
    } অন্যথা {
      // Start the next phase at the next smallest suffix.
      Active.গ্ৰন্থি = Active.গ্ৰন্থি->getLink();
    }
  }

  ঘূৰাই SuffixesToAdd;
}

শূন্য Suffix_বৃক্ষ::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = nullptr;

  // Each leaf node represents a repeat of a string.
  SmallVector<unsigned> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  যেতিয়ালৈ (!InternalNodesToVisit.empty()) {
    RepeatedSubstringStarts.clear();
    auto *Curr = InternalNodesToVisit.বেৰ_কৰো_back_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    unsigned Length = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    বাবে (auto &ChildPair : Curr->Children)
      // Save all of this node's children for processing.
      যদি (auto *InternalChild =
              dyn_cast<Suffix_বৃক্ষ_Internal_গ্ৰন্থি>(ChildPair.second))
        InternalNodesToVisit.ঠেলো_back(InternalChild);

    // If length of repeated substring is below threshold, then skip it.
    যদি (Length < MinLength)
      অনুৱর্ত;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    যদি (Curr->is_মূল())
      অনুৱর্ত;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    যদি (OutlinerLeafDescendants) {
      বাবে (unsigned I = Curr->get_বাম_Leaf_Idx(); I <= Curr->get_ডান_Leaf_Idx();
           ++I)
        RepeatedSubstringStarts.ঠেলো_back(LeafNodes[I]->getSuffixIdx());
    } অন্যথা {
      বাবে (auto &ChildPair : Curr->Children)
        যদি (auto *Leaf = dyn_cast<Suffix_বৃক্ষ_Leaf_গ্ৰন্থি>(ChildPair.second))
          RepeatedSubstringStarts.ঠেলো_back(Leaf->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    যদি (RepeatedSubstringStarts.size() < 2)
      অনুৱর্ত;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.Length = Length;
    llvm::append_range(RS.StartIndices, RepeatedSubstringStarts);
    ভঙ্গ;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
