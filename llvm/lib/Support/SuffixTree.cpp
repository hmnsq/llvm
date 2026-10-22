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
स्थिर माप_प्रकार अंक_Elements_In_Substring(स्थिरांक Suffix_वृक्ष_गाठ *N) {
  प्रतिज्ञा(N && "Got a null node?");
  जर (स्वतः *Internal = dyn_cast<Suffix_वृक्ष_Internal_गाठ>(N))
    जर (Internal->is_मूळ())
      परत 0;
  परत N->getEndIdx() - N->getStartIdx() + 1;
}

Suffix_वृक्ष::Suffix_वृक्ष(स्थिरांक ArrayRef<अचिन्हित> &Str,
                       तार्किक OutlinerLeafDescendants)
    : Str(Str), OutlinerLeafDescendants(OutlinerLeafDescendants) {
  मूळ_2 = घाला_मूळ();
  Active.गाठ = मूळ_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  अचिन्हित SuffixesToAdd = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  साठी (अचिन्हित PfxEndIdx = 0, End = Str.size(); PfxEndIdx < End; PfxEndIdx++) {
    SuffixesToAdd++;
    LeafEndIdx = PfxEndIdx; // Extend each of the leaves.
    SuffixesToAdd = extend(PfxEndIdx, SuffixesToAdd);
  }

  // Set the suffix indices of each leaf.
  प्रतिज्ञा(मूळ_2 && "Root node can't be nullptr!");
  setSuffixIndices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  जर (OutlinerLeafDescendants)
    setLeafNodes();
}

Suffix_वृक्ष_गाठ *Suffix_वृक्ष::घाला_Leaf(Suffix_वृक्ष_Internal_गाठ &Parent,
                                       अचिन्हित StartIdx, अचिन्हित Edge) {
  प्रतिज्ञा(StartIdx <= LeafEndIdx && "String can't start after it ends!");
  स्वतः *N = नवीन (Leaf_गाठ_Allocator.Allocate())
      Suffix_वृक्ष_Leaf_गाठ(StartIdx, &LeafEndIdx);
  Parent.Children[Edge] = N;
  परत N;
}

Suffix_वृक्ष_Internal_गाठ *
Suffix_वृक्ष::घाला_Internal_गाठ(Suffix_वृक्ष_Internal_गाठ *Parent,
                               अचिन्हित StartIdx, अचिन्हित EndIdx,
                               अचिन्हित Edge) {
  प्रतिज्ञा(StartIdx <= EndIdx && "String can't start after it ends!");
  प्रतिज्ञा(!(!Parent && StartIdx != Suffix_वृक्ष_गाठ::EmptyIdx) &&
         "Non-root internal nodes must have parents!");
  स्वतः *N = नवीन (Internal_गाठ_Allocator.Allocate())
      Suffix_वृक्ष_Internal_गाठ(StartIdx, EndIdx, मूळ_2);
  जर (Parent)
    Parent->Children[Edge] = N;
  परत N;
}

Suffix_वृक्ष_Internal_गाठ *Suffix_वृक्ष::घाला_मूळ() {
  परत घाला_Internal_गाठ(/*Parent = */ nullptr, Suffix_वृक्ष_गाठ::EmptyIdx,
                            Suffix_वृक्ष_गाठ::EmptyIdx, /*Edge = */ 0);
}

शून्य Suffix_वृक्ष::setSuffixIndices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  SmallVector<std::pair<Suffix_वृक्ष_गाठ *, अचिन्हित>> ToVisit;

  // Current node being visited.
  Suffix_वृक्ष_गाठ *Curr_गाठ = मूळ_2;

  // Sum of the lengths of the nodes down the path to the current one.
  अचिन्हित Curr_गाठ_Len = 0;
  ToVisit.ढकला_back({Curr_गाठ, Curr_गाठ_Len});
  पर्यंत (!ToVisit.empty()) {
    std::tie(Curr_गाठ, Curr_गाठ_Len) = ToVisit.काढा_back_val();
    // Length of the current node from the root down to here.
    Curr_गाठ->setConcatLen(Curr_गाठ_Len);
    जर (स्वतः *Internal_गाठ = dyn_cast<Suffix_वृक्ष_Internal_गाठ>(Curr_गाठ))
      साठी (स्वतः &ChildPair : Internal_गाठ->Children) {
        प्रतिज्ञा(ChildPair.second && "Node had a null child!");
        ToVisit.ढकला_back(
            {ChildPair.second,
             Curr_गाठ_Len + अंक_Elements_In_Substring(ChildPair.second)});
      }
    // No children, so we are at the end of the string.
    जर (स्वतः *Leaf_गाठ = dyn_cast<Suffix_वृक्ष_Leaf_गाठ>(Curr_गाठ))
      Leaf_गाठ->setSuffixIdx(Str.size() - Curr_गाठ_Len);
  }
}

शून्य Suffix_वृक्ष::setLeafNodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  SmallVector<Suffix_वृक्ष_गाठ *> ToVisit;
  ToVisit.ढकला_back(मूळ_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  अचिन्हित LeafCounter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_वृक्ष_Internal_गाठ *,
           std::pair<Suffix_वृक्ष_गाठ *, Suffix_वृक्ष_गाठ *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  पर्यंत (!ToVisit.empty()) {
    Suffix_वृक्ष_गाठ *Curr_गाठ = ToVisit.काढा_back_val();
    जर (स्वतः *Curr_Internal_गाठ = dyn_cast<Suffix_वृक्ष_Internal_गाठ>(Curr_गाठ)) {
      // The current node is an internal node.
      स्वतः I = ChildrenMap.find(Curr_Internal_गाठ);
      जर (I == ChildrenMap.end()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        स्वतः J = Curr_Internal_गाठ->Children.begin();
        जर (J != Curr_Internal_गाठ->Children.end()) {
          ToVisit.ढकला_back(Curr_गाठ);
          Suffix_वृक्ष_गाठ *FirstChild = J->second;
          Suffix_वृक्ष_गाठ *LastChild = nullptr;
          साठी (; J != Curr_Internal_गाठ->Children.end(); ++J) {
            LastChild = J->second;
            ToVisit.ढकला_back(LastChild);
          }
          ChildrenMap[Curr_Internal_गाठ] = {FirstChild, LastChild};
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
        Curr_गाठ->set_उजवा_Leaf_Idx(FirstChild->get_उजवा_Leaf_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_गाठ->set_डावा_Leaf_Idx(LastChild->get_डावा_Leaf_Idx());
        प्रतिज्ञा(Curr_गाठ->get_डावा_Leaf_Idx() <= Curr_गाठ->get_उजवा_Leaf_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } अन्यथा {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_गाठ->set_डावा_Leaf_Idx(LeafCounter);
      Curr_गाठ->set_उजवा_Leaf_Idx(LeafCounter);
      ++LeafCounter;
      स्वतः *Curr_Leaf_गाठ = cast<Suffix_वृक्ष_Leaf_गाठ>(Curr_गाठ);
      LeafNodes.ढकला_back(Curr_Leaf_गाठ);
    }
  }
}

अचिन्हित Suffix_वृक्ष::extend(अचिन्हित EndIdx, अचिन्हित SuffixesToAdd) {
  Suffix_वृक्ष_Internal_गाठ *NeedsLink = nullptr;

  पर्यंत (SuffixesToAdd > 0) {

    // Are we waiting to add anything other than just the last character?
    जर (Active.Len == 0) {
      // If not, then say the active index is the end index.
      Active.Idx = EndIdx;
    }

    प्रतिज्ञा(Active.Idx <= EndIdx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    अचिन्हित FirstChar = Str[Active.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    जर (स्वतः It = Active.गाठ->Children.find(FirstChar);
        It == Active.गाठ->Children.end()) {
      // If not, then we can just insert a leaf and move to the next step.
      घाला_Leaf(*Active.गाठ, EndIdx, FirstChar);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      जर (NeedsLink) {
        NeedsLink->setLink(Active.गाठ);
        NeedsLink = nullptr;
      }
    } अन्यथा {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_वृक्ष_गाठ *Next_गाठ = It->second;

      अचिन्हित SubstringLen = अंक_Elements_In_Substring(Next_गाठ);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      जर (Active.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        प्रतिज्ञा(isa<Suffix_वृक्ष_Internal_गाठ>(Next_गाठ) &&
               "Expected an internal node?");
        Active.Idx += SubstringLen;
        Active.Len -= SubstringLen;
        Active.गाठ = cast<Suffix_वृक्ष_Internal_गाठ>(Next_गाठ);
        सुरूठेवा;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      अचिन्हित LastChar = Str[EndIdx];

      // Is the string we're trying to insert a substring of the next node?
      जर (Str[Next_गाठ->getStartIdx() + Active.Len] == LastChar) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        जर (NeedsLink && !Active.गाठ->is_मूळ()) {
          NeedsLink->setLink(Active.गाठ);
          NeedsLink = nullptr;
        }

        Active.Len++;
        थांबा;
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
      Suffix_वृक्ष_Internal_गाठ *Split_गाठ = घाला_Internal_गाठ(
          Active.गाठ, Next_गाठ->getStartIdx(),
          Next_गाठ->getStartIdx() + Active.Len - 1, FirstChar);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      घाला_Leaf(*Split_गाठ, EndIdx, LastChar);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      Next_गाठ->incrementStartIdx(Active.Len);
      Split_गाठ->Children[Str[Next_गाठ->getStartIdx()]] = Next_गाठ;

      // SplitNode is an internal node, update the suffix link.
      जर (NeedsLink)
        NeedsLink->setLink(Split_गाठ);

      NeedsLink = Split_गाठ;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    SuffixesToAdd--;

    जर (Active.गाठ->is_मूळ()) {
      जर (Active.Len > 0) {
        Active.Len--;
        Active.Idx = EndIdx - SuffixesToAdd + 1;
      }
    } अन्यथा {
      // Start the next phase at the next smallest suffix.
      Active.गाठ = Active.गाठ->getLink();
    }
  }

  परत SuffixesToAdd;
}

शून्य Suffix_वृक्ष::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = nullptr;

  // Each leaf node represents a repeat of a string.
  SmallVector<अचिन्हित> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  पर्यंत (!InternalNodesToVisit.empty()) {
    RepeatedSubstringStarts.clear();
    स्वतः *Curr = InternalNodesToVisit.काढा_back_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    अचिन्हित Length = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    साठी (स्वतः &ChildPair : Curr->Children)
      // Save all of this node's children for processing.
      जर (स्वतः *InternalChild =
              dyn_cast<Suffix_वृक्ष_Internal_गाठ>(ChildPair.second))
        InternalNodesToVisit.ढकला_back(InternalChild);

    // If length of repeated substring is below threshold, then skip it.
    जर (Length < MinLength)
      सुरूठेवा;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    जर (Curr->is_मूळ())
      सुरूठेवा;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    जर (OutlinerLeafDescendants) {
      साठी (अचिन्हित I = Curr->get_डावा_Leaf_Idx(); I <= Curr->get_उजवा_Leaf_Idx();
           ++I)
        RepeatedSubstringStarts.ढकला_back(LeafNodes[I]->getSuffixIdx());
    } अन्यथा {
      साठी (स्वतः &ChildPair : Curr->Children)
        जर (स्वतः *Leaf = dyn_cast<Suffix_वृक्ष_Leaf_गाठ>(ChildPair.second))
          RepeatedSubstringStarts.ढकला_back(Leaf->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    जर (RepeatedSubstringStarts.size() < 2)
      सुरूठेवा;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.Length = Length;
    llvm::append_range(RS.StartIndices, RepeatedSubstringStarts);
    थांबा;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
