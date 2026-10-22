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

#ಅನ್ತರ್ಭಾವ "llvm/Support/SuffixTree.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Allocator.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Casting.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/SuffixTreeNode.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

/// \returns the number of elements in the substring associated with \p N.
ಸ್ಥಿರ size_t ಅಂಕೆ_Elements_ಒಳ_Substring(ಸ್ಥಿರಾಂಕ Suffix_ವೃಕ್ಷ_ಗಂಟು *N) {
  ಪ್ರತಿಜ್ಞಾ(N && "Got a null node?");
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *ಆಂತರಿಕ = dyn_cast<Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು>(N))
    ಆದರೆ (ಆಂತರಿಕ->is_ಬೇರು())
      ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ N->get_ಕೊನೆ_Idx() - N->get_ಆರಂಭ_Idx() + 1;
}

Suffix_ವೃಕ್ಷ::Suffix_ವೃಕ್ಷ(ಸ್ಥಿರಾಂಕ ArrayRef<ಚಿಹ್ನೆರಹಿತ> &Str,
                       ತಾರ್ಕಿಕ Outliner_ಎಲೆ_Descendants)
    : Str(Str), Outliner_ಎಲೆ_Descendants(Outliner_ಎಲೆ_Descendants) {
  ಬೇರು_2 = ಸೇರಿಸು_ಬೇರು();
  ಸಕ್ರಿಯ.ಗಂಟು = ಬೇರು_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  ಚಿಹ್ನೆರಹಿತ Suffixes_ಗಮ್ಯ_ಕೂಡಿಸು = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ Pfx_ಕೊನೆ_Idx = 0, ಕೊನೆ_2 = Str.ಗಾತ್ರ_3(); Pfx_ಕೊನೆ_Idx < ಕೊನೆ_2; Pfx_ಕೊನೆ_Idx++) {
    Suffixes_ಗಮ್ಯ_ಕೂಡಿಸು++;
    ಎಲೆ_ಕೊನೆ_Idx = Pfx_ಕೊನೆ_Idx; // Extend each of the leaves.
    Suffixes_ಗಮ್ಯ_ಕೂಡಿಸು = extend(Pfx_ಕೊನೆ_Idx, Suffixes_ಗಮ್ಯ_ಕೂಡಿಸು);
  }

  // Set the suffix indices of each leaf.
  ಪ್ರತಿಜ್ಞಾ(ಬೇರು_2 && "Root node can't be nullptr!");
  ಗಣ_Suffix_Indices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  ಆದರೆ (Outliner_ಎಲೆ_Descendants)
    ಗಣ_ಎಲೆ_Nodes();
}

Suffix_ವೃಕ್ಷ_ಗಂಟು *Suffix_ವೃಕ್ಷ::ಸೇರಿಸು_ಎಲೆ(Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು &ಪೋಷಕ_2,
                                       ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_Idx, ಚಿಹ್ನೆರಹಿತ ಅಂಚು) {
  ಪ್ರತಿಜ್ಞಾ(ಆರಂಭ_Idx <= ಎಲೆ_ಕೊನೆ_Idx && "String can't start after it ends!");
  ಸ್ವಯಂಚಾಲಿತ *N = ಹೊಸ (ಎಲೆ_ಗಂಟು_Allocator.Allocate())
      Suffix_ವೃಕ್ಷ_ಎಲೆ_ಗಂಟು(ಆರಂಭ_Idx, &ಎಲೆ_ಕೊನೆ_Idx);
  ಪೋಷಕ_2.Children[ಅಂಚು] = N;
  ಹಿಂತಿರುಗಿ N;
}

Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು *
Suffix_ವೃಕ್ಷ::ಸೇರಿಸು_ಆಂತರಿಕ_ಗಂಟು(Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು *ಪೋಷಕ_2,
                               ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_Idx, ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_Idx,
                               ಚಿಹ್ನೆರಹಿತ ಅಂಚು) {
  ಪ್ರತಿಜ್ಞಾ(ಆರಂಭ_Idx <= ಕೊನೆ_Idx && "String can't start after it ends!");
  ಪ್ರತಿಜ್ಞಾ(!(!ಪೋಷಕ_2 && ಆರಂಭ_Idx != Suffix_ವೃಕ್ಷ_ಗಂಟು::ಖಾಲಿಗಳು_Idx) &&
         "Non-root internal nodes must have parents!");
  ಸ್ವಯಂಚಾಲಿತ *N = ಹೊಸ (ಆಂತರಿಕ_ಗಂಟು_Allocator.Allocate())
      Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು(ಆರಂಭ_Idx, ಕೊನೆ_Idx, ಬೇರು_2);
  ಆದರೆ (ಪೋಷಕ_2)
    ಪೋಷಕ_2->Children[ಅಂಚು] = N;
  ಹಿಂತಿರುಗಿ N;
}

Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು *Suffix_ವೃಕ್ಷ::ಸೇರಿಸು_ಬೇರು() {
  ಹಿಂತಿರುಗಿ ಸೇರಿಸು_ಆಂತರಿಕ_ಗಂಟು(/*Parent = */ ಶೂನ್ಯಸೂಚಕ, Suffix_ವೃಕ್ಷ_ಗಂಟು::ಖಾಲಿಗಳು_Idx,
                            Suffix_ವೃಕ್ಷ_ಗಂಟು::ಖಾಲಿಗಳು_Idx, /*Edge = */ 0);
}

ಶೂನ್ಯ Suffix_ವೃಕ್ಷ::ಗಣ_Suffix_Indices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  Small_ಸದಿಶ<std::pair<Suffix_ವೃಕ್ಷ_ಗಂಟು *, ಚಿಹ್ನೆರಹಿತ>> ಗಮ್ಯ_ಭೇಟಿ;

  // Current node being visited.
  Suffix_ವೃಕ್ಷ_ಗಂಟು *Curr_ಗಂಟು = ಬೇರು_2;

  // Sum of the lengths of the nodes down the path to the current one.
  ಚಿಹ್ನೆರಹಿತ Curr_ಗಂಟು_Len = 0;
  ಗಮ್ಯ_ಭೇಟಿ.ತಳ್ಳು_ಹಿಂಭಾಗ({Curr_ಗಂಟು, Curr_ಗಂಟು_Len});
  ಆಗ (!ಗಮ್ಯ_ಭೇಟಿ.ಖಾಲಿಗಳು()) {
    std::tie(Curr_ಗಂಟು, Curr_ಗಂಟು_Len) = ಗಮ್ಯ_ಭೇಟಿ.ತೆಗೆ_ಹಿಂಭಾಗ_val();
    // Length of the current node from the root down to here.
    Curr_ಗಂಟು->ಗಣ_Concat_Len(Curr_ಗಂಟು_Len);
    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *ಆಂತರಿಕ_ಗಂಟು = dyn_cast<Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು>(Curr_ಗಂಟು))
      ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &ಮಗು_Pair : ಆಂತರಿಕ_ಗಂಟು->Children) {
        ಪ್ರತಿಜ್ಞಾ(ಮಗು_Pair.ಎರಡನೇ_2 && "Node had a null child!");
        ಗಮ್ಯ_ಭೇಟಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(
            {ಮಗು_Pair.ಎರಡನೇ_2,
             Curr_ಗಂಟು_Len + ಅಂಕೆ_Elements_ಒಳ_Substring(ಮಗು_Pair.ಎರಡನೇ_2)});
      }
    // No children, so we are at the end of the string.
    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *ಎಲೆ_ಗಂಟು = dyn_cast<Suffix_ವೃಕ್ಷ_ಎಲೆ_ಗಂಟು>(Curr_ಗಂಟು))
      ಎಲೆ_ಗಂಟು->ಗಣ_Suffix_Idx(Str.ಗಾತ್ರ_3() - Curr_ಗಂಟು_Len);
  }
}

ಶೂನ್ಯ Suffix_ವೃಕ್ಷ::ಗಣ_ಎಲೆ_Nodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  Small_ಸದಿಶ<Suffix_ವೃಕ್ಷ_ಗಂಟು *> ಗಮ್ಯ_ಭೇಟಿ;
  ಗಮ್ಯ_ಭೇಟಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಬೇರು_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  ಚಿಹ್ನೆರಹಿತ ಎಲೆ_Counter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು *,
           std::pair<Suffix_ವೃಕ್ಷ_ಗಂಟು *, Suffix_ವೃಕ್ಷ_ಗಂಟು *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  ಆಗ (!ಗಮ್ಯ_ಭೇಟಿ.ಖಾಲಿಗಳು()) {
    Suffix_ವೃಕ್ಷ_ಗಂಟು *Curr_ಗಂಟು = ಗಮ್ಯ_ಭೇಟಿ.ತೆಗೆ_ಹಿಂಭಾಗ_val();
    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *Curr_ಆಂತರಿಕ_ಗಂಟು = dyn_cast<Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು>(Curr_ಗಂಟು)) {
      // The current node is an internal node.
      ಸ್ವಯಂಚಾಲಿತ I = ChildrenMap.ಕಂಡುಹಿಡಿ(Curr_ಆಂತರಿಕ_ಗಂಟು);
      ಆದರೆ (I == ChildrenMap.ಕೊನೆ()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        ಸ್ವಯಂಚಾಲಿತ J = Curr_ಆಂತರಿಕ_ಗಂಟು->Children.begin();
        ಆದರೆ (J != Curr_ಆಂತರಿಕ_ಗಂಟು->Children.ಕೊನೆ()) {
          ಗಮ್ಯ_ಭೇಟಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(Curr_ಗಂಟು);
          Suffix_ವೃಕ್ಷ_ಗಂಟು *ಮೊದಲ_ಮಗು = J->ಎರಡನೇ_2;
          Suffix_ವೃಕ್ಷ_ಗಂಟು *ಕೊನೆಯ_ಮಗು = ಶೂನ್ಯಸೂಚಕ;
          ಗಾಗಿ (; J != Curr_ಆಂತರಿಕ_ಗಂಟು->Children.ಕೊನೆ(); ++J) {
            ಕೊನೆಯ_ಮಗು = J->ಎರಡನೇ_2;
            ಗಮ್ಯ_ಭೇಟಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಕೊನೆಯ_ಮಗು);
          }
          ChildrenMap[Curr_ಆಂತರಿಕ_ಗಂಟು] = {ಮೊದಲ_ಮಗು, ಕೊನೆಯ_ಮಗು};
        }
      } ಇಲ್ಲದಿದ್ದರೆ {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        ಸ್ವಯಂಚಾಲಿತ [ಮೊದಲ_ಮಗು, ಕೊನೆಯ_ಮಗು] = I->ಎರಡನೇ_2;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_ಗಂಟು->ಗಣ_ಬಲ_ಎಲೆ_Idx(ಮೊದಲ_ಮಗು->get_ಬಲ_ಎಲೆ_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_ಗಂಟು->ಗಣ_ಎಡ_ಎಲೆ_Idx(ಕೊನೆಯ_ಮಗು->get_ಎಡ_ಎಲೆ_Idx());
        ಪ್ರತಿಜ್ಞಾ(Curr_ಗಂಟು->get_ಎಡ_ಎಲೆ_Idx() <= Curr_ಗಂಟು->get_ಬಲ_ಎಲೆ_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } ಇಲ್ಲದಿದ್ದರೆ {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_ಗಂಟು->ಗಣ_ಎಡ_ಎಲೆ_Idx(ಎಲೆ_Counter);
      Curr_ಗಂಟು->ಗಣ_ಬಲ_ಎಲೆ_Idx(ಎಲೆ_Counter);
      ++ಎಲೆ_Counter;
      ಸ್ವಯಂಚಾಲಿತ *Curr_ಎಲೆ_ಗಂಟು = cast<Suffix_ವೃಕ್ಷ_ಎಲೆ_ಗಂಟು>(Curr_ಗಂಟು);
      ಎಲೆ_Nodes.ತಳ್ಳು_ಹಿಂಭಾಗ(Curr_ಎಲೆ_ಗಂಟು);
    }
  }
}

ಚಿಹ್ನೆರಹಿತ Suffix_ವೃಕ್ಷ::extend(ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_Idx, ಚಿಹ್ನೆರಹಿತ Suffixes_ಗಮ್ಯ_ಕೂಡಿಸು) {
  Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು *NeedsLink = ಶೂನ್ಯಸೂಚಕ;

  ಆಗ (Suffixes_ಗಮ್ಯ_ಕೂಡಿಸು > 0) {

    // Are we waiting to add anything other than just the last character?
    ಆದರೆ (ಸಕ್ರಿಯ.Len == 0) {
      // If not, then say the active index is the end index.
      ಸಕ್ರಿಯ.Idx = ಕೊನೆ_Idx;
    }

    ಪ್ರತಿಜ್ಞಾ(ಸಕ್ರಿಯ.Idx <= ಕೊನೆ_Idx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    ಚಿಹ್ನೆರಹಿತ ಮೊದಲ_Char = Str[ಸಕ್ರಿಯ.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ It = ಸಕ್ರಿಯ.ಗಂಟು->Children.ಕಂಡುಹಿಡಿ(ಮೊದಲ_Char);
        It == ಸಕ್ರಿಯ.ಗಂಟು->Children.ಕೊನೆ()) {
      // If not, then we can just insert a leaf and move to the next step.
      ಸೇರಿಸು_ಎಲೆ(*ಸಕ್ರಿಯ.ಗಂಟು, ಕೊನೆ_Idx, ಮೊದಲ_Char);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      ಆದರೆ (NeedsLink) {
        NeedsLink->ಗಣ_Link(ಸಕ್ರಿಯ.ಗಂಟು);
        NeedsLink = ಶೂನ್ಯಸೂಚಕ;
      }
    } ಇಲ್ಲದಿದ್ದರೆ {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_ವೃಕ್ಷ_ಗಂಟು *ಮುಂದಿನ_ಗಂಟು = It->ಎರಡನೇ_2;

      ಚಿಹ್ನೆರಹಿತ SubstringLen = ಅಂಕೆ_Elements_ಒಳ_Substring(ಮುಂದಿನ_ಗಂಟು);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      ಆದರೆ (ಸಕ್ರಿಯ.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        ಪ್ರತಿಜ್ಞಾ(isa<Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು>(ಮುಂದಿನ_ಗಂಟು) &&
               "Expected an internal node?");
        ಸಕ್ರಿಯ.Idx += SubstringLen;
        ಸಕ್ರಿಯ.Len -= SubstringLen;
        ಸಕ್ರಿಯ.ಗಂಟು = cast<Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು>(ಮುಂದಿನ_ಗಂಟು);
        ಮುಂದುವರಿ;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      ಚಿಹ್ನೆರಹಿತ ಕೊನೆಯ_Char = Str[ಕೊನೆ_Idx];

      // Is the string we're trying to insert a substring of the next node?
      ಆದರೆ (Str[ಮುಂದಿನ_ಗಂಟು->get_ಆರಂಭ_Idx() + ಸಕ್ರಿಯ.Len] == ಕೊನೆಯ_Char) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        ಆದರೆ (NeedsLink && !ಸಕ್ರಿಯ.ಗಂಟು->is_ಬೇರು()) {
          NeedsLink->ಗಣ_Link(ಸಕ್ರಿಯ.ಗಂಟು);
          NeedsLink = ಶೂನ್ಯಸೂಚಕ;
        }

        ಸಕ್ರಿಯ.Len++;
        ನಿಲ್ಲಿಸು;
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
      Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು *ವಿಭಜಿಸು_ಗಂಟು = ಸೇರಿಸು_ಆಂತರಿಕ_ಗಂಟು(
          ಸಕ್ರಿಯ.ಗಂಟು, ಮುಂದಿನ_ಗಂಟು->get_ಆರಂಭ_Idx(),
          ಮುಂದಿನ_ಗಂಟು->get_ಆರಂಭ_Idx() + ಸಕ್ರಿಯ.Len - 1, ಮೊದಲ_Char);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      ಸೇರಿಸು_ಎಲೆ(*ವಿಭಜಿಸು_ಗಂಟು, ಕೊನೆ_Idx, ಕೊನೆಯ_Char);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      ಮುಂದಿನ_ಗಂಟು->increment_ಆರಂಭ_Idx(ಸಕ್ರಿಯ.Len);
      ವಿಭಜಿಸು_ಗಂಟು->Children[Str[ಮುಂದಿನ_ಗಂಟು->get_ಆರಂಭ_Idx()]] = ಮುಂದಿನ_ಗಂಟು;

      // SplitNode is an internal node, update the suffix link.
      ಆದರೆ (NeedsLink)
        NeedsLink->ಗಣ_Link(ವಿಭಜಿಸು_ಗಂಟು);

      NeedsLink = ವಿಭಜಿಸು_ಗಂಟು;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    Suffixes_ಗಮ್ಯ_ಕೂಡಿಸು--;

    ಆದರೆ (ಸಕ್ರಿಯ.ಗಂಟು->is_ಬೇರು()) {
      ಆದರೆ (ಸಕ್ರಿಯ.Len > 0) {
        ಸಕ್ರಿಯ.Len--;
        ಸಕ್ರಿಯ.Idx = ಕೊನೆ_Idx - Suffixes_ಗಮ್ಯ_ಕೂಡಿಸು + 1;
      }
    } ಇಲ್ಲದಿದ್ದರೆ {
      // Start the next phase at the next smallest suffix.
      ಸಕ್ರಿಯ.ಗಂಟು = ಸಕ್ರಿಯ.ಗಂಟು->getLink();
    }
  }

  ಹಿಂತಿರುಗಿ Suffixes_ಗಮ್ಯ_ಕೂಡಿಸು;
}

ಶೂನ್ಯ Suffix_ವೃಕ್ಷ::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = ಶೂನ್ಯಸೂಚಕ;

  // Each leaf node represents a repeat of a string.
  Small_ಸದಿಶ<ಚಿಹ್ನೆರಹಿತ> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  ಆಗ (!ಆಂತರಿಕ_Nodes_ಗಮ್ಯ_ಭೇಟಿ.ಖಾಲಿಗಳು()) {
    RepeatedSubstringStarts.clear();
    ಸ್ವಯಂಚಾಲಿತ *Curr = ಆಂತರಿಕ_Nodes_ಗಮ್ಯ_ಭೇಟಿ.ತೆಗೆ_ಹಿಂಭಾಗ_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    ಚಿಹ್ನೆರಹಿತ ಉದ್ದಳತೆ = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &ಮಗು_Pair : Curr->Children)
      // Save all of this node's children for processing.
      ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *ಆಂತರಿಕ_ಮಗು =
              dyn_cast<Suffix_ವೃಕ್ಷ_ಆಂತರಿಕ_ಗಂಟು>(ಮಗು_Pair.ಎರಡನೇ_2))
        ಆಂತರಿಕ_Nodes_ಗಮ್ಯ_ಭೇಟಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಆಂತರಿಕ_ಮಗು);

    // If length of repeated substring is below threshold, then skip it.
    ಆದರೆ (ಉದ್ದಳತೆ < Min_ಉದ್ದಳತೆ)
      ಮುಂದುವರಿ;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    ಆದರೆ (Curr->is_ಬೇರು())
      ಮುಂದುವರಿ;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    ಆದರೆ (Outliner_ಎಲೆ_Descendants) {
      ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = Curr->get_ಎಡ_ಎಲೆ_Idx(); I <= Curr->get_ಬಲ_ಎಲೆ_Idx();
           ++I)
        RepeatedSubstringStarts.ತಳ್ಳು_ಹಿಂಭಾಗ(ಎಲೆ_Nodes[I]->getSuffixIdx());
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &ಮಗು_Pair : Curr->Children)
        ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *ಎಲೆ = dyn_cast<Suffix_ವೃಕ್ಷ_ಎಲೆ_ಗಂಟು>(ಮಗು_Pair.ಎರಡನೇ_2))
          RepeatedSubstringStarts.ತಳ್ಳು_ಹಿಂಭಾಗ(ಎಲೆ->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    ಆದರೆ (RepeatedSubstringStarts.ಗಾತ್ರ_3() < 2)
      ಮುಂದುವರಿ;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.ಉದ್ದಳತೆ = ಉದ್ದಳತೆ;
    llvm::append_range(RS.ಆರಂಭ_Indices, RepeatedSubstringStarts);
    ನಿಲ್ಲಿಸು;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
