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

#அந்தர்பாவ "llvm/Support/SuffixTree.h"
#அந்தர்பாவ "llvm/Support/Allocator.h"
#அந்தர்பாவ "llvm/Support/Casting.h"
#அந்தர்பாவ "llvm/Support/SuffixTreeNode.h"

using நாமாகாஶ llvm;

/// \returns the number of elements in the substring associated with \p N.
நிலையான size_t இலக்கம்_Elements_In_Substring(மாறிலி Suffix_மரம்_முனை *N) {
  ப்ரதிஜ்ஞா(N && "Got a null node?");
  எனில் (auto *Internal = dyn_cast<Suffix_மரம்_Internal_முனை>(N))
    எனில் (Internal->is_வேர்())
      திரும்பு 0;
  திரும்பு N->getEndIdx() - N->getStartIdx() + 1;
}

Suffix_மரம்::Suffix_மரம்(மாறிலி ArrayRef<unsigned> &Str,
                       தருக்க OutlinerLeafDescendants)
    : Str(Str), OutlinerLeafDescendants(OutlinerLeafDescendants) {
  வேர்_2 = செருகு_வேர்();
  Active.முனை = வேர்_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  unsigned SuffixesToAdd = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  க்கு (unsigned PfxEndIdx = 0, End = Str.size(); PfxEndIdx < End; PfxEndIdx++) {
    SuffixesToAdd++;
    LeafEndIdx = PfxEndIdx; // Extend each of the leaves.
    SuffixesToAdd = extend(PfxEndIdx, SuffixesToAdd);
  }

  // Set the suffix indices of each leaf.
  ப்ரதிஜ்ஞா(வேர்_2 && "Root node can't be nullptr!");
  setSuffixIndices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  எனில் (OutlinerLeafDescendants)
    setLeafNodes();
}

Suffix_மரம்_முனை *Suffix_மரம்::செருகு_Leaf(Suffix_மரம்_Internal_முனை &Parent,
                                       unsigned StartIdx, unsigned Edge) {
  ப்ரதிஜ்ஞா(StartIdx <= LeafEndIdx && "String can't start after it ends!");
  auto *N = புதிய (Leaf_முனை_Allocator.Allocate())
      Suffix_மரம்_Leaf_முனை(StartIdx, &LeafEndIdx);
  Parent.Children[Edge] = N;
  திரும்பு N;
}

Suffix_மரம்_Internal_முனை *
Suffix_மரம்::செருகு_Internal_முனை(Suffix_மரம்_Internal_முனை *Parent,
                               unsigned StartIdx, unsigned EndIdx,
                               unsigned Edge) {
  ப்ரதிஜ்ஞா(StartIdx <= EndIdx && "String can't start after it ends!");
  ப்ரதிஜ்ஞா(!(!Parent && StartIdx != Suffix_மரம்_முனை::EmptyIdx) &&
         "Non-root internal nodes must have parents!");
  auto *N = புதிய (Internal_முனை_Allocator.Allocate())
      Suffix_மரம்_Internal_முனை(StartIdx, EndIdx, வேர்_2);
  எனில் (Parent)
    Parent->Children[Edge] = N;
  திரும்பு N;
}

Suffix_மரம்_Internal_முனை *Suffix_மரம்::செருகு_வேர்() {
  திரும்பு செருகு_Internal_முனை(/*Parent = */ nullptr, Suffix_மரம்_முனை::EmptyIdx,
                            Suffix_மரம்_முனை::EmptyIdx, /*Edge = */ 0);
}

வெறுமை Suffix_மரம்::setSuffixIndices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  SmallVector<std::pair<Suffix_மரம்_முனை *, unsigned>> ToVisit;

  // Current node being visited.
  Suffix_மரம்_முனை *Curr_முனை = வேர்_2;

  // Sum of the lengths of the nodes down the path to the current one.
  unsigned Curr_முனை_Len = 0;
  ToVisit.தள்ளு_back({Curr_முனை, Curr_முனை_Len});
  போது (!ToVisit.empty()) {
    std::tie(Curr_முனை, Curr_முனை_Len) = ToVisit.எடு_back_val();
    // Length of the current node from the root down to here.
    Curr_முனை->setConcatLen(Curr_முனை_Len);
    எனில் (auto *Internal_முனை = dyn_cast<Suffix_மரம்_Internal_முனை>(Curr_முனை))
      க்கு (auto &ChildPair : Internal_முனை->Children) {
        ப்ரதிஜ்ஞா(ChildPair.second && "Node had a null child!");
        ToVisit.தள்ளு_back(
            {ChildPair.second,
             Curr_முனை_Len + இலக்கம்_Elements_In_Substring(ChildPair.second)});
      }
    // No children, so we are at the end of the string.
    எனில் (auto *Leaf_முனை = dyn_cast<Suffix_மரம்_Leaf_முனை>(Curr_முனை))
      Leaf_முனை->setSuffixIdx(Str.size() - Curr_முனை_Len);
  }
}

வெறுமை Suffix_மரம்::setLeafNodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  SmallVector<Suffix_மரம்_முனை *> ToVisit;
  ToVisit.தள்ளு_back(வேர்_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  unsigned LeafCounter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_மரம்_Internal_முனை *,
           std::pair<Suffix_மரம்_முனை *, Suffix_மரம்_முனை *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  போது (!ToVisit.empty()) {
    Suffix_மரம்_முனை *Curr_முனை = ToVisit.எடு_back_val();
    எனில் (auto *Curr_Internal_முனை = dyn_cast<Suffix_மரம்_Internal_முனை>(Curr_முனை)) {
      // The current node is an internal node.
      auto I = ChildrenMap.find(Curr_Internal_முனை);
      எனில் (I == ChildrenMap.end()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        auto J = Curr_Internal_முனை->Children.begin();
        எனில் (J != Curr_Internal_முனை->Children.end()) {
          ToVisit.தள்ளு_back(Curr_முனை);
          Suffix_மரம்_முனை *FirstChild = J->second;
          Suffix_மரம்_முனை *LastChild = nullptr;
          க்கு (; J != Curr_Internal_முனை->Children.end(); ++J) {
            LastChild = J->second;
            ToVisit.தள்ளு_back(LastChild);
          }
          ChildrenMap[Curr_Internal_முனை] = {FirstChild, LastChild};
        }
      } இல்லையெனில் {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        auto [FirstChild, LastChild] = I->second;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_முனை->set_வலது_Leaf_Idx(FirstChild->get_வலது_Leaf_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_முனை->set_இடது_Leaf_Idx(LastChild->get_இடது_Leaf_Idx());
        ப்ரதிஜ்ஞா(Curr_முனை->get_இடது_Leaf_Idx() <= Curr_முனை->get_வலது_Leaf_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } இல்லையெனில் {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_முனை->set_இடது_Leaf_Idx(LeafCounter);
      Curr_முனை->set_வலது_Leaf_Idx(LeafCounter);
      ++LeafCounter;
      auto *Curr_Leaf_முனை = cast<Suffix_மரம்_Leaf_முனை>(Curr_முனை);
      LeafNodes.தள்ளு_back(Curr_Leaf_முனை);
    }
  }
}

unsigned Suffix_மரம்::extend(unsigned EndIdx, unsigned SuffixesToAdd) {
  Suffix_மரம்_Internal_முனை *NeedsLink = nullptr;

  போது (SuffixesToAdd > 0) {

    // Are we waiting to add anything other than just the last character?
    எனில் (Active.Len == 0) {
      // If not, then say the active index is the end index.
      Active.Idx = EndIdx;
    }

    ப்ரதிஜ்ஞா(Active.Idx <= EndIdx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    unsigned FirstChar = Str[Active.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    எனில் (auto It = Active.முனை->Children.find(FirstChar);
        It == Active.முனை->Children.end()) {
      // If not, then we can just insert a leaf and move to the next step.
      செருகு_Leaf(*Active.முனை, EndIdx, FirstChar);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      எனில் (NeedsLink) {
        NeedsLink->setLink(Active.முனை);
        NeedsLink = nullptr;
      }
    } இல்லையெனில் {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_மரம்_முனை *Next_முனை = It->second;

      unsigned SubstringLen = இலக்கம்_Elements_In_Substring(Next_முனை);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      எனில் (Active.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        ப்ரதிஜ்ஞா(isa<Suffix_மரம்_Internal_முனை>(Next_முனை) &&
               "Expected an internal node?");
        Active.Idx += SubstringLen;
        Active.Len -= SubstringLen;
        Active.முனை = cast<Suffix_மரம்_Internal_முனை>(Next_முனை);
        தொடர்;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      unsigned LastChar = Str[EndIdx];

      // Is the string we're trying to insert a substring of the next node?
      எனில் (Str[Next_முனை->getStartIdx() + Active.Len] == LastChar) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        எனில் (NeedsLink && !Active.முனை->is_வேர்()) {
          NeedsLink->setLink(Active.முனை);
          NeedsLink = nullptr;
        }

        Active.Len++;
        நிறுத்து;
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
      Suffix_மரம்_Internal_முனை *Split_முனை = செருகு_Internal_முனை(
          Active.முனை, Next_முனை->getStartIdx(),
          Next_முனை->getStartIdx() + Active.Len - 1, FirstChar);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      செருகு_Leaf(*Split_முனை, EndIdx, LastChar);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      Next_முனை->incrementStartIdx(Active.Len);
      Split_முனை->Children[Str[Next_முனை->getStartIdx()]] = Next_முனை;

      // SplitNode is an internal node, update the suffix link.
      எனில் (NeedsLink)
        NeedsLink->setLink(Split_முனை);

      NeedsLink = Split_முனை;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    SuffixesToAdd--;

    எனில் (Active.முனை->is_வேர்()) {
      எனில் (Active.Len > 0) {
        Active.Len--;
        Active.Idx = EndIdx - SuffixesToAdd + 1;
      }
    } இல்லையெனில் {
      // Start the next phase at the next smallest suffix.
      Active.முனை = Active.முனை->getLink();
    }
  }

  திரும்பு SuffixesToAdd;
}

வெறுமை Suffix_மரம்::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = nullptr;

  // Each leaf node represents a repeat of a string.
  SmallVector<unsigned> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  போது (!InternalNodesToVisit.empty()) {
    RepeatedSubstringStarts.clear();
    auto *Curr = InternalNodesToVisit.எடு_back_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    unsigned Length = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    க்கு (auto &ChildPair : Curr->Children)
      // Save all of this node's children for processing.
      எனில் (auto *InternalChild =
              dyn_cast<Suffix_மரம்_Internal_முனை>(ChildPair.second))
        InternalNodesToVisit.தள்ளு_back(InternalChild);

    // If length of repeated substring is below threshold, then skip it.
    எனில் (Length < MinLength)
      தொடர்;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    எனில் (Curr->is_வேர்())
      தொடர்;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    எனில் (OutlinerLeafDescendants) {
      க்கு (unsigned I = Curr->get_இடது_Leaf_Idx(); I <= Curr->get_வலது_Leaf_Idx();
           ++I)
        RepeatedSubstringStarts.தள்ளு_back(LeafNodes[I]->getSuffixIdx());
    } இல்லையெனில் {
      க்கு (auto &ChildPair : Curr->Children)
        எனில் (auto *Leaf = dyn_cast<Suffix_மரம்_Leaf_முனை>(ChildPair.second))
          RepeatedSubstringStarts.தள்ளு_back(Leaf->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    எனில் (RepeatedSubstringStarts.size() < 2)
      தொடர்;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.Length = Length;
    llvm::append_range(RS.StartIndices, RepeatedSubstringStarts);
    நிறுத்து;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
