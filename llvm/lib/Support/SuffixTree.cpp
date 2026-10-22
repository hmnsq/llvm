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
स्थिर माप_प्रकार अङ्क_Elements_In_Substring(अचर Suffix_रूख_गाँठो *N) {
  प्रतिज्ञा(N && "Got a null node?");
  यदि (स्वतः *Internal = dyn_cast<Suffix_रूख_Internal_गाँठो>(N))
    यदि (Internal->is_जरा())
      फर्काऊ 0;
  फर्काऊ N->getEndIdx() - N->getStartIdx() + 1;
}

Suffix_रूख::Suffix_रूख(अचर ArrayRef<अचिन्हित> &Str,
                       तार्किक OutlinerLeafDescendants)
    : Str(Str), OutlinerLeafDescendants(OutlinerLeafDescendants) {
  जरा_2 = घुसाउ_जरा();
  Active.गाँठो = जरा_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  अचिन्हित SuffixesToAdd = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  लागि (अचिन्हित PfxEndIdx = 0, End = Str.size(); PfxEndIdx < End; PfxEndIdx++) {
    SuffixesToAdd++;
    LeafEndIdx = PfxEndIdx; // Extend each of the leaves.
    SuffixesToAdd = extend(PfxEndIdx, SuffixesToAdd);
  }

  // Set the suffix indices of each leaf.
  प्रतिज्ञा(जरा_2 && "Root node can't be nullptr!");
  setSuffixIndices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  यदि (OutlinerLeafDescendants)
    setLeafNodes();
}

Suffix_रूख_गाँठो *Suffix_रूख::घुसाउ_Leaf(Suffix_रूख_Internal_गाँठो &Parent,
                                       अचिन्हित StartIdx, अचिन्हित Edge) {
  प्रतिज्ञा(StartIdx <= LeafEndIdx && "String can't start after it ends!");
  स्वतः *N = नूतन (Leaf_गाँठो_Allocator.Allocate())
      Suffix_रूख_Leaf_गाँठो(StartIdx, &LeafEndIdx);
  Parent.Children[Edge] = N;
  फर्काऊ N;
}

Suffix_रूख_Internal_गाँठो *
Suffix_रूख::घुसाउ_Internal_गाँठो(Suffix_रूख_Internal_गाँठो *Parent,
                               अचिन्हित StartIdx, अचिन्हित EndIdx,
                               अचिन्हित Edge) {
  प्रतिज्ञा(StartIdx <= EndIdx && "String can't start after it ends!");
  प्रतिज्ञा(!(!Parent && StartIdx != Suffix_रूख_गाँठो::EmptyIdx) &&
         "Non-root internal nodes must have parents!");
  स्वतः *N = नूतन (Internal_गाँठो_Allocator.Allocate())
      Suffix_रूख_Internal_गाँठो(StartIdx, EndIdx, जरा_2);
  यदि (Parent)
    Parent->Children[Edge] = N;
  फर्काऊ N;
}

Suffix_रूख_Internal_गाँठो *Suffix_रूख::घुसाउ_जरा() {
  फर्काऊ घुसाउ_Internal_गाँठो(/*Parent = */ nullptr, Suffix_रूख_गाँठो::EmptyIdx,
                            Suffix_रूख_गाँठो::EmptyIdx, /*Edge = */ 0);
}

शून्य Suffix_रूख::setSuffixIndices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  SmallVector<std::pair<Suffix_रूख_गाँठो *, अचिन्हित>> ToVisit;

  // Current node being visited.
  Suffix_रूख_गाँठो *Curr_गाँठो = जरा_2;

  // Sum of the lengths of the nodes down the path to the current one.
  अचिन्हित Curr_गाँठो_Len = 0;
  ToVisit.थप_back({Curr_गाँठो, Curr_गाँठो_Len});
  जबसम्म (!ToVisit.empty()) {
    std::tie(Curr_गाँठो, Curr_गाँठो_Len) = ToVisit.झिक_back_val();
    // Length of the current node from the root down to here.
    Curr_गाँठो->setConcatLen(Curr_गाँठो_Len);
    यदि (स्वतः *Internal_गाँठो = dyn_cast<Suffix_रूख_Internal_गाँठो>(Curr_गाँठो))
      लागि (स्वतः &ChildPair : Internal_गाँठो->Children) {
        प्रतिज्ञा(ChildPair.second && "Node had a null child!");
        ToVisit.थप_back(
            {ChildPair.second,
             Curr_गाँठो_Len + अङ्क_Elements_In_Substring(ChildPair.second)});
      }
    // No children, so we are at the end of the string.
    यदि (स्वतः *Leaf_गाँठो = dyn_cast<Suffix_रूख_Leaf_गाँठो>(Curr_गाँठो))
      Leaf_गाँठो->setSuffixIdx(Str.size() - Curr_गाँठो_Len);
  }
}

शून्य Suffix_रूख::setLeafNodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  SmallVector<Suffix_रूख_गाँठो *> ToVisit;
  ToVisit.थप_back(जरा_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  अचिन्हित LeafCounter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_रूख_Internal_गाँठो *,
           std::pair<Suffix_रूख_गाँठो *, Suffix_रूख_गाँठो *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  जबसम्म (!ToVisit.empty()) {
    Suffix_रूख_गाँठो *Curr_गाँठो = ToVisit.झिक_back_val();
    यदि (स्वतः *Curr_Internal_गाँठो = dyn_cast<Suffix_रूख_Internal_गाँठो>(Curr_गाँठो)) {
      // The current node is an internal node.
      स्वतः I = ChildrenMap.find(Curr_Internal_गाँठो);
      यदि (I == ChildrenMap.end()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        स्वतः J = Curr_Internal_गाँठो->Children.begin();
        यदि (J != Curr_Internal_गाँठो->Children.end()) {
          ToVisit.थप_back(Curr_गाँठो);
          Suffix_रूख_गाँठो *FirstChild = J->second;
          Suffix_रूख_गाँठो *LastChild = nullptr;
          लागि (; J != Curr_Internal_गाँठो->Children.end(); ++J) {
            LastChild = J->second;
            ToVisit.थप_back(LastChild);
          }
          ChildrenMap[Curr_Internal_गाँठो] = {FirstChild, LastChild};
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
        Curr_गाँठो->set_दायाँ_Leaf_Idx(FirstChild->get_दायाँ_Leaf_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_गाँठो->set_बायाँ_Leaf_Idx(LastChild->get_बायाँ_Leaf_Idx());
        प्रतिज्ञा(Curr_गाँठो->get_बायाँ_Leaf_Idx() <= Curr_गाँठो->get_दायाँ_Leaf_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } अन्यथा {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_गाँठो->set_बायाँ_Leaf_Idx(LeafCounter);
      Curr_गाँठो->set_दायाँ_Leaf_Idx(LeafCounter);
      ++LeafCounter;
      स्वतः *Curr_Leaf_गाँठो = cast<Suffix_रूख_Leaf_गाँठो>(Curr_गाँठो);
      LeafNodes.थप_back(Curr_Leaf_गाँठो);
    }
  }
}

अचिन्हित Suffix_रूख::extend(अचिन्हित EndIdx, अचिन्हित SuffixesToAdd) {
  Suffix_रूख_Internal_गाँठो *NeedsLink = nullptr;

  जबसम्म (SuffixesToAdd > 0) {

    // Are we waiting to add anything other than just the last character?
    यदि (Active.Len == 0) {
      // If not, then say the active index is the end index.
      Active.Idx = EndIdx;
    }

    प्रतिज्ञा(Active.Idx <= EndIdx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    अचिन्हित FirstChar = Str[Active.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    यदि (स्वतः It = Active.गाँठो->Children.find(FirstChar);
        It == Active.गाँठो->Children.end()) {
      // If not, then we can just insert a leaf and move to the next step.
      घुसाउ_Leaf(*Active.गाँठो, EndIdx, FirstChar);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      यदि (NeedsLink) {
        NeedsLink->setLink(Active.गाँठो);
        NeedsLink = nullptr;
      }
    } अन्यथा {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_रूख_गाँठो *Next_गाँठो = It->second;

      अचिन्हित SubstringLen = अङ्क_Elements_In_Substring(Next_गाँठो);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      यदि (Active.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        प्रतिज्ञा(isa<Suffix_रूख_Internal_गाँठो>(Next_गाँठो) &&
               "Expected an internal node?");
        Active.Idx += SubstringLen;
        Active.Len -= SubstringLen;
        Active.गाँठो = cast<Suffix_रूख_Internal_गाँठो>(Next_गाँठो);
        जारी;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      अचिन्हित LastChar = Str[EndIdx];

      // Is the string we're trying to insert a substring of the next node?
      यदि (Str[Next_गाँठो->getStartIdx() + Active.Len] == LastChar) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        यदि (NeedsLink && !Active.गाँठो->is_जरा()) {
          NeedsLink->setLink(Active.गाँठो);
          NeedsLink = nullptr;
        }

        Active.Len++;
        रोक;
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
      Suffix_रूख_Internal_गाँठो *Split_गाँठो = घुसाउ_Internal_गाँठो(
          Active.गाँठो, Next_गाँठो->getStartIdx(),
          Next_गाँठो->getStartIdx() + Active.Len - 1, FirstChar);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      घुसाउ_Leaf(*Split_गाँठो, EndIdx, LastChar);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      Next_गाँठो->incrementStartIdx(Active.Len);
      Split_गाँठो->Children[Str[Next_गाँठो->getStartIdx()]] = Next_गाँठो;

      // SplitNode is an internal node, update the suffix link.
      यदि (NeedsLink)
        NeedsLink->setLink(Split_गाँठो);

      NeedsLink = Split_गाँठो;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    SuffixesToAdd--;

    यदि (Active.गाँठो->is_जरा()) {
      यदि (Active.Len > 0) {
        Active.Len--;
        Active.Idx = EndIdx - SuffixesToAdd + 1;
      }
    } अन्यथा {
      // Start the next phase at the next smallest suffix.
      Active.गाँठो = Active.गाँठो->getLink();
    }
  }

  फर्काऊ SuffixesToAdd;
}

शून्य Suffix_रूख::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = nullptr;

  // Each leaf node represents a repeat of a string.
  SmallVector<अचिन्हित> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  जबसम्म (!InternalNodesToVisit.empty()) {
    RepeatedSubstringStarts.clear();
    स्वतः *Curr = InternalNodesToVisit.झिक_back_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    अचिन्हित Length = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    लागि (स्वतः &ChildPair : Curr->Children)
      // Save all of this node's children for processing.
      यदि (स्वतः *InternalChild =
              dyn_cast<Suffix_रूख_Internal_गाँठो>(ChildPair.second))
        InternalNodesToVisit.थप_back(InternalChild);

    // If length of repeated substring is below threshold, then skip it.
    यदि (Length < MinLength)
      जारी;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    यदि (Curr->is_जरा())
      जारी;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    यदि (OutlinerLeafDescendants) {
      लागि (अचिन्हित I = Curr->get_बायाँ_Leaf_Idx(); I <= Curr->get_दायाँ_Leaf_Idx();
           ++I)
        RepeatedSubstringStarts.थप_back(LeafNodes[I]->getSuffixIdx());
    } अन्यथा {
      लागि (स्वतः &ChildPair : Curr->Children)
        यदि (स्वतः *Leaf = dyn_cast<Suffix_रूख_Leaf_गाँठो>(ChildPair.second))
          RepeatedSubstringStarts.थप_back(Leaf->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    यदि (RepeatedSubstringStarts.size() < 2)
      जारी;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.Length = Length;
    llvm::append_range(RS.StartIndices, RepeatedSubstringStarts);
    रोक;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
