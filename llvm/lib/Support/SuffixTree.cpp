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

#समावेश "llvm/Support/SuffixTree.h"
#समावेश "llvm/Support/Allocator.h"
#समावेश "llvm/Support/Casting.h"
#समावेश "llvm/Support/SuffixTreeNode.h"

using namespace llvm;

/// \returns the number of elements in the substring associated with \p N.
अटल माप_प्रकार अंक_Elements_In_Substring(स्थिर Suffix_वृक्ष_गाँठ *N) {
  निश्चित(N && "Got a null node?");
  अगर (स्वतः *Internal = dyn_cast<Suffix_वृक्ष_Internal_गाँठ>(N))
    अगर (Internal->is_मूल())
      वापस 0;
  वापस N->getEndIdx() - N->getStartIdx() + 1;
}

Suffix_वृक्ष::Suffix_वृक्ष(स्थिर ArrayRef<अचिन्हित> &Str,
                       bool OutlinerLeafDescendants)
    : Str(Str), OutlinerLeafDescendants(OutlinerLeafDescendants) {
  मूल_2 = जोड़ो_मूल();
  Active.गाँठ = मूल_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  अचिन्हित SuffixesToAdd = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  क्रम (अचिन्हित PfxEndIdx = 0, End = Str.आकार_2(); PfxEndIdx < End; PfxEndIdx++) {
    SuffixesToAdd++;
    LeafEndIdx = PfxEndIdx; // Extend each of the leaves.
    SuffixesToAdd = extend(PfxEndIdx, SuffixesToAdd);
  }

  // Set the suffix indices of each leaf.
  निश्चित(मूल_2 && "Root node can't be nullptr!");
  setSuffixIndices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  अगर (OutlinerLeafDescendants)
    setLeafNodes();
}

Suffix_वृक्ष_गाँठ *Suffix_वृक्ष::जोड़ो_Leaf(Suffix_वृक्ष_Internal_गाँठ &Parent,
                                       अचिन्हित StartIdx, अचिन्हित Edge) {
  निश्चित(StartIdx <= LeafEndIdx && "String can't start after it ends!");
  स्वतः *N = नया (Leaf_गाँठ_Allocator.Allocate())
      Suffix_वृक्ष_Leaf_गाँठ(StartIdx, &LeafEndIdx);
  Parent.Children[Edge] = N;
  वापस N;
}

Suffix_वृक्ष_Internal_गाँठ *
Suffix_वृक्ष::जोड़ो_Internal_गाँठ(Suffix_वृक्ष_Internal_गाँठ *Parent,
                               अचिन्हित StartIdx, अचिन्हित EndIdx,
                               अचिन्हित Edge) {
  निश्चित(StartIdx <= EndIdx && "String can't start after it ends!");
  निश्चित(!(!Parent && StartIdx != Suffix_वृक्ष_गाँठ::EmptyIdx) &&
         "Non-root internal nodes must have parents!");
  स्वतः *N = नया (Internal_गाँठ_Allocator.Allocate())
      Suffix_वृक्ष_Internal_गाँठ(StartIdx, EndIdx, मूल_2);
  अगर (Parent)
    Parent->Children[Edge] = N;
  वापस N;
}

Suffix_वृक्ष_Internal_गाँठ *Suffix_वृक्ष::जोड़ो_मूल() {
  वापस जोड़ो_Internal_गाँठ(/*Parent = */ nullptr, Suffix_वृक्ष_गाँठ::EmptyIdx,
                            Suffix_वृक्ष_गाँठ::EmptyIdx, /*Edge = */ 0);
}

व्योम Suffix_वृक्ष::setSuffixIndices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  SmallVector<std::pair<Suffix_वृक्ष_गाँठ *, अचिन्हित>> ToVisit;

  // Current node being visited.
  Suffix_वृक्ष_गाँठ *Curr_गाँठ = मूल_2;

  // Sum of the lengths of the nodes down the path to the current one.
  अचिन्हित Curr_गाँठ_Len = 0;
  ToVisit.धकेलो_back({Curr_गाँठ, Curr_गाँठ_Len});
  जबतक (!ToVisit.empty()) {
    std::tie(Curr_गाँठ, Curr_गाँठ_Len) = ToVisit.निकालो_back_val();
    // Length of the current node from the root down to here.
    Curr_गाँठ->setConcatLen(Curr_गाँठ_Len);
    अगर (स्वतः *Internal_गाँठ = dyn_cast<Suffix_वृक्ष_Internal_गाँठ>(Curr_गाँठ))
      क्रम (स्वतः &ChildPair : Internal_गाँठ->Children) {
        निश्चित(ChildPair.second && "Node had a null child!");
        ToVisit.धकेलो_back(
            {ChildPair.second,
             Curr_गाँठ_Len + अंक_Elements_In_Substring(ChildPair.second)});
      }
    // No children, so we are at the end of the string.
    अगर (स्वतः *Leaf_गाँठ = dyn_cast<Suffix_वृक्ष_Leaf_गाँठ>(Curr_गाँठ))
      Leaf_गाँठ->setSuffixIdx(Str.आकार_2() - Curr_गाँठ_Len);
  }
}

व्योम Suffix_वृक्ष::setLeafNodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  SmallVector<Suffix_वृक्ष_गाँठ *> ToVisit;
  ToVisit.धकेलो_back(मूल_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  अचिन्हित Leaf_गणक = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_वृक्ष_Internal_गाँठ *,
           std::pair<Suffix_वृक्ष_गाँठ *, Suffix_वृक्ष_गाँठ *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  जबतक (!ToVisit.empty()) {
    Suffix_वृक्ष_गाँठ *Curr_गाँठ = ToVisit.निकालो_back_val();
    अगर (स्वतः *Curr_Internal_गाँठ = dyn_cast<Suffix_वृक्ष_Internal_गाँठ>(Curr_गाँठ)) {
      // The current node is an internal node.
      स्वतः I = ChildrenMap.find(Curr_Internal_गाँठ);
      अगर (I == ChildrenMap.end()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        स्वतः J = Curr_Internal_गाँठ->Children.begin();
        अगर (J != Curr_Internal_गाँठ->Children.end()) {
          ToVisit.धकेलो_back(Curr_गाँठ);
          Suffix_वृक्ष_गाँठ *FirstChild = J->second;
          Suffix_वृक्ष_गाँठ *LastChild = nullptr;
          क्रम (; J != Curr_Internal_गाँठ->Children.end(); ++J) {
            LastChild = J->second;
            ToVisit.धकेलो_back(LastChild);
          }
          ChildrenMap[Curr_Internal_गाँठ] = {FirstChild, LastChild};
        }
      } अन्यथा {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        स्वतः [FirstChild, LastChild] = I->second;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_गाँठ->set_दायाँ_Leaf_Idx(FirstChild->get_दायाँ_Leaf_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_गाँठ->set_बायाँ_Leaf_Idx(LastChild->get_बायाँ_Leaf_Idx());
        निश्चित(Curr_गाँठ->get_बायाँ_Leaf_Idx() <= Curr_गाँठ->get_दायाँ_Leaf_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } अन्यथा {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_गाँठ->set_बायाँ_Leaf_Idx(Leaf_गणक);
      Curr_गाँठ->set_दायाँ_Leaf_Idx(Leaf_गणक);
      ++Leaf_गणक;
      स्वतः *Curr_Leaf_गाँठ = cast<Suffix_वृक्ष_Leaf_गाँठ>(Curr_गाँठ);
      LeafNodes.धकेलो_back(Curr_Leaf_गाँठ);
    }
  }
}

अचिन्हित Suffix_वृक्ष::extend(अचिन्हित EndIdx, अचिन्हित SuffixesToAdd) {
  Suffix_वृक्ष_Internal_गाँठ *NeedsLink = nullptr;

  जबतक (SuffixesToAdd > 0) {

    // Are we waiting to add anything other than just the last character?
    अगर (Active.Len == 0) {
      // If not, then say the active index is the end index.
      Active.Idx = EndIdx;
    }

    निश्चित(Active.Idx <= EndIdx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    अचिन्हित FirstChar = Str[Active.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    अगर (स्वतः It = Active.गाँठ->Children.find(FirstChar);
        It == Active.गाँठ->Children.end()) {
      // If not, then we can just insert a leaf and move to the next step.
      जोड़ो_Leaf(*Active.गाँठ, EndIdx, FirstChar);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      अगर (NeedsLink) {
        NeedsLink->setLink(Active.गाँठ);
        NeedsLink = nullptr;
      }
    } अन्यथा {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_वृक्ष_गाँठ *अग्र_गाँठ = It->second;

      अचिन्हित SubstringLen = अंक_Elements_In_Substring(अग्र_गाँठ);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      अगर (Active.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        निश्चित(isa<Suffix_वृक्ष_Internal_गाँठ>(अग्र_गाँठ) &&
               "Expected an internal node?");
        Active.Idx += SubstringLen;
        Active.Len -= SubstringLen;
        Active.गाँठ = cast<Suffix_वृक्ष_Internal_गाँठ>(अग्र_गाँठ);
        जारी;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      अचिन्हित LastChar = Str[EndIdx];

      // Is the string we're trying to insert a substring of the next node?
      अगर (Str[अग्र_गाँठ->getStartIdx() + Active.Len] == LastChar) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        अगर (NeedsLink && !Active.गाँठ->is_मूल()) {
          NeedsLink->setLink(Active.गाँठ);
          NeedsLink = nullptr;
        }

        Active.Len++;
        अवरोध;
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
          Active.गाँठ, अग्र_गाँठ->getStartIdx(),
          अग्र_गाँठ->getStartIdx() + Active.Len - 1, FirstChar);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      जोड़ो_Leaf(*Split_गाँठ, EndIdx, LastChar);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      अग्र_गाँठ->incrementStartIdx(Active.Len);
      Split_गाँठ->Children[Str[अग्र_गाँठ->getStartIdx()]] = अग्र_गाँठ;

      // SplitNode is an internal node, update the suffix link.
      अगर (NeedsLink)
        NeedsLink->setLink(Split_गाँठ);

      NeedsLink = Split_गाँठ;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    SuffixesToAdd--;

    अगर (Active.गाँठ->is_मूल()) {
      अगर (Active.Len > 0) {
        Active.Len--;
        Active.Idx = EndIdx - SuffixesToAdd + 1;
      }
    } अन्यथा {
      // Start the next phase at the next smallest suffix.
      Active.गाँठ = Active.गाँठ->getLink();
    }
  }

  वापस SuffixesToAdd;
}

व्योम Suffix_वृक्ष::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = nullptr;

  // Each leaf node represents a repeat of a string.
  SmallVector<अचिन्हित> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  जबतक (!InternalNodesToVisit.empty()) {
    RepeatedSubstringStarts.सफा();
    स्वतः *Curr = InternalNodesToVisit.निकालो_back_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    अचिन्हित लंबाई = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    क्रम (स्वतः &ChildPair : Curr->Children)
      // Save all of this node's children for processing.
      अगर (स्वतः *InternalChild =
              dyn_cast<Suffix_वृक्ष_Internal_गाँठ>(ChildPair.second))
        InternalNodesToVisit.धकेलो_back(InternalChild);

    // If length of repeated substring is below threshold, then skip it.
    अगर (लंबाई < Min_लंबाई)
      जारी;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    अगर (Curr->is_मूल())
      जारी;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    अगर (OutlinerLeafDescendants) {
      क्रम (अचिन्हित I = Curr->get_बायाँ_Leaf_Idx(); I <= Curr->get_दायाँ_Leaf_Idx();
           ++I)
        RepeatedSubstringStarts.धकेलो_back(LeafNodes[I]->getSuffixIdx());
    } अन्यथा {
      क्रम (स्वतः &ChildPair : Curr->Children)
        अगर (स्वतः *Leaf = dyn_cast<Suffix_वृक्ष_Leaf_गाँठ>(ChildPair.second))
          RepeatedSubstringStarts.धकेलो_back(Leaf->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    अगर (RepeatedSubstringStarts.आकार_2() < 2)
      जारी;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.लंबाई = लंबाई;
    llvm::append_range(RS.StartIndices, RepeatedSubstringStarts);
    अवरोध;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
