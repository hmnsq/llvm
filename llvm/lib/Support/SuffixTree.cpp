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

#അന്തര്ഭാവ "llvm/Support/SuffixTree.h"
#അന്തര്ഭാവ "llvm/Support/Allocator.h"
#അന്തര്ഭാവ "llvm/Support/Casting.h"
#അന്തര്ഭാവ "llvm/Support/SuffixTreeNode.h"

using നാമാകാശ llvm;

/// \returns the number of elements in the substring associated with \p N.
സ്ഥിതം size_t അക്കം_Elements_In_Substring(സ്ഥിരം Suffix_വൃക്ഷം_കണ്ണി *N) {
  പ്രതിജ്ഞാ(N && "Got a null node?");
  എങ്കിൽ (auto *Internal = dyn_cast<Suffix_വൃക്ഷം_Internal_കണ്ണി>(N))
    എങ്കിൽ (Internal->is_വേര്())
      തിരികെ 0;
  തിരികെ N->getEndIdx() - N->getStartIdx() + 1;
}

Suffix_വൃക്ഷം::Suffix_വൃക്ഷം(സ്ഥിരം ArrayRef<unsigned> &Str,
                       തർക്കം OutlinerLeafDescendants)
    : Str(Str), OutlinerLeafDescendants(OutlinerLeafDescendants) {
  വേര്_2 = തിരുകുക_വേര്();
  Active.കണ്ണി = വേര്_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  unsigned SuffixesToAdd = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  വേണ്ടി (unsigned PfxEndIdx = 0, End = Str.size(); PfxEndIdx < End; PfxEndIdx++) {
    SuffixesToAdd++;
    LeafEndIdx = PfxEndIdx; // Extend each of the leaves.
    SuffixesToAdd = extend(PfxEndIdx, SuffixesToAdd);
  }

  // Set the suffix indices of each leaf.
  പ്രതിജ്ഞാ(വേര്_2 && "Root node can't be nullptr!");
  setSuffixIndices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  എങ്കിൽ (OutlinerLeafDescendants)
    setLeafNodes();
}

Suffix_വൃക്ഷം_കണ്ണി *Suffix_വൃക്ഷം::തിരുകുക_Leaf(Suffix_വൃക്ഷം_Internal_കണ്ണി &Parent,
                                       unsigned StartIdx, unsigned Edge) {
  പ്രതിജ്ഞാ(StartIdx <= LeafEndIdx && "String can't start after it ends!");
  auto *N = പുതിയ (Leaf_കണ്ണി_Allocator.Allocate())
      Suffix_വൃക്ഷം_Leaf_കണ്ണി(StartIdx, &LeafEndIdx);
  Parent.Children[Edge] = N;
  തിരികെ N;
}

Suffix_വൃക്ഷം_Internal_കണ്ണി *
Suffix_വൃക്ഷം::തിരുകുക_Internal_കണ്ണി(Suffix_വൃക്ഷം_Internal_കണ്ണി *Parent,
                               unsigned StartIdx, unsigned EndIdx,
                               unsigned Edge) {
  പ്രതിജ്ഞാ(StartIdx <= EndIdx && "String can't start after it ends!");
  പ്രതിജ്ഞാ(!(!Parent && StartIdx != Suffix_വൃക്ഷം_കണ്ണി::EmptyIdx) &&
         "Non-root internal nodes must have parents!");
  auto *N = പുതിയ (Internal_കണ്ണി_Allocator.Allocate())
      Suffix_വൃക്ഷം_Internal_കണ്ണി(StartIdx, EndIdx, വേര്_2);
  എങ്കിൽ (Parent)
    Parent->Children[Edge] = N;
  തിരികെ N;
}

Suffix_വൃക്ഷം_Internal_കണ്ണി *Suffix_വൃക്ഷം::തിരുകുക_വേര്() {
  തിരികെ തിരുകുക_Internal_കണ്ണി(/*Parent = */ nullptr, Suffix_വൃക്ഷം_കണ്ണി::EmptyIdx,
                            Suffix_വൃക്ഷം_കണ്ണി::EmptyIdx, /*Edge = */ 0);
}

ശൂന്യം Suffix_വൃക്ഷം::setSuffixIndices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  SmallVector<std::pair<Suffix_വൃക്ഷം_കണ്ണി *, unsigned>> ToVisit;

  // Current node being visited.
  Suffix_വൃക്ഷം_കണ്ണി *Curr_കണ്ണി = വേര്_2;

  // Sum of the lengths of the nodes down the path to the current one.
  unsigned Curr_കണ്ണി_Len = 0;
  ToVisit.തള്ളുക_back({Curr_കണ്ണി, Curr_കണ്ണി_Len});
  ആകുമ്പോൾ (!ToVisit.empty()) {
    std::tie(Curr_കണ്ണി, Curr_കണ്ണി_Len) = ToVisit.എടുക്കുക_back_val();
    // Length of the current node from the root down to here.
    Curr_കണ്ണി->setConcatLen(Curr_കണ്ണി_Len);
    എങ്കിൽ (auto *Internal_കണ്ണി = dyn_cast<Suffix_വൃക്ഷം_Internal_കണ്ണി>(Curr_കണ്ണി))
      വേണ്ടി (auto &ChildPair : Internal_കണ്ണി->Children) {
        പ്രതിജ്ഞാ(ChildPair.second && "Node had a null child!");
        ToVisit.തള്ളുക_back(
            {ChildPair.second,
             Curr_കണ്ണി_Len + അക്കം_Elements_In_Substring(ChildPair.second)});
      }
    // No children, so we are at the end of the string.
    എങ്കിൽ (auto *Leaf_കണ്ണി = dyn_cast<Suffix_വൃക്ഷം_Leaf_കണ്ണി>(Curr_കണ്ണി))
      Leaf_കണ്ണി->setSuffixIdx(Str.size() - Curr_കണ്ണി_Len);
  }
}

ശൂന്യം Suffix_വൃക്ഷം::setLeafNodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  SmallVector<Suffix_വൃക്ഷം_കണ്ണി *> ToVisit;
  ToVisit.തള്ളുക_back(വേര്_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  unsigned LeafCounter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_വൃക്ഷം_Internal_കണ്ണി *,
           std::pair<Suffix_വൃക്ഷം_കണ്ണി *, Suffix_വൃക്ഷം_കണ്ണി *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  ആകുമ്പോൾ (!ToVisit.empty()) {
    Suffix_വൃക്ഷം_കണ്ണി *Curr_കണ്ണി = ToVisit.എടുക്കുക_back_val();
    എങ്കിൽ (auto *Curr_Internal_കണ്ണി = dyn_cast<Suffix_വൃക്ഷം_Internal_കണ്ണി>(Curr_കണ്ണി)) {
      // The current node is an internal node.
      auto I = ChildrenMap.find(Curr_Internal_കണ്ണി);
      എങ്കിൽ (I == ChildrenMap.end()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        auto J = Curr_Internal_കണ്ണി->Children.begin();
        എങ്കിൽ (J != Curr_Internal_കണ്ണി->Children.end()) {
          ToVisit.തള്ളുക_back(Curr_കണ്ണി);
          Suffix_വൃക്ഷം_കണ്ണി *FirstChild = J->second;
          Suffix_വൃക്ഷം_കണ്ണി *LastChild = nullptr;
          വേണ്ടി (; J != Curr_Internal_കണ്ണി->Children.end(); ++J) {
            LastChild = J->second;
            ToVisit.തള്ളുക_back(LastChild);
          }
          ChildrenMap[Curr_Internal_കണ്ണി] = {FirstChild, LastChild};
        }
      } അല്ലെങ്കിൽ {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        auto [FirstChild, LastChild] = I->second;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_കണ്ണി->set_വലത്_Leaf_Idx(FirstChild->get_വലത്_Leaf_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_കണ്ണി->set_ഇടത്_Leaf_Idx(LastChild->get_ഇടത്_Leaf_Idx());
        പ്രതിജ്ഞാ(Curr_കണ്ണി->get_ഇടത്_Leaf_Idx() <= Curr_കണ്ണി->get_വലത്_Leaf_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } അല്ലെങ്കിൽ {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_കണ്ണി->set_ഇടത്_Leaf_Idx(LeafCounter);
      Curr_കണ്ണി->set_വലത്_Leaf_Idx(LeafCounter);
      ++LeafCounter;
      auto *Curr_Leaf_കണ്ണി = cast<Suffix_വൃക്ഷം_Leaf_കണ്ണി>(Curr_കണ്ണി);
      LeafNodes.തള്ളുക_back(Curr_Leaf_കണ്ണി);
    }
  }
}

unsigned Suffix_വൃക്ഷം::extend(unsigned EndIdx, unsigned SuffixesToAdd) {
  Suffix_വൃക്ഷം_Internal_കണ്ണി *NeedsLink = nullptr;

  ആകുമ്പോൾ (SuffixesToAdd > 0) {

    // Are we waiting to add anything other than just the last character?
    എങ്കിൽ (Active.Len == 0) {
      // If not, then say the active index is the end index.
      Active.Idx = EndIdx;
    }

    പ്രതിജ്ഞാ(Active.Idx <= EndIdx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    unsigned FirstChar = Str[Active.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    എങ്കിൽ (auto It = Active.കണ്ണി->Children.find(FirstChar);
        It == Active.കണ്ണി->Children.end()) {
      // If not, then we can just insert a leaf and move to the next step.
      തിരുകുക_Leaf(*Active.കണ്ണി, EndIdx, FirstChar);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      എങ്കിൽ (NeedsLink) {
        NeedsLink->setLink(Active.കണ്ണി);
        NeedsLink = nullptr;
      }
    } അല്ലെങ്കിൽ {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_വൃക്ഷം_കണ്ണി *Next_കണ്ണി = It->second;

      unsigned SubstringLen = അക്കം_Elements_In_Substring(Next_കണ്ണി);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      എങ്കിൽ (Active.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        പ്രതിജ്ഞാ(isa<Suffix_വൃക്ഷം_Internal_കണ്ണി>(Next_കണ്ണി) &&
               "Expected an internal node?");
        Active.Idx += SubstringLen;
        Active.Len -= SubstringLen;
        Active.കണ്ണി = cast<Suffix_വൃക്ഷം_Internal_കണ്ണി>(Next_കണ്ണി);
        തുടരുക;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      unsigned LastChar = Str[EndIdx];

      // Is the string we're trying to insert a substring of the next node?
      എങ്കിൽ (Str[Next_കണ്ണി->getStartIdx() + Active.Len] == LastChar) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        എങ്കിൽ (NeedsLink && !Active.കണ്ണി->is_വേര്()) {
          NeedsLink->setLink(Active.കണ്ണി);
          NeedsLink = nullptr;
        }

        Active.Len++;
        നിർത്തുക;
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
      Suffix_വൃക്ഷം_Internal_കണ്ണി *Split_കണ്ണി = തിരുകുക_Internal_കണ്ണി(
          Active.കണ്ണി, Next_കണ്ണി->getStartIdx(),
          Next_കണ്ണി->getStartIdx() + Active.Len - 1, FirstChar);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      തിരുകുക_Leaf(*Split_കണ്ണി, EndIdx, LastChar);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      Next_കണ്ണി->incrementStartIdx(Active.Len);
      Split_കണ്ണി->Children[Str[Next_കണ്ണി->getStartIdx()]] = Next_കണ്ണി;

      // SplitNode is an internal node, update the suffix link.
      എങ്കിൽ (NeedsLink)
        NeedsLink->setLink(Split_കണ്ണി);

      NeedsLink = Split_കണ്ണി;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    SuffixesToAdd--;

    എങ്കിൽ (Active.കണ്ണി->is_വേര്()) {
      എങ്കിൽ (Active.Len > 0) {
        Active.Len--;
        Active.Idx = EndIdx - SuffixesToAdd + 1;
      }
    } അല്ലെങ്കിൽ {
      // Start the next phase at the next smallest suffix.
      Active.കണ്ണി = Active.കണ്ണി->getLink();
    }
  }

  തിരികെ SuffixesToAdd;
}

ശൂന്യം Suffix_വൃക്ഷം::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = nullptr;

  // Each leaf node represents a repeat of a string.
  SmallVector<unsigned> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  ആകുമ്പോൾ (!InternalNodesToVisit.empty()) {
    RepeatedSubstringStarts.clear();
    auto *Curr = InternalNodesToVisit.എടുക്കുക_back_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    unsigned Length = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    വേണ്ടി (auto &ChildPair : Curr->Children)
      // Save all of this node's children for processing.
      എങ്കിൽ (auto *InternalChild =
              dyn_cast<Suffix_വൃക്ഷം_Internal_കണ്ണി>(ChildPair.second))
        InternalNodesToVisit.തള്ളുക_back(InternalChild);

    // If length of repeated substring is below threshold, then skip it.
    എങ്കിൽ (Length < MinLength)
      തുടരുക;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    എങ്കിൽ (Curr->is_വേര്())
      തുടരുക;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    എങ്കിൽ (OutlinerLeafDescendants) {
      വേണ്ടി (unsigned I = Curr->get_ഇടത്_Leaf_Idx(); I <= Curr->get_വലത്_Leaf_Idx();
           ++I)
        RepeatedSubstringStarts.തള്ളുക_back(LeafNodes[I]->getSuffixIdx());
    } അല്ലെങ്കിൽ {
      വേണ്ടി (auto &ChildPair : Curr->Children)
        എങ്കിൽ (auto *Leaf = dyn_cast<Suffix_വൃക്ഷം_Leaf_കണ്ണി>(ChildPair.second))
          RepeatedSubstringStarts.തള്ളുക_back(Leaf->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    എങ്കിൽ (RepeatedSubstringStarts.size() < 2)
      തുടരുക;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.Length = Length;
    llvm::append_range(RS.StartIndices, RepeatedSubstringStarts);
    നിർത്തുക;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
