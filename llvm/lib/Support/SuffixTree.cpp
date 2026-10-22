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

#अन्तर्भाव "llvm/Support/SuffixTree.h"
#अन्तर्भाव "llvm/Support/Allocator.h"
#अन्तर्भाव "llvm/Support/Casting.h"
#अन्तर्भाव "llvm/Support/SuffixTreeNode.h"

using नामाकाश llvm;

/// \returns the number of elements in the substring associated with \p N.
स्थिर size_t अंक_Elements_In_Substring(ध्रुव Suffix_वृक्ष_गाँठ *N) {
  प्रतिज्ञा(N && "Got a null node?");
  यदि (auto *Internal = dyn_cast<Suffix_वृक्ष_Internal_गाँठ>(N))
    यदि (Internal->is_मूल())
      प्रत्यागम 0;
  प्रत्यागम N->getEndIdx() - N->getStartIdx() + 1;
}

Suffix_वृक्ष::Suffix_वृक्ष(ध्रुव ArrayRef<unsigned> &Str,
                       तार्किक OutlinerLeafDescendants)
    : Str(Str), OutlinerLeafDescendants(OutlinerLeafDescendants) {
  मूल_2 = जोड़ो_मूल();
  Active.गाँठ = मूल_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  unsigned SuffixesToAdd = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  प्रति (unsigned PfxEndIdx = 0, End = Str.size(); PfxEndIdx < End; PfxEndIdx++) {
    SuffixesToAdd++;
    LeafEndIdx = PfxEndIdx; // Extend each of the leaves.
    SuffixesToAdd = extend(PfxEndIdx, SuffixesToAdd);
  }

  // Set the suffix indices of each leaf.
  प्रतिज्ञा(मूल_2 && "Root node can't be nullptr!");
  setSuffixIndices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  यदि (OutlinerLeafDescendants)
    setLeafNodes();
}

Suffix_वृक्ष_गाँठ *Suffix_वृक्ष::जोड़ो_Leaf(Suffix_वृक्ष_Internal_गाँठ &Parent,
                                       unsigned StartIdx, unsigned Edge) {
  प्रतिज्ञा(StartIdx <= LeafEndIdx && "String can't start after it ends!");
  auto *N = नूतन (Leaf_गाँठ_Allocator.Allocate())
      Suffix_वृक्ष_Leaf_गाँठ(StartIdx, &LeafEndIdx);
  Parent.Children[Edge] = N;
  प्रत्यागम N;
}

Suffix_वृक्ष_Internal_गाँठ *
Suffix_वृक्ष::जोड़ो_Internal_गाँठ(Suffix_वृक्ष_Internal_गाँठ *Parent,
                               unsigned StartIdx, unsigned EndIdx,
                               unsigned Edge) {
  प्रतिज्ञा(StartIdx <= EndIdx && "String can't start after it ends!");
  प्रतिज्ञा(!(!Parent && StartIdx != Suffix_वृक्ष_गाँठ::EmptyIdx) &&
         "Non-root internal nodes must have parents!");
  auto *N = नूतन (Internal_गाँठ_Allocator.Allocate())
      Suffix_वृक्ष_Internal_गाँठ(StartIdx, EndIdx, मूल_2);
  यदि (Parent)
    Parent->Children[Edge] = N;
  प्रत्यागम N;
}

Suffix_वृक्ष_Internal_गाँठ *Suffix_वृक्ष::जोड़ो_मूल() {
  प्रत्यागम जोड़ो_Internal_गाँठ(/*Parent = */ nullptr, Suffix_वृक्ष_गाँठ::EmptyIdx,
                            Suffix_वृक्ष_गाँठ::EmptyIdx, /*Edge = */ 0);
}

शून्य Suffix_वृक्ष::setSuffixIndices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  SmallVector<std::pair<Suffix_वृक्ष_गाँठ *, unsigned>> ToVisit;

  // Current node being visited.
  Suffix_वृक्ष_गाँठ *Curr_गाँठ = मूल_2;

  // Sum of the lengths of the nodes down the path to the current one.
  unsigned Curr_गाँठ_Len = 0;
  ToVisit.धकेलो_back({Curr_गाँठ, Curr_गाँठ_Len});
  यावत् (!ToVisit.empty()) {
    std::tie(Curr_गाँठ, Curr_गाँठ_Len) = ToVisit.निकालो_back_val();
    // Length of the current node from the root down to here.
    Curr_गाँठ->setConcatLen(Curr_गाँठ_Len);
    यदि (auto *Internal_गाँठ = dyn_cast<Suffix_वृक्ष_Internal_गाँठ>(Curr_गाँठ))
      प्रति (auto &ChildPair : Internal_गाँठ->Children) {
        प्रतिज्ञा(ChildPair.second && "Node had a null child!");
        ToVisit.धकेलो_back(
            {ChildPair.second,
             Curr_गाँठ_Len + अंक_Elements_In_Substring(ChildPair.second)});
      }
    // No children, so we are at the end of the string.
    यदि (auto *Leaf_गाँठ = dyn_cast<Suffix_वृक्ष_Leaf_गाँठ>(Curr_गाँठ))
      Leaf_गाँठ->setSuffixIdx(Str.size() - Curr_गाँठ_Len);
  }
}

शून्य Suffix_वृक्ष::setLeafNodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  SmallVector<Suffix_वृक्ष_गाँठ *> ToVisit;
  ToVisit.धकेलो_back(मूल_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  unsigned LeafCounter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_वृक्ष_Internal_गाँठ *,
           std::pair<Suffix_वृक्ष_गाँठ *, Suffix_वृक्ष_गाँठ *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  यावत् (!ToVisit.empty()) {
    Suffix_वृक्ष_गाँठ *Curr_गाँठ = ToVisit.निकालो_back_val();
    यदि (auto *Curr_Internal_गाँठ = dyn_cast<Suffix_वृक्ष_Internal_गाँठ>(Curr_गाँठ)) {
      // The current node is an internal node.
      auto I = ChildrenMap.find(Curr_Internal_गाँठ);
      यदि (I == ChildrenMap.end()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        auto J = Curr_Internal_गाँठ->Children.begin();
        यदि (J != Curr_Internal_गाँठ->Children.end()) {
          ToVisit.धकेलो_back(Curr_गाँठ);
          Suffix_वृक्ष_गाँठ *FirstChild = J->second;
          Suffix_वृक्ष_गाँठ *LastChild = nullptr;
          प्रति (; J != Curr_Internal_गाँठ->Children.end(); ++J) {
            LastChild = J->second;
            ToVisit.धकेलो_back(LastChild);
          }
          ChildrenMap[Curr_Internal_गाँठ] = {FirstChild, LastChild};
        }
      } अन्यथा {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        auto [FirstChild, LastChild] = I->second;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_गाँठ->set_दायाँ_Leaf_Idx(FirstChild->get_दायाँ_Leaf_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_गाँठ->set_बायाँ_Leaf_Idx(LastChild->get_बायाँ_Leaf_Idx());
        प्रतिज्ञा(Curr_गाँठ->get_बायाँ_Leaf_Idx() <= Curr_गाँठ->get_दायाँ_Leaf_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } अन्यथा {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_गाँठ->set_बायाँ_Leaf_Idx(LeafCounter);
      Curr_गाँठ->set_दायाँ_Leaf_Idx(LeafCounter);
      ++LeafCounter;
      auto *Curr_Leaf_गाँठ = cast<Suffix_वृक्ष_Leaf_गाँठ>(Curr_गाँठ);
      LeafNodes.धकेलो_back(Curr_Leaf_गाँठ);
    }
  }
}

unsigned Suffix_वृक्ष::extend(unsigned EndIdx, unsigned SuffixesToAdd) {
  Suffix_वृक्ष_Internal_गाँठ *NeedsLink = nullptr;

  यावत् (SuffixesToAdd > 0) {

    // Are we waiting to add anything other than just the last character?
    यदि (Active.Len == 0) {
      // If not, then say the active index is the end index.
      Active.Idx = EndIdx;
    }

    प्रतिज्ञा(Active.Idx <= EndIdx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    unsigned FirstChar = Str[Active.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    यदि (auto It = Active.गाँठ->Children.find(FirstChar);
        It == Active.गाँठ->Children.end()) {
      // If not, then we can just insert a leaf and move to the next step.
      जोड़ो_Leaf(*Active.गाँठ, EndIdx, FirstChar);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      यदि (NeedsLink) {
        NeedsLink->setLink(Active.गाँठ);
        NeedsLink = nullptr;
      }
    } अन्यथा {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_वृक्ष_गाँठ *Next_गाँठ = It->second;

      unsigned SubstringLen = अंक_Elements_In_Substring(Next_गाँठ);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      यदि (Active.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        प्रतिज्ञा(isa<Suffix_वृक्ष_Internal_गाँठ>(Next_गाँठ) &&
               "Expected an internal node?");
        Active.Idx += SubstringLen;
        Active.Len -= SubstringLen;
        Active.गाँठ = cast<Suffix_वृक्ष_Internal_गाँठ>(Next_गाँठ);
        अनुवर्त;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      unsigned LastChar = Str[EndIdx];

      // Is the string we're trying to insert a substring of the next node?
      यदि (Str[Next_गाँठ->getStartIdx() + Active.Len] == LastChar) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        यदि (NeedsLink && !Active.गाँठ->is_मूल()) {
          NeedsLink->setLink(Active.गाँठ);
          NeedsLink = nullptr;
        }

        Active.Len++;
        भङ्ग;
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
      Suffix_वृक्ष_Internal_गाँठ *Split_गाँठ = जोड़ो_Internal_गाँठ(
          Active.गाँठ, Next_गाँठ->getStartIdx(),
          Next_गाँठ->getStartIdx() + Active.Len - 1, FirstChar);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      जोड़ो_Leaf(*Split_गाँठ, EndIdx, LastChar);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      Next_गाँठ->incrementStartIdx(Active.Len);
      Split_गाँठ->Children[Str[Next_गाँठ->getStartIdx()]] = Next_गाँठ;

      // SplitNode is an internal node, update the suffix link.
      यदि (NeedsLink)
        NeedsLink->setLink(Split_गाँठ);

      NeedsLink = Split_गाँठ;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    SuffixesToAdd--;

    यदि (Active.गाँठ->is_मूल()) {
      यदि (Active.Len > 0) {
        Active.Len--;
        Active.Idx = EndIdx - SuffixesToAdd + 1;
      }
    } अन्यथा {
      // Start the next phase at the next smallest suffix.
      Active.गाँठ = Active.गाँठ->getLink();
    }
  }

  प्रत्यागम SuffixesToAdd;
}

शून्य Suffix_वृक्ष::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = nullptr;

  // Each leaf node represents a repeat of a string.
  SmallVector<unsigned> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  यावत् (!InternalNodesToVisit.empty()) {
    RepeatedSubstringStarts.clear();
    auto *Curr = InternalNodesToVisit.निकालो_back_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    unsigned Length = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    प्रति (auto &ChildPair : Curr->Children)
      // Save all of this node's children for processing.
      यदि (auto *InternalChild =
              dyn_cast<Suffix_वृक्ष_Internal_गाँठ>(ChildPair.second))
        InternalNodesToVisit.धकेलो_back(InternalChild);

    // If length of repeated substring is below threshold, then skip it.
    यदि (Length < MinLength)
      अनुवर्त;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    यदि (Curr->is_मूल())
      अनुवर्त;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    यदि (OutlinerLeafDescendants) {
      प्रति (unsigned I = Curr->get_बायाँ_Leaf_Idx(); I <= Curr->get_दायाँ_Leaf_Idx();
           ++I)
        RepeatedSubstringStarts.धकेलो_back(LeafNodes[I]->getSuffixIdx());
    } अन्यथा {
      प्रति (auto &ChildPair : Curr->Children)
        यदि (auto *Leaf = dyn_cast<Suffix_वृक्ष_Leaf_गाँठ>(ChildPair.second))
          RepeatedSubstringStarts.धकेलो_back(Leaf->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    यदि (RepeatedSubstringStarts.size() < 2)
      अनुवर्त;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.Length = Length;
    llvm::append_range(RS.StartIndices, RepeatedSubstringStarts);
    भङ्ग;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
