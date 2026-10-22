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

#અન્તર્ભાવ "llvm/Support/SuffixTree.h"
#અન્તર્ભાવ "llvm/Support/Allocator.h"
#અન્તર્ભાવ "llvm/Support/Casting.h"
#અન્તર્ભાવ "llvm/Support/SuffixTreeNode.h"

using નામાકાશ llvm;

/// \returns the number of elements in the substring associated with \p N.
સ્થિર size_t અંક_Elements_In_Substring(અચલ Suffix_વૃક્ષ_ગાંઠ *N) {
  પ્રતિજ્ઞા(N && "Got a null node?");
  જો (auto *Internal = dyn_cast<Suffix_વૃક્ષ_Internal_ગાંઠ>(N))
    જો (Internal->is_મૂળ())
      પાછા 0;
  પાછા N->getEndIdx() - N->getStartIdx() + 1;
}

Suffix_વૃક્ષ::Suffix_વૃક્ષ(અચલ ArrayRef<unsigned> &Str,
                       તાર્કિક OutlinerLeafDescendants)
    : Str(Str), OutlinerLeafDescendants(OutlinerLeafDescendants) {
  મૂળ_2 = ઉમેરો_મૂળ();
  Active.ગાંઠ = મૂળ_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  unsigned SuffixesToAdd = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  માટે (unsigned PfxEndIdx = 0, End = Str.size(); PfxEndIdx < End; PfxEndIdx++) {
    SuffixesToAdd++;
    LeafEndIdx = PfxEndIdx; // Extend each of the leaves.
    SuffixesToAdd = extend(PfxEndIdx, SuffixesToAdd);
  }

  // Set the suffix indices of each leaf.
  પ્રતિજ્ઞા(મૂળ_2 && "Root node can't be nullptr!");
  setSuffixIndices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  જો (OutlinerLeafDescendants)
    setLeafNodes();
}

Suffix_વૃક્ષ_ગાંઠ *Suffix_વૃક્ષ::ઉમેરો_Leaf(Suffix_વૃક્ષ_Internal_ગાંઠ &Parent,
                                       unsigned StartIdx, unsigned Edge) {
  પ્રતિજ્ઞા(StartIdx <= LeafEndIdx && "String can't start after it ends!");
  auto *N = નવું (Leaf_ગાંઠ_Allocator.Allocate())
      Suffix_વૃક્ષ_Leaf_ગાંઠ(StartIdx, &LeafEndIdx);
  Parent.Children[Edge] = N;
  પાછા N;
}

Suffix_વૃક્ષ_Internal_ગાંઠ *
Suffix_વૃક્ષ::ઉમેરો_Internal_ગાંઠ(Suffix_વૃક્ષ_Internal_ગાંઠ *Parent,
                               unsigned StartIdx, unsigned EndIdx,
                               unsigned Edge) {
  પ્રતિજ્ઞા(StartIdx <= EndIdx && "String can't start after it ends!");
  પ્રતિજ્ઞા(!(!Parent && StartIdx != Suffix_વૃક્ષ_ગાંઠ::EmptyIdx) &&
         "Non-root internal nodes must have parents!");
  auto *N = નવું (Internal_ગાંઠ_Allocator.Allocate())
      Suffix_વૃક્ષ_Internal_ગાંઠ(StartIdx, EndIdx, મૂળ_2);
  જો (Parent)
    Parent->Children[Edge] = N;
  પાછા N;
}

Suffix_વૃક્ષ_Internal_ગાંઠ *Suffix_વૃક્ષ::ઉમેરો_મૂળ() {
  પાછા ઉમેરો_Internal_ગાંઠ(/*Parent = */ nullptr, Suffix_વૃક્ષ_ગાંઠ::EmptyIdx,
                            Suffix_વૃક્ષ_ગાંઠ::EmptyIdx, /*Edge = */ 0);
}

શૂન્ય Suffix_વૃક્ષ::setSuffixIndices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  SmallVector<std::pair<Suffix_વૃક્ષ_ગાંઠ *, unsigned>> ToVisit;

  // Current node being visited.
  Suffix_વૃક્ષ_ગાંઠ *Curr_ગાંઠ = મૂળ_2;

  // Sum of the lengths of the nodes down the path to the current one.
  unsigned Curr_ગાંઠ_Len = 0;
  ToVisit.ધકેલો_back({Curr_ગાંઠ, Curr_ગાંઠ_Len});
  જ્યારે (!ToVisit.empty()) {
    std::tie(Curr_ગાંઠ, Curr_ગાંઠ_Len) = ToVisit.કાઢો_back_val();
    // Length of the current node from the root down to here.
    Curr_ગાંઠ->setConcatLen(Curr_ગાંઠ_Len);
    જો (auto *Internal_ગાંઠ = dyn_cast<Suffix_વૃક્ષ_Internal_ગાંઠ>(Curr_ગાંઠ))
      માટે (auto &ChildPair : Internal_ગાંઠ->Children) {
        પ્રતિજ્ઞા(ChildPair.second && "Node had a null child!");
        ToVisit.ધકેલો_back(
            {ChildPair.second,
             Curr_ગાંઠ_Len + અંક_Elements_In_Substring(ChildPair.second)});
      }
    // No children, so we are at the end of the string.
    જો (auto *Leaf_ગાંઠ = dyn_cast<Suffix_વૃક્ષ_Leaf_ગાંઠ>(Curr_ગાંઠ))
      Leaf_ગાંઠ->setSuffixIdx(Str.size() - Curr_ગાંઠ_Len);
  }
}

શૂન્ય Suffix_વૃક્ષ::setLeafNodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  SmallVector<Suffix_વૃક્ષ_ગાંઠ *> ToVisit;
  ToVisit.ધકેલો_back(મૂળ_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  unsigned LeafCounter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_વૃક્ષ_Internal_ગાંઠ *,
           std::pair<Suffix_વૃક્ષ_ગાંઠ *, Suffix_વૃક્ષ_ગાંઠ *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  જ્યારે (!ToVisit.empty()) {
    Suffix_વૃક્ષ_ગાંઠ *Curr_ગાંઠ = ToVisit.કાઢો_back_val();
    જો (auto *Curr_Internal_ગાંઠ = dyn_cast<Suffix_વૃક્ષ_Internal_ગાંઠ>(Curr_ગાંઠ)) {
      // The current node is an internal node.
      auto I = ChildrenMap.find(Curr_Internal_ગાંઠ);
      જો (I == ChildrenMap.end()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        auto J = Curr_Internal_ગાંઠ->Children.begin();
        જો (J != Curr_Internal_ગાંઠ->Children.end()) {
          ToVisit.ધકેલો_back(Curr_ગાંઠ);
          Suffix_વૃક્ષ_ગાંઠ *FirstChild = J->second;
          Suffix_વૃક્ષ_ગાંઠ *LastChild = nullptr;
          માટે (; J != Curr_Internal_ગાંઠ->Children.end(); ++J) {
            LastChild = J->second;
            ToVisit.ધકેલો_back(LastChild);
          }
          ChildrenMap[Curr_Internal_ગાંઠ] = {FirstChild, LastChild};
        }
      } નહીંતર {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        auto [FirstChild, LastChild] = I->second;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_ગાંઠ->set_જમણું_Leaf_Idx(FirstChild->get_જમણું_Leaf_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_ગાંઠ->set_ડાબું_Leaf_Idx(LastChild->get_ડાબું_Leaf_Idx());
        પ્રતિજ્ઞા(Curr_ગાંઠ->get_ડાબું_Leaf_Idx() <= Curr_ગાંઠ->get_જમણું_Leaf_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } નહીંતર {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_ગાંઠ->set_ડાબું_Leaf_Idx(LeafCounter);
      Curr_ગાંઠ->set_જમણું_Leaf_Idx(LeafCounter);
      ++LeafCounter;
      auto *Curr_Leaf_ગાંઠ = cast<Suffix_વૃક્ષ_Leaf_ગાંઠ>(Curr_ગાંઠ);
      LeafNodes.ધકેલો_back(Curr_Leaf_ગાંઠ);
    }
  }
}

unsigned Suffix_વૃક્ષ::extend(unsigned EndIdx, unsigned SuffixesToAdd) {
  Suffix_વૃક્ષ_Internal_ગાંઠ *NeedsLink = nullptr;

  જ્યારે (SuffixesToAdd > 0) {

    // Are we waiting to add anything other than just the last character?
    જો (Active.Len == 0) {
      // If not, then say the active index is the end index.
      Active.Idx = EndIdx;
    }

    પ્રતિજ્ઞા(Active.Idx <= EndIdx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    unsigned FirstChar = Str[Active.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    જો (auto It = Active.ગાંઠ->Children.find(FirstChar);
        It == Active.ગાંઠ->Children.end()) {
      // If not, then we can just insert a leaf and move to the next step.
      ઉમેરો_Leaf(*Active.ગાંઠ, EndIdx, FirstChar);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      જો (NeedsLink) {
        NeedsLink->setLink(Active.ગાંઠ);
        NeedsLink = nullptr;
      }
    } નહીંતર {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_વૃક્ષ_ગાંઠ *Next_ગાંઠ = It->second;

      unsigned SubstringLen = અંક_Elements_In_Substring(Next_ગાંઠ);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      જો (Active.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        પ્રતિજ્ઞા(isa<Suffix_વૃક્ષ_Internal_ગાંઠ>(Next_ગાંઠ) &&
               "Expected an internal node?");
        Active.Idx += SubstringLen;
        Active.Len -= SubstringLen;
        Active.ગાંઠ = cast<Suffix_વૃક્ષ_Internal_ગાંઠ>(Next_ગાંઠ);
        ચાલુ;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      unsigned LastChar = Str[EndIdx];

      // Is the string we're trying to insert a substring of the next node?
      જો (Str[Next_ગાંઠ->getStartIdx() + Active.Len] == LastChar) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        જો (NeedsLink && !Active.ગાંઠ->is_મૂળ()) {
          NeedsLink->setLink(Active.ગાંઠ);
          NeedsLink = nullptr;
        }

        Active.Len++;
        તોડો;
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
      Suffix_વૃક્ષ_Internal_ગાંઠ *Split_ગાંઠ = ઉમેરો_Internal_ગાંઠ(
          Active.ગાંઠ, Next_ગાંઠ->getStartIdx(),
          Next_ગાંઠ->getStartIdx() + Active.Len - 1, FirstChar);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      ઉમેરો_Leaf(*Split_ગાંઠ, EndIdx, LastChar);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      Next_ગાંઠ->incrementStartIdx(Active.Len);
      Split_ગાંઠ->Children[Str[Next_ગાંઠ->getStartIdx()]] = Next_ગાંઠ;

      // SplitNode is an internal node, update the suffix link.
      જો (NeedsLink)
        NeedsLink->setLink(Split_ગાંઠ);

      NeedsLink = Split_ગાંઠ;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    SuffixesToAdd--;

    જો (Active.ગાંઠ->is_મૂળ()) {
      જો (Active.Len > 0) {
        Active.Len--;
        Active.Idx = EndIdx - SuffixesToAdd + 1;
      }
    } નહીંતર {
      // Start the next phase at the next smallest suffix.
      Active.ગાંઠ = Active.ગાંઠ->getLink();
    }
  }

  પાછા SuffixesToAdd;
}

શૂન્ય Suffix_વૃક્ષ::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = nullptr;

  // Each leaf node represents a repeat of a string.
  SmallVector<unsigned> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  જ્યારે (!InternalNodesToVisit.empty()) {
    RepeatedSubstringStarts.clear();
    auto *Curr = InternalNodesToVisit.કાઢો_back_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    unsigned Length = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    માટે (auto &ChildPair : Curr->Children)
      // Save all of this node's children for processing.
      જો (auto *InternalChild =
              dyn_cast<Suffix_વૃક્ષ_Internal_ગાંઠ>(ChildPair.second))
        InternalNodesToVisit.ધકેલો_back(InternalChild);

    // If length of repeated substring is below threshold, then skip it.
    જો (Length < MinLength)
      ચાલુ;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    જો (Curr->is_મૂળ())
      ચાલુ;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    જો (OutlinerLeafDescendants) {
      માટે (unsigned I = Curr->get_ડાબું_Leaf_Idx(); I <= Curr->get_જમણું_Leaf_Idx();
           ++I)
        RepeatedSubstringStarts.ધકેલો_back(LeafNodes[I]->getSuffixIdx());
    } નહીંતર {
      માટે (auto &ChildPair : Curr->Children)
        જો (auto *Leaf = dyn_cast<Suffix_વૃક્ષ_Leaf_ગાંઠ>(ChildPair.second))
          RepeatedSubstringStarts.ધકેલો_back(Leaf->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    જો (RepeatedSubstringStarts.size() < 2)
      ચાલુ;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.Length = Length;
    llvm::append_range(RS.StartIndices, RepeatedSubstringStarts);
    તોડો;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
