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

#شامل "llvm/Support/SuffixTree.h"
#شامل "llvm/Support/Allocator.h"
#شامل "llvm/Support/Casting.h"
#شامل "llvm/Support/SuffixTreeNode.h"

using فضاینام llvm;

/// \returns the number of elements in the substring associated with \p N.
ایستا size_t رقم_Elements_In_Substring(ثابت Suffix_درخت_گره *N) {
  أكد(N && "Got a null node?");
  اگر (auto *Internal = dyn_cast<Suffix_درخت_Internal_گره>(N))
    اگر (Internal->is_ریشه())
      بازگشت 0;
  بازگشت N->getEndIdx() - N->getStartIdx() + 1;
}

Suffix_درخت::Suffix_درخت(ثابت ArrayRef<unsigned> &Str,
                       منطقی OutlinerLeafDescendants)
    : Str(Str), OutlinerLeafDescendants(OutlinerLeafDescendants) {
  ریشه_2 = درج_ریشه();
  Active.گره = ریشه_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  unsigned SuffixesToAdd = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  برای (unsigned PfxEndIdx = 0, End = Str.size(); PfxEndIdx < End; PfxEndIdx++) {
    SuffixesToAdd++;
    LeafEndIdx = PfxEndIdx; // Extend each of the leaves.
    SuffixesToAdd = extend(PfxEndIdx, SuffixesToAdd);
  }

  // Set the suffix indices of each leaf.
  أكد(ریشه_2 && "Root node can't be nullptr!");
  setSuffixIndices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  اگر (OutlinerLeafDescendants)
    setLeafNodes();
}

Suffix_درخت_گره *Suffix_درخت::درج_Leaf(Suffix_درخت_Internal_گره &Parent,
                                       unsigned StartIdx, unsigned Edge) {
  أكد(StartIdx <= LeafEndIdx && "String can't start after it ends!");
  auto *N = جدید (Leaf_گره_Allocator.Allocate())
      Suffix_درخت_Leaf_گره(StartIdx, &LeafEndIdx);
  Parent.Children[Edge] = N;
  بازگشت N;
}

Suffix_درخت_Internal_گره *
Suffix_درخت::درج_Internal_گره(Suffix_درخت_Internal_گره *Parent,
                               unsigned StartIdx, unsigned EndIdx,
                               unsigned Edge) {
  أكد(StartIdx <= EndIdx && "String can't start after it ends!");
  أكد(!(!Parent && StartIdx != Suffix_درخت_گره::EmptyIdx) &&
         "Non-root internal nodes must have parents!");
  auto *N = جدید (Internal_گره_Allocator.Allocate())
      Suffix_درخت_Internal_گره(StartIdx, EndIdx, ریشه_2);
  اگر (Parent)
    Parent->Children[Edge] = N;
  بازگشت N;
}

Suffix_درخت_Internal_گره *Suffix_درخت::درج_ریشه() {
  بازگشت درج_Internal_گره(/*Parent = */ nullptr, Suffix_درخت_گره::EmptyIdx,
                            Suffix_درخت_گره::EmptyIdx, /*Edge = */ 0);
}

تهی Suffix_درخت::setSuffixIndices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  SmallVector<std::pair<Suffix_درخت_گره *, unsigned>> ToVisit;

  // Current node being visited.
  Suffix_درخت_گره *Curr_گره = ریشه_2;

  // Sum of the lengths of the nodes down the path to the current one.
  unsigned Curr_گره_Len = 0;
  ToVisit.افزودن_back({Curr_گره, Curr_گره_Len});
  وقتی (!ToVisit.empty()) {
    std::tie(Curr_گره, Curr_گره_Len) = ToVisit.برداشت_back_val();
    // Length of the current node from the root down to here.
    Curr_گره->setConcatLen(Curr_گره_Len);
    اگر (auto *Internal_گره = dyn_cast<Suffix_درخت_Internal_گره>(Curr_گره))
      برای (auto &ChildPair : Internal_گره->Children) {
        أكد(ChildPair.second && "Node had a null child!");
        ToVisit.افزودن_back(
            {ChildPair.second,
             Curr_گره_Len + رقم_Elements_In_Substring(ChildPair.second)});
      }
    // No children, so we are at the end of the string.
    اگر (auto *Leaf_گره = dyn_cast<Suffix_درخت_Leaf_گره>(Curr_گره))
      Leaf_گره->setSuffixIdx(Str.size() - Curr_گره_Len);
  }
}

تهی Suffix_درخت::setLeafNodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  SmallVector<Suffix_درخت_گره *> ToVisit;
  ToVisit.افزودن_back(ریشه_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  unsigned LeafCounter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_درخت_Internal_گره *,
           std::pair<Suffix_درخت_گره *, Suffix_درخت_گره *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  وقتی (!ToVisit.empty()) {
    Suffix_درخت_گره *Curr_گره = ToVisit.برداشت_back_val();
    اگر (auto *Curr_Internal_گره = dyn_cast<Suffix_درخت_Internal_گره>(Curr_گره)) {
      // The current node is an internal node.
      auto I = ChildrenMap.find(Curr_Internal_گره);
      اگر (I == ChildrenMap.end()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        auto J = Curr_Internal_گره->Children.begin();
        اگر (J != Curr_Internal_گره->Children.end()) {
          ToVisit.افزودن_back(Curr_گره);
          Suffix_درخت_گره *FirstChild = J->second;
          Suffix_درخت_گره *LastChild = nullptr;
          برای (; J != Curr_Internal_گره->Children.end(); ++J) {
            LastChild = J->second;
            ToVisit.افزودن_back(LastChild);
          }
          ChildrenMap[Curr_Internal_گره] = {FirstChild, LastChild};
        }
      } وگرنه {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        auto [FirstChild, LastChild] = I->second;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_گره->set_راست_Leaf_Idx(FirstChild->get_راست_Leaf_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_گره->set_چپ_Leaf_Idx(LastChild->get_چپ_Leaf_Idx());
        أكد(Curr_گره->get_چپ_Leaf_Idx() <= Curr_گره->get_راست_Leaf_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } وگرنه {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_گره->set_چپ_Leaf_Idx(LeafCounter);
      Curr_گره->set_راست_Leaf_Idx(LeafCounter);
      ++LeafCounter;
      auto *Curr_Leaf_گره = cast<Suffix_درخت_Leaf_گره>(Curr_گره);
      LeafNodes.افزودن_back(Curr_Leaf_گره);
    }
  }
}

unsigned Suffix_درخت::extend(unsigned EndIdx, unsigned SuffixesToAdd) {
  Suffix_درخت_Internal_گره *NeedsLink = nullptr;

  وقتی (SuffixesToAdd > 0) {

    // Are we waiting to add anything other than just the last character?
    اگر (Active.Len == 0) {
      // If not, then say the active index is the end index.
      Active.Idx = EndIdx;
    }

    أكد(Active.Idx <= EndIdx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    unsigned FirstChar = Str[Active.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    اگر (auto It = Active.گره->Children.find(FirstChar);
        It == Active.گره->Children.end()) {
      // If not, then we can just insert a leaf and move to the next step.
      درج_Leaf(*Active.گره, EndIdx, FirstChar);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      اگر (NeedsLink) {
        NeedsLink->setLink(Active.گره);
        NeedsLink = nullptr;
      }
    } وگرنه {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_درخت_گره *Next_گره = It->second;

      unsigned SubstringLen = رقم_Elements_In_Substring(Next_گره);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      اگر (Active.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        أكد(isa<Suffix_درخت_Internal_گره>(Next_گره) &&
               "Expected an internal node?");
        Active.Idx += SubstringLen;
        Active.Len -= SubstringLen;
        Active.گره = cast<Suffix_درخت_Internal_گره>(Next_گره);
        ادامه;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      unsigned LastChar = Str[EndIdx];

      // Is the string we're trying to insert a substring of the next node?
      اگر (Str[Next_گره->getStartIdx() + Active.Len] == LastChar) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        اگر (NeedsLink && !Active.گره->is_ریشه()) {
          NeedsLink->setLink(Active.گره);
          NeedsLink = nullptr;
        }

        Active.Len++;
        بشکن;
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
      Suffix_درخت_Internal_گره *Split_گره = درج_Internal_گره(
          Active.گره, Next_گره->getStartIdx(),
          Next_گره->getStartIdx() + Active.Len - 1, FirstChar);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      درج_Leaf(*Split_گره, EndIdx, LastChar);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      Next_گره->incrementStartIdx(Active.Len);
      Split_گره->Children[Str[Next_گره->getStartIdx()]] = Next_گره;

      // SplitNode is an internal node, update the suffix link.
      اگر (NeedsLink)
        NeedsLink->setLink(Split_گره);

      NeedsLink = Split_گره;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    SuffixesToAdd--;

    اگر (Active.گره->is_ریشه()) {
      اگر (Active.Len > 0) {
        Active.Len--;
        Active.Idx = EndIdx - SuffixesToAdd + 1;
      }
    } وگرنه {
      // Start the next phase at the next smallest suffix.
      Active.گره = Active.گره->getLink();
    }
  }

  بازگشت SuffixesToAdd;
}

تهی Suffix_درخت::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = nullptr;

  // Each leaf node represents a repeat of a string.
  SmallVector<unsigned> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  وقتی (!InternalNodesToVisit.empty()) {
    RepeatedSubstringStarts.clear();
    auto *Curr = InternalNodesToVisit.برداشت_back_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    unsigned Length = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    برای (auto &ChildPair : Curr->Children)
      // Save all of this node's children for processing.
      اگر (auto *InternalChild =
              dyn_cast<Suffix_درخت_Internal_گره>(ChildPair.second))
        InternalNodesToVisit.افزودن_back(InternalChild);

    // If length of repeated substring is below threshold, then skip it.
    اگر (Length < MinLength)
      ادامه;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    اگر (Curr->is_ریشه())
      ادامه;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    اگر (OutlinerLeafDescendants) {
      برای (unsigned I = Curr->get_چپ_Leaf_Idx(); I <= Curr->get_راست_Leaf_Idx();
           ++I)
        RepeatedSubstringStarts.افزودن_back(LeafNodes[I]->getSuffixIdx());
    } وگرنه {
      برای (auto &ChildPair : Curr->Children)
        اگر (auto *Leaf = dyn_cast<Suffix_درخت_Leaf_گره>(ChildPair.second))
          RepeatedSubstringStarts.افزودن_back(Leaf->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    اگر (RepeatedSubstringStarts.size() < 2)
      ادامه;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.Length = Length;
    llvm::append_range(RS.StartIndices, RepeatedSubstringStarts);
    بشکن;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
