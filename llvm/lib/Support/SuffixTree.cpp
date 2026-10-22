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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SuffixTree.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Allocator.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Casting.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SuffixTreeNode.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

/// \returns the number of elements in the substring associated with \p N.
ਸਥਿਰ size_t ਅੰਕ_Elements_ਵਿੱਚ_Substring(ਅਚਲ Suffix_ਰੁੱਖ_ਗੰਢ *N) {
  ਪ੍ਰਤਿਜ੍ਞਾ(N && "Got a null node?");
  ਜੇ (ਸਵੈਚਾਲਿਤ *ਅੰਦਰੂਨੀ = dyn_cast<Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ>(N))
    ਜੇ (ਅੰਦਰੂਨੀ->is_ਜੜ੍ਹ())
      ਵਾਪਸ 0;
  ਵਾਪਸ N->get_ਸਿਰਾ_Idx() - N->get_ਸ਼ੁਰੂ_Idx() + 1;
}

Suffix_ਰੁੱਖ::Suffix_ਰੁੱਖ(ਅਚਲ ArrayRef<ਬਿਨਾਨਿਸ਼ਾਨੀ> &Str,
                       ਤਾਰਕਿਕ Outliner_ਪੱਤੀ_Descendants)
    : Str(Str), Outliner_ਪੱਤੀ_Descendants(Outliner_ਪੱਤੀ_Descendants) {
  ਜੜ੍ਹ_2 = ਪਾਓ_ਜੜ੍ਹ();
  ਸਰਗਰਮ.ਗੰਢ = ਜੜ੍ਹ_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  ਬਿਨਾਨਿਸ਼ਾਨੀ Suffixes_ਮੰਜ਼ਿਲ_ਜੋੜੋ = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ Pfx_ਸਿਰਾ_Idx = 0, ਸਿਰਾ_2 = Str.ਮਾਪ_2(); Pfx_ਸਿਰਾ_Idx < ਸਿਰਾ_2; Pfx_ਸਿਰਾ_Idx++) {
    Suffixes_ਮੰਜ਼ਿਲ_ਜੋੜੋ++;
    ਪੱਤੀ_ਸਿਰਾ_Idx = Pfx_ਸਿਰਾ_Idx; // Extend each of the leaves.
    Suffixes_ਮੰਜ਼ਿਲ_ਜੋੜੋ = extend(Pfx_ਸਿਰਾ_Idx, Suffixes_ਮੰਜ਼ਿਲ_ਜੋੜੋ);
  }

  // Set the suffix indices of each leaf.
  ਪ੍ਰਤਿਜ੍ਞਾ(ਜੜ੍ਹ_2 && "Root node can't be nullptr!");
  ਸੈੱਟ_Suffix_Indices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  ਜੇ (Outliner_ਪੱਤੀ_Descendants)
    ਸੈੱਟ_ਪੱਤੀ_Nodes();
}

Suffix_ਰੁੱਖ_ਗੰਢ *Suffix_ਰੁੱਖ::ਪਾਓ_ਪੱਤੀ(Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ &ਮਾਪਾ_2,
                                       ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_Idx, ਬਿਨਾਨਿਸ਼ਾਨੀ ਕਿਨਾਰਾ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਸ਼ੁਰੂ_Idx <= ਪੱਤੀ_ਸਿਰਾ_Idx && "String can't start after it ends!");
  ਸਵੈਚਾਲਿਤ *N = ਨਵਾਂ (ਪੱਤੀ_ਗੰਢ_Allocator.Allocate())
      Suffix_ਰੁੱਖ_ਪੱਤੀ_ਗੰਢ(ਸ਼ੁਰੂ_Idx, &ਪੱਤੀ_ਸਿਰਾ_Idx);
  ਮਾਪਾ_2.Children[ਕਿਨਾਰਾ] = N;
  ਵਾਪਸ N;
}

Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ *
Suffix_ਰੁੱਖ::ਪਾਓ_ਅੰਦਰੂਨੀ_ਗੰਢ(Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ *ਮਾਪਾ_2,
                               ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_Idx, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_Idx,
                               ਬਿਨਾਨਿਸ਼ਾਨੀ ਕਿਨਾਰਾ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਸ਼ੁਰੂ_Idx <= ਸਿਰਾ_Idx && "String can't start after it ends!");
  ਪ੍ਰਤਿਜ੍ਞਾ(!(!ਮਾਪਾ_2 && ਸ਼ੁਰੂ_Idx != Suffix_ਰੁੱਖ_ਗੰਢ::ਖਾਲੀ_ਥਾਵਾਂ_Idx) &&
         "Non-root internal nodes must have parents!");
  ਸਵੈਚਾਲਿਤ *N = ਨਵਾਂ (ਅੰਦਰੂਨੀ_ਗੰਢ_Allocator.Allocate())
      Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ(ਸ਼ੁਰੂ_Idx, ਸਿਰਾ_Idx, ਜੜ੍ਹ_2);
  ਜੇ (ਮਾਪਾ_2)
    ਮਾਪਾ_2->Children[ਕਿਨਾਰਾ] = N;
  ਵਾਪਸ N;
}

Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ *Suffix_ਰੁੱਖ::ਪਾਓ_ਜੜ੍ਹ() {
  ਵਾਪਸ ਪਾਓ_ਅੰਦਰੂਨੀ_ਗੰਢ(/*Parent = */ ਖਾਲੀ_ਸੂਚਕ, Suffix_ਰੁੱਖ_ਗੰਢ::ਖਾਲੀ_ਥਾਵਾਂ_Idx,
                            Suffix_ਰੁੱਖ_ਗੰਢ::ਖਾਲੀ_ਥਾਵਾਂ_Idx, /*Edge = */ 0);
}

ਖਾਲੀ Suffix_ਰੁੱਖ::ਸੈੱਟ_Suffix_Indices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  Small_ਸਦਿਸ਼<std::pair<Suffix_ਰੁੱਖ_ਗੰਢ *, ਬਿਨਾਨਿਸ਼ਾਨੀ>> ਮੰਜ਼ਿਲ_ਦੇਖੋ;

  // Current node being visited.
  Suffix_ਰੁੱਖ_ਗੰਢ *Curr_ਗੰਢ = ਜੜ੍ਹ_2;

  // Sum of the lengths of the nodes down the path to the current one.
  ਬਿਨਾਨਿਸ਼ਾਨੀ Curr_ਗੰਢ_Len = 0;
  ਮੰਜ਼ਿਲ_ਦੇਖੋ.ਧੱਕੋ_ਪਿੱਠ({Curr_ਗੰਢ, Curr_ਗੰਢ_Len});
  ਜਦੋਂ (!ਮੰਜ਼ਿਲ_ਦੇਖੋ.ਖਾਲੀ_ਥਾਵਾਂ()) {
    std::tie(Curr_ਗੰਢ, Curr_ਗੰਢ_Len) = ਮੰਜ਼ਿਲ_ਦੇਖੋ.ਕੱਢੋ_ਪਿੱਠ_val();
    // Length of the current node from the root down to here.
    Curr_ਗੰਢ->ਸੈੱਟ_Concat_Len(Curr_ਗੰਢ_Len);
    ਜੇ (ਸਵੈਚਾਲਿਤ *ਅੰਦਰੂਨੀ_ਗੰਢ = dyn_cast<Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ>(Curr_ਗੰਢ))
      ਲਈ (ਸਵੈਚਾਲਿਤ &ਬੱਚਾ_Pair : ਅੰਦਰੂਨੀ_ਗੰਢ->Children) {
        ਪ੍ਰਤਿਜ੍ਞਾ(ਬੱਚਾ_Pair.ਦੂਜਾ_2 && "Node had a null child!");
        ਮੰਜ਼ਿਲ_ਦੇਖੋ.ਧੱਕੋ_ਪਿੱਠ(
            {ਬੱਚਾ_Pair.ਦੂਜਾ_2,
             Curr_ਗੰਢ_Len + ਅੰਕ_Elements_ਵਿੱਚ_Substring(ਬੱਚਾ_Pair.ਦੂਜਾ_2)});
      }
    // No children, so we are at the end of the string.
    ਜੇ (ਸਵੈਚਾਲਿਤ *ਪੱਤੀ_ਗੰਢ = dyn_cast<Suffix_ਰੁੱਖ_ਪੱਤੀ_ਗੰਢ>(Curr_ਗੰਢ))
      ਪੱਤੀ_ਗੰਢ->ਸੈੱਟ_Suffix_Idx(Str.ਮਾਪ_2() - Curr_ਗੰਢ_Len);
  }
}

ਖਾਲੀ Suffix_ਰੁੱਖ::ਸੈੱਟ_ਪੱਤੀ_Nodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  Small_ਸਦਿਸ਼<Suffix_ਰੁੱਖ_ਗੰਢ *> ਮੰਜ਼ਿਲ_ਦੇਖੋ;
  ਮੰਜ਼ਿਲ_ਦੇਖੋ.ਧੱਕੋ_ਪਿੱਠ(ਜੜ੍ਹ_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੱਤੀ_Counter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ *,
           std::pair<Suffix_ਰੁੱਖ_ਗੰਢ *, Suffix_ਰੁੱਖ_ਗੰਢ *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  ਜਦੋਂ (!ਮੰਜ਼ਿਲ_ਦੇਖੋ.ਖਾਲੀ_ਥਾਵਾਂ()) {
    Suffix_ਰੁੱਖ_ਗੰਢ *Curr_ਗੰਢ = ਮੰਜ਼ਿਲ_ਦੇਖੋ.ਕੱਢੋ_ਪਿੱਠ_val();
    ਜੇ (ਸਵੈਚਾਲਿਤ *Curr_ਅੰਦਰੂਨੀ_ਗੰਢ = dyn_cast<Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ>(Curr_ਗੰਢ)) {
      // The current node is an internal node.
      ਸਵੈਚਾਲਿਤ I = ChildrenMap.ਲੱਭੋ(Curr_ਅੰਦਰੂਨੀ_ਗੰਢ);
      ਜੇ (I == ChildrenMap.ਸਿਰਾ()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        ਸਵੈਚਾਲਿਤ J = Curr_ਅੰਦਰੂਨੀ_ਗੰਢ->Children.begin();
        ਜੇ (J != Curr_ਅੰਦਰੂਨੀ_ਗੰਢ->Children.ਸਿਰਾ()) {
          ਮੰਜ਼ਿਲ_ਦੇਖੋ.ਧੱਕੋ_ਪਿੱਠ(Curr_ਗੰਢ);
          Suffix_ਰੁੱਖ_ਗੰਢ *ਪਹਿਲਾ_ਬੱਚਾ = J->ਦੂਜਾ_2;
          Suffix_ਰੁੱਖ_ਗੰਢ *ਆਖਰੀ_ਬੱਚਾ = ਖਾਲੀ_ਸੂਚਕ;
          ਲਈ (; J != Curr_ਅੰਦਰੂਨੀ_ਗੰਢ->Children.ਸਿਰਾ(); ++J) {
            ਆਖਰੀ_ਬੱਚਾ = J->ਦੂਜਾ_2;
            ਮੰਜ਼ਿਲ_ਦੇਖੋ.ਧੱਕੋ_ਪਿੱਠ(ਆਖਰੀ_ਬੱਚਾ);
          }
          ChildrenMap[Curr_ਅੰਦਰੂਨੀ_ਗੰਢ] = {ਪਹਿਲਾ_ਬੱਚਾ, ਆਖਰੀ_ਬੱਚਾ};
        }
      } ਨਹੀਂਤਾਂ {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        ਸਵੈਚਾਲਿਤ [ਪਹਿਲਾ_ਬੱਚਾ, ਆਖਰੀ_ਬੱਚਾ] = I->ਦੂਜਾ_2;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_ਗੰਢ->ਸੈੱਟ_ਸੱਜਾ_ਪੱਤੀ_Idx(ਪਹਿਲਾ_ਬੱਚਾ->get_ਸੱਜਾ_ਪੱਤੀ_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_ਗੰਢ->ਸੈੱਟ_ਖੱਬਾ_ਪੱਤੀ_Idx(ਆਖਰੀ_ਬੱਚਾ->get_ਖੱਬਾ_ਪੱਤੀ_Idx());
        ਪ੍ਰਤਿਜ੍ਞਾ(Curr_ਗੰਢ->get_ਖੱਬਾ_ਪੱਤੀ_Idx() <= Curr_ਗੰਢ->get_ਸੱਜਾ_ਪੱਤੀ_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } ਨਹੀਂਤਾਂ {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_ਗੰਢ->ਸੈੱਟ_ਖੱਬਾ_ਪੱਤੀ_Idx(ਪੱਤੀ_Counter);
      Curr_ਗੰਢ->ਸੈੱਟ_ਸੱਜਾ_ਪੱਤੀ_Idx(ਪੱਤੀ_Counter);
      ++ਪੱਤੀ_Counter;
      ਸਵੈਚਾਲਿਤ *Curr_ਪੱਤੀ_ਗੰਢ = cast<Suffix_ਰੁੱਖ_ਪੱਤੀ_ਗੰਢ>(Curr_ਗੰਢ);
      ਪੱਤੀ_Nodes.ਧੱਕੋ_ਪਿੱਠ(Curr_ਪੱਤੀ_ਗੰਢ);
    }
  }
}

ਬਿਨਾਨਿਸ਼ਾਨੀ Suffix_ਰੁੱਖ::extend(ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_Idx, ਬਿਨਾਨਿਸ਼ਾਨੀ Suffixes_ਮੰਜ਼ਿਲ_ਜੋੜੋ) {
  Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ *NeedsLink = ਖਾਲੀ_ਸੂਚਕ;

  ਜਦੋਂ (Suffixes_ਮੰਜ਼ਿਲ_ਜੋੜੋ > 0) {

    // Are we waiting to add anything other than just the last character?
    ਜੇ (ਸਰਗਰਮ.Len == 0) {
      // If not, then say the active index is the end index.
      ਸਰਗਰਮ.Idx = ਸਿਰਾ_Idx;
    }

    ਪ੍ਰਤਿਜ੍ਞਾ(ਸਰਗਰਮ.Idx <= ਸਿਰਾ_Idx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪਹਿਲਾ_Char = Str[ਸਰਗਰਮ.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    ਜੇ (ਸਵੈਚਾਲਿਤ It = ਸਰਗਰਮ.ਗੰਢ->Children.ਲੱਭੋ(ਪਹਿਲਾ_Char);
        It == ਸਰਗਰਮ.ਗੰਢ->Children.ਸਿਰਾ()) {
      // If not, then we can just insert a leaf and move to the next step.
      ਪਾਓ_ਪੱਤੀ(*ਸਰਗਰਮ.ਗੰਢ, ਸਿਰਾ_Idx, ਪਹਿਲਾ_Char);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      ਜੇ (NeedsLink) {
        NeedsLink->ਸੈੱਟ_Link(ਸਰਗਰਮ.ਗੰਢ);
        NeedsLink = ਖਾਲੀ_ਸੂਚਕ;
      }
    } ਨਹੀਂਤਾਂ {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_ਰੁੱਖ_ਗੰਢ *ਅਗਲਾ_ਗੰਢ = It->ਦੂਜਾ_2;

      ਬਿਨਾਨਿਸ਼ਾਨੀ SubstringLen = ਅੰਕ_Elements_ਵਿੱਚ_Substring(ਅਗਲਾ_ਗੰਢ);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      ਜੇ (ਸਰਗਰਮ.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        ਪ੍ਰਤਿਜ੍ਞਾ(isa<Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ>(ਅਗਲਾ_ਗੰਢ) &&
               "Expected an internal node?");
        ਸਰਗਰਮ.Idx += SubstringLen;
        ਸਰਗਰਮ.Len -= SubstringLen;
        ਸਰਗਰਮ.ਗੰਢ = cast<Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ>(ਅਗਲਾ_ਗੰਢ);
        ਜਾਰੀ;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਆਖਰੀ_Char = Str[ਸਿਰਾ_Idx];

      // Is the string we're trying to insert a substring of the next node?
      ਜੇ (Str[ਅਗਲਾ_ਗੰਢ->get_ਸ਼ੁਰੂ_Idx() + ਸਰਗਰਮ.Len] == ਆਖਰੀ_Char) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        ਜੇ (NeedsLink && !ਸਰਗਰਮ.ਗੰਢ->is_ਜੜ੍ਹ()) {
          NeedsLink->ਸੈੱਟ_Link(ਸਰਗਰਮ.ਗੰਢ);
          NeedsLink = ਖਾਲੀ_ਸੂਚਕ;
        }

        ਸਰਗਰਮ.Len++;
        ਰੋਕੋ;
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
      Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ *ਵੰਡੋ_ਗੰਢ = ਪਾਓ_ਅੰਦਰੂਨੀ_ਗੰਢ(
          ਸਰਗਰਮ.ਗੰਢ, ਅਗਲਾ_ਗੰਢ->get_ਸ਼ੁਰੂ_Idx(),
          ਅਗਲਾ_ਗੰਢ->get_ਸ਼ੁਰੂ_Idx() + ਸਰਗਰਮ.Len - 1, ਪਹਿਲਾ_Char);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      ਪਾਓ_ਪੱਤੀ(*ਵੰਡੋ_ਗੰਢ, ਸਿਰਾ_Idx, ਆਖਰੀ_Char);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      ਅਗਲਾ_ਗੰਢ->increment_ਸ਼ੁਰੂ_Idx(ਸਰਗਰਮ.Len);
      ਵੰਡੋ_ਗੰਢ->Children[Str[ਅਗਲਾ_ਗੰਢ->get_ਸ਼ੁਰੂ_Idx()]] = ਅਗਲਾ_ਗੰਢ;

      // SplitNode is an internal node, update the suffix link.
      ਜੇ (NeedsLink)
        NeedsLink->ਸੈੱਟ_Link(ਵੰਡੋ_ਗੰਢ);

      NeedsLink = ਵੰਡੋ_ਗੰਢ;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    Suffixes_ਮੰਜ਼ਿਲ_ਜੋੜੋ--;

    ਜੇ (ਸਰਗਰਮ.ਗੰਢ->is_ਜੜ੍ਹ()) {
      ਜੇ (ਸਰਗਰਮ.Len > 0) {
        ਸਰਗਰਮ.Len--;
        ਸਰਗਰਮ.Idx = ਸਿਰਾ_Idx - Suffixes_ਮੰਜ਼ਿਲ_ਜੋੜੋ + 1;
      }
    } ਨਹੀਂਤਾਂ {
      // Start the next phase at the next smallest suffix.
      ਸਰਗਰਮ.ਗੰਢ = ਸਰਗਰਮ.ਗੰਢ->getLink();
    }
  }

  ਵਾਪਸ Suffixes_ਮੰਜ਼ਿਲ_ਜੋੜੋ;
}

ਖਾਲੀ Suffix_ਰੁੱਖ::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = ਖਾਲੀ_ਸੂਚਕ;

  // Each leaf node represents a repeat of a string.
  Small_ਸਦਿਸ਼<ਬਿਨਾਨਿਸ਼ਾਨੀ> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  ਜਦੋਂ (!ਅੰਦਰੂਨੀ_Nodes_ਮੰਜ਼ਿਲ_ਦੇਖੋ.ਖਾਲੀ_ਥਾਵਾਂ()) {
    RepeatedSubstringStarts.clear();
    ਸਵੈਚਾਲਿਤ *Curr = ਅੰਦਰੂਨੀ_Nodes_ਮੰਜ਼ਿਲ_ਦੇਖੋ.ਕੱਢੋ_ਪਿੱਠ_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾਈ = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    ਲਈ (ਸਵੈਚਾਲਿਤ &ਬੱਚਾ_Pair : Curr->Children)
      // Save all of this node's children for processing.
      ਜੇ (ਸਵੈਚਾਲਿਤ *ਅੰਦਰੂਨੀ_ਬੱਚਾ =
              dyn_cast<Suffix_ਰੁੱਖ_ਅੰਦਰੂਨੀ_ਗੰਢ>(ਬੱਚਾ_Pair.ਦੂਜਾ_2))
        ਅੰਦਰੂਨੀ_Nodes_ਮੰਜ਼ਿਲ_ਦੇਖੋ.ਧੱਕੋ_ਪਿੱਠ(ਅੰਦਰੂਨੀ_ਬੱਚਾ);

    // If length of repeated substring is below threshold, then skip it.
    ਜੇ (ਲੰਮਾਈ < Min_ਲੰਮਾਈ)
      ਜਾਰੀ;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    ਜੇ (Curr->is_ਜੜ੍ਹ())
      ਜਾਰੀ;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    ਜੇ (Outliner_ਪੱਤੀ_Descendants) {
      ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = Curr->get_ਖੱਬਾ_ਪੱਤੀ_Idx(); I <= Curr->get_ਸੱਜਾ_ਪੱਤੀ_Idx();
           ++I)
        RepeatedSubstringStarts.ਧੱਕੋ_ਪਿੱਠ(ਪੱਤੀ_Nodes[I]->getSuffixIdx());
    } ਨਹੀਂਤਾਂ {
      ਲਈ (ਸਵੈਚਾਲਿਤ &ਬੱਚਾ_Pair : Curr->Children)
        ਜੇ (ਸਵੈਚਾਲਿਤ *ਪੱਤੀ = dyn_cast<Suffix_ਰੁੱਖ_ਪੱਤੀ_ਗੰਢ>(ਬੱਚਾ_Pair.ਦੂਜਾ_2))
          RepeatedSubstringStarts.ਧੱਕੋ_ਪਿੱਠ(ਪੱਤੀ->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    ਜੇ (RepeatedSubstringStarts.ਮਾਪ_2() < 2)
      ਜਾਰੀ;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.ਲੰਮਾਈ = ਲੰਮਾਈ;
    llvm::append_range(RS.ਸ਼ੁਰੂ_Indices, RepeatedSubstringStarts);
    ਰੋਕੋ;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
