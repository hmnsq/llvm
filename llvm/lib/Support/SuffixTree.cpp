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

#ضمن "llvm/Support/SuffixTree.h"
#ضمن "llvm/Support/Allocator.h"
#ضمن "llvm/Support/Casting.h"
#ضمن "llvm/Support/SuffixTreeNode.h"

using نطاق llvm;

/// \returns the number of elements in the substring associated with \p N.
ساكن size_t رقم_Elements_In_Substring(ثابت Suffix_شجرة_عقدة *N) {
  أكد(N && "Got a null node?");
  إذا (auto *Internal = dyn_cast<Suffix_شجرة_Internal_عقدة>(N))
    إذا (Internal->is_جذر())
      إرجاع 0;
  إرجاع N->getEndIdx() - N->getStartIdx() + 1;
}

Suffix_شجرة::Suffix_شجرة(ثابت ArrayRef<unsigned> &Str,
                       منطقي OutlinerLeafDescendants)
    : Str(Str), OutlinerLeafDescendants(OutlinerLeafDescendants) {
  جذر_2 = أدرج_جذر();
  Active.عقدة = جذر_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  unsigned SuffixesToAdd = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  لكل (unsigned PfxEndIdx = 0, End = Str.size(); PfxEndIdx < End; PfxEndIdx++) {
    SuffixesToAdd++;
    LeafEndIdx = PfxEndIdx; // Extend each of the leaves.
    SuffixesToAdd = extend(PfxEndIdx, SuffixesToAdd);
  }

  // Set the suffix indices of each leaf.
  أكد(جذر_2 && "Root node can't be nullptr!");
  setSuffixIndices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  إذا (OutlinerLeafDescendants)
    setLeafNodes();
}

Suffix_شجرة_عقدة *Suffix_شجرة::أدرج_Leaf(Suffix_شجرة_Internal_عقدة &Parent,
                                       unsigned StartIdx, unsigned Edge) {
  أكد(StartIdx <= LeafEndIdx && "String can't start after it ends!");
  auto *N = جديد (Leaf_عقدة_Allocator.Allocate())
      Suffix_شجرة_Leaf_عقدة(StartIdx, &LeafEndIdx);
  Parent.Children[Edge] = N;
  إرجاع N;
}

Suffix_شجرة_Internal_عقدة *
Suffix_شجرة::أدرج_Internal_عقدة(Suffix_شجرة_Internal_عقدة *Parent,
                               unsigned StartIdx, unsigned EndIdx,
                               unsigned Edge) {
  أكد(StartIdx <= EndIdx && "String can't start after it ends!");
  أكد(!(!Parent && StartIdx != Suffix_شجرة_عقدة::EmptyIdx) &&
         "Non-root internal nodes must have parents!");
  auto *N = جديد (Internal_عقدة_Allocator.Allocate())
      Suffix_شجرة_Internal_عقدة(StartIdx, EndIdx, جذر_2);
  إذا (Parent)
    Parent->Children[Edge] = N;
  إرجاع N;
}

Suffix_شجرة_Internal_عقدة *Suffix_شجرة::أدرج_جذر() {
  إرجاع أدرج_Internal_عقدة(/*Parent = */ nullptr, Suffix_شجرة_عقدة::EmptyIdx,
                            Suffix_شجرة_عقدة::EmptyIdx, /*Edge = */ 0);
}

فراغ Suffix_شجرة::setSuffixIndices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  SmallVector<std::pair<Suffix_شجرة_عقدة *, unsigned>> ToVisit;

  // Current node being visited.
  Suffix_شجرة_عقدة *Curr_عقدة = جذر_2;

  // Sum of the lengths of the nodes down the path to the current one.
  unsigned Curr_عقدة_Len = 0;
  ToVisit.ادفع_back({Curr_عقدة, Curr_عقدة_Len});
  بينما (!ToVisit.empty()) {
    std::tie(Curr_عقدة, Curr_عقدة_Len) = ToVisit.اسحب_back_val();
    // Length of the current node from the root down to here.
    Curr_عقدة->setConcatLen(Curr_عقدة_Len);
    إذا (auto *Internal_عقدة = dyn_cast<Suffix_شجرة_Internal_عقدة>(Curr_عقدة))
      لكل (auto &ChildPair : Internal_عقدة->Children) {
        أكد(ChildPair.second && "Node had a null child!");
        ToVisit.ادفع_back(
            {ChildPair.second,
             Curr_عقدة_Len + رقم_Elements_In_Substring(ChildPair.second)});
      }
    // No children, so we are at the end of the string.
    إذا (auto *Leaf_عقدة = dyn_cast<Suffix_شجرة_Leaf_عقدة>(Curr_عقدة))
      Leaf_عقدة->setSuffixIdx(Str.size() - Curr_عقدة_Len);
  }
}

فراغ Suffix_شجرة::setLeafNodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  SmallVector<Suffix_شجرة_عقدة *> ToVisit;
  ToVisit.ادفع_back(جذر_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  unsigned LeafCounter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_شجرة_Internal_عقدة *,
           std::pair<Suffix_شجرة_عقدة *, Suffix_شجرة_عقدة *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  بينما (!ToVisit.empty()) {
    Suffix_شجرة_عقدة *Curr_عقدة = ToVisit.اسحب_back_val();
    إذا (auto *Curr_Internal_عقدة = dyn_cast<Suffix_شجرة_Internal_عقدة>(Curr_عقدة)) {
      // The current node is an internal node.
      auto I = ChildrenMap.find(Curr_Internal_عقدة);
      إذا (I == ChildrenMap.end()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        auto J = Curr_Internal_عقدة->Children.begin();
        إذا (J != Curr_Internal_عقدة->Children.end()) {
          ToVisit.ادفع_back(Curr_عقدة);
          Suffix_شجرة_عقدة *FirstChild = J->second;
          Suffix_شجرة_عقدة *LastChild = nullptr;
          لكل (; J != Curr_Internal_عقدة->Children.end(); ++J) {
            LastChild = J->second;
            ToVisit.ادفع_back(LastChild);
          }
          ChildrenMap[Curr_Internal_عقدة] = {FirstChild, LastChild};
        }
      } وإلا {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        auto [FirstChild, LastChild] = I->second;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_عقدة->set_يمين_Leaf_Idx(FirstChild->get_يمين_Leaf_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_عقدة->set_يسار_Leaf_Idx(LastChild->get_يسار_Leaf_Idx());
        أكد(Curr_عقدة->get_يسار_Leaf_Idx() <= Curr_عقدة->get_يمين_Leaf_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } وإلا {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_عقدة->set_يسار_Leaf_Idx(LeafCounter);
      Curr_عقدة->set_يمين_Leaf_Idx(LeafCounter);
      ++LeafCounter;
      auto *Curr_Leaf_عقدة = cast<Suffix_شجرة_Leaf_عقدة>(Curr_عقدة);
      LeafNodes.ادفع_back(Curr_Leaf_عقدة);
    }
  }
}

unsigned Suffix_شجرة::extend(unsigned EndIdx, unsigned SuffixesToAdd) {
  Suffix_شجرة_Internal_عقدة *NeedsLink = nullptr;

  بينما (SuffixesToAdd > 0) {

    // Are we waiting to add anything other than just the last character?
    إذا (Active.Len == 0) {
      // If not, then say the active index is the end index.
      Active.Idx = EndIdx;
    }

    أكد(Active.Idx <= EndIdx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    unsigned FirstChar = Str[Active.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    إذا (auto It = Active.عقدة->Children.find(FirstChar);
        It == Active.عقدة->Children.end()) {
      // If not, then we can just insert a leaf and move to the next step.
      أدرج_Leaf(*Active.عقدة, EndIdx, FirstChar);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      إذا (NeedsLink) {
        NeedsLink->setLink(Active.عقدة);
        NeedsLink = nullptr;
      }
    } وإلا {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_شجرة_عقدة *Next_عقدة = It->second;

      unsigned SubstringLen = رقم_Elements_In_Substring(Next_عقدة);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      إذا (Active.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        أكد(isa<Suffix_شجرة_Internal_عقدة>(Next_عقدة) &&
               "Expected an internal node?");
        Active.Idx += SubstringLen;
        Active.Len -= SubstringLen;
        Active.عقدة = cast<Suffix_شجرة_Internal_عقدة>(Next_عقدة);
        استمر;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      unsigned LastChar = Str[EndIdx];

      // Is the string we're trying to insert a substring of the next node?
      إذا (Str[Next_عقدة->getStartIdx() + Active.Len] == LastChar) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        إذا (NeedsLink && !Active.عقدة->is_جذر()) {
          NeedsLink->setLink(Active.عقدة);
          NeedsLink = nullptr;
        }

        Active.Len++;
        اكسر;
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
      Suffix_شجرة_Internal_عقدة *Split_عقدة = أدرج_Internal_عقدة(
          Active.عقدة, Next_عقدة->getStartIdx(),
          Next_عقدة->getStartIdx() + Active.Len - 1, FirstChar);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      أدرج_Leaf(*Split_عقدة, EndIdx, LastChar);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      Next_عقدة->incrementStartIdx(Active.Len);
      Split_عقدة->Children[Str[Next_عقدة->getStartIdx()]] = Next_عقدة;

      // SplitNode is an internal node, update the suffix link.
      إذا (NeedsLink)
        NeedsLink->setLink(Split_عقدة);

      NeedsLink = Split_عقدة;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    SuffixesToAdd--;

    إذا (Active.عقدة->is_جذر()) {
      إذا (Active.Len > 0) {
        Active.Len--;
        Active.Idx = EndIdx - SuffixesToAdd + 1;
      }
    } وإلا {
      // Start the next phase at the next smallest suffix.
      Active.عقدة = Active.عقدة->getLink();
    }
  }

  إرجاع SuffixesToAdd;
}

فراغ Suffix_شجرة::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = nullptr;

  // Each leaf node represents a repeat of a string.
  SmallVector<unsigned> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  بينما (!InternalNodesToVisit.empty()) {
    RepeatedSubstringStarts.clear();
    auto *Curr = InternalNodesToVisit.اسحب_back_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    unsigned Length = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    لكل (auto &ChildPair : Curr->Children)
      // Save all of this node's children for processing.
      إذا (auto *InternalChild =
              dyn_cast<Suffix_شجرة_Internal_عقدة>(ChildPair.second))
        InternalNodesToVisit.ادفع_back(InternalChild);

    // If length of repeated substring is below threshold, then skip it.
    إذا (Length < MinLength)
      استمر;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    إذا (Curr->is_جذر())
      استمر;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    إذا (OutlinerLeafDescendants) {
      لكل (unsigned I = Curr->get_يسار_Leaf_Idx(); I <= Curr->get_يمين_Leaf_Idx();
           ++I)
        RepeatedSubstringStarts.ادفع_back(LeafNodes[I]->getSuffixIdx());
    } وإلا {
      لكل (auto &ChildPair : Curr->Children)
        إذا (auto *Leaf = dyn_cast<Suffix_شجرة_Leaf_عقدة>(ChildPair.second))
          RepeatedSubstringStarts.ادفع_back(Leaf->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    إذا (RepeatedSubstringStarts.size() < 2)
      استمر;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.Length = Length;
    llvm::append_range(RS.StartIndices, RepeatedSubstringStarts);
    اكسر;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
