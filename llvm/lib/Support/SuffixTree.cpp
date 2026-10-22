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

استعمال نطاق llvm;

/// \returns the number of elements in the substring associated with \p N.
ساكن size_t انگ_Elements_اندريون_Substring(ثابت Suffix_وڻ_ڳنڍ *N) {
  أكد(N && "Got a null node?");
  إذا (خودڪار *اندروني = dyn_cast<Suffix_وڻ_اندروني_ڳنڍ>(N))
    إذا (اندروني->is_پاڙ())
      أرجع 0;
  أرجع N->get_پڇاڙي_Idx() - N->get_شروعات_Idx() + 1;
}

Suffix_وڻ::Suffix_وڻ(ثابت ArrayRef<بنانشان> &Str,
                       منطقي Outliner_پن_Descendants)
    : Str(Str), Outliner_پن_Descendants(Outliner_پن_Descendants) {
  پاڙ_2 = وجهو_پاڙ();
  سرگرم.ڳنڍ = پاڙ_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  بنانشان Suffixes_منزل_جوڙيو = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  لكل (بنانشان Pfx_پڇاڙي_Idx = 0, پڇاڙي_2 = Str.سائيز_2(); Pfx_پڇاڙي_Idx < پڇاڙي_2; Pfx_پڇاڙي_Idx++) {
    Suffixes_منزل_جوڙيو++;
    پن_پڇاڙي_Idx = Pfx_پڇاڙي_Idx; // Extend each of the leaves.
    Suffixes_منزل_جوڙيو = extend(Pfx_پڇاڙي_Idx, Suffixes_منزل_جوڙيو);
  }

  // Set the suffix indices of each leaf.
  أكد(پاڙ_2 && "Root node can't be nullptr!");
  سيٽ_Suffix_Indices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  إذا (Outliner_پن_Descendants)
    سيٽ_پن_Nodes();
}

Suffix_وڻ_ڳنڍ *Suffix_وڻ::وجهو_پن(Suffix_وڻ_اندروني_ڳنڍ &والد_2,
                                       بنانشان شروعات_Idx, بنانشان ڪنارو) {
  أكد(شروعات_Idx <= پن_پڇاڙي_Idx && "String can't start after it ends!");
  خودڪار *N = جديد (پن_ڳنڍ_Allocator.Allocate())
      Suffix_وڻ_پن_ڳنڍ(شروعات_Idx, &پن_پڇاڙي_Idx);
  والد_2.Children[ڪنارو] = N;
  أرجع N;
}

Suffix_وڻ_اندروني_ڳنڍ *
Suffix_وڻ::وجهو_اندروني_ڳنڍ(Suffix_وڻ_اندروني_ڳنڍ *والد_2,
                               بنانشان شروعات_Idx, بنانشان پڇاڙي_Idx,
                               بنانشان ڪنارو) {
  أكد(شروعات_Idx <= پڇاڙي_Idx && "String can't start after it ends!");
  أكد(!(!والد_2 && شروعات_Idx != Suffix_وڻ_ڳنڍ::خالي_جايون_Idx) &&
         "Non-root internal nodes must have parents!");
  خودڪار *N = جديد (اندروني_ڳنڍ_Allocator.Allocate())
      Suffix_وڻ_اندروني_ڳنڍ(شروعات_Idx, پڇاڙي_Idx, پاڙ_2);
  إذا (والد_2)
    والد_2->Children[ڪنارو] = N;
  أرجع N;
}

Suffix_وڻ_اندروني_ڳنڍ *Suffix_وڻ::وجهو_پاڙ() {
  أرجع وجهو_اندروني_ڳنڍ(/*Parent = */ خالي_اشارو, Suffix_وڻ_ڳنڍ::خالي_جايون_Idx,
                            Suffix_وڻ_ڳنڍ::خالي_جايون_Idx, /*Edge = */ 0);
}

فراغ Suffix_وڻ::سيٽ_Suffix_Indices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  Small_ويڪٽر<std::pair<Suffix_وڻ_ڳنڍ *, بنانشان>> منزل_گهمو;

  // Current node being visited.
  Suffix_وڻ_ڳنڍ *Curr_ڳنڍ = پاڙ_2;

  // Sum of the lengths of the nodes down the path to the current one.
  بنانشان Curr_ڳنڍ_Len = 0;
  منزل_گهمو.ڌڪو_پٺ({Curr_ڳنڍ, Curr_ڳنڍ_Len});
  طالما (!منزل_گهمو.خالي_جايون()) {
    std::tie(Curr_ڳنڍ, Curr_ڳنڍ_Len) = منزل_گهمو.ڪڍو_پٺ_val();
    // Length of the current node from the root down to here.
    Curr_ڳنڍ->سيٽ_Concat_Len(Curr_ڳنڍ_Len);
    إذا (خودڪار *اندروني_ڳنڍ = dyn_cast<Suffix_وڻ_اندروني_ڳنڍ>(Curr_ڳنڍ))
      لكل (خودڪار &ٻار_Pair : اندروني_ڳنڍ->Children) {
        أكد(ٻار_Pair.ٻيو_2 && "Node had a null child!");
        منزل_گهمو.ڌڪو_پٺ(
            {ٻار_Pair.ٻيو_2,
             Curr_ڳنڍ_Len + انگ_Elements_اندريون_Substring(ٻار_Pair.ٻيو_2)});
      }
    // No children, so we are at the end of the string.
    إذا (خودڪار *پن_ڳنڍ = dyn_cast<Suffix_وڻ_پن_ڳنڍ>(Curr_ڳنڍ))
      پن_ڳنڍ->سيٽ_Suffix_Idx(Str.سائيز_2() - Curr_ڳنڍ_Len);
  }
}

فراغ Suffix_وڻ::سيٽ_پن_Nodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  Small_ويڪٽر<Suffix_وڻ_ڳنڍ *> منزل_گهمو;
  منزل_گهمو.ڌڪو_پٺ(پاڙ_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  بنانشان پن_Counter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_وڻ_اندروني_ڳنڍ *,
           std::pair<Suffix_وڻ_ڳنڍ *, Suffix_وڻ_ڳنڍ *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  طالما (!منزل_گهمو.خالي_جايون()) {
    Suffix_وڻ_ڳنڍ *Curr_ڳنڍ = منزل_گهمو.ڪڍو_پٺ_val();
    إذا (خودڪار *Curr_اندروني_ڳنڍ = dyn_cast<Suffix_وڻ_اندروني_ڳنڍ>(Curr_ڳنڍ)) {
      // The current node is an internal node.
      خودڪار I = ChildrenMap.ڳوليو(Curr_اندروني_ڳنڍ);
      إذا (I == ChildrenMap.پڇاڙي()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        خودڪار J = Curr_اندروني_ڳنڍ->Children.begin();
        إذا (J != Curr_اندروني_ڳنڍ->Children.پڇاڙي()) {
          منزل_گهمو.ڌڪو_پٺ(Curr_ڳنڍ);
          Suffix_وڻ_ڳنڍ *پهريون_ٻار = J->ٻيو_2;
          Suffix_وڻ_ڳنڍ *آخري_ٻار = خالي_اشارو;
          لكل (; J != Curr_اندروني_ڳنڍ->Children.پڇاڙي(); ++J) {
            آخري_ٻار = J->ٻيو_2;
            منزل_گهمو.ڌڪو_پٺ(آخري_ٻار);
          }
          ChildrenMap[Curr_اندروني_ڳنڍ] = {پهريون_ٻار, آخري_ٻار};
        }
      } وإلا {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        خودڪار [پهريون_ٻار, آخري_ٻار] = I->ٻيو_2;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_ڳنڍ->سيٽ_ساڄو_پن_Idx(پهريون_ٻار->get_ساڄو_پن_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_ڳنڍ->سيٽ_کاٻو_پن_Idx(آخري_ٻار->get_کاٻو_پن_Idx());
        أكد(Curr_ڳنڍ->get_کاٻو_پن_Idx() <= Curr_ڳنڍ->get_ساڄو_پن_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } وإلا {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_ڳنڍ->سيٽ_کاٻو_پن_Idx(پن_Counter);
      Curr_ڳنڍ->سيٽ_ساڄو_پن_Idx(پن_Counter);
      ++پن_Counter;
      خودڪار *Curr_پن_ڳنڍ = cast<Suffix_وڻ_پن_ڳنڍ>(Curr_ڳنڍ);
      پن_Nodes.ڌڪو_پٺ(Curr_پن_ڳنڍ);
    }
  }
}

بنانشان Suffix_وڻ::extend(بنانشان پڇاڙي_Idx, بنانشان Suffixes_منزل_جوڙيو) {
  Suffix_وڻ_اندروني_ڳنڍ *NeedsLink = خالي_اشارو;

  طالما (Suffixes_منزل_جوڙيو > 0) {

    // Are we waiting to add anything other than just the last character?
    إذا (سرگرم.Len == 0) {
      // If not, then say the active index is the end index.
      سرگرم.Idx = پڇاڙي_Idx;
    }

    أكد(سرگرم.Idx <= پڇاڙي_Idx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    بنانشان پهريون_Char = Str[سرگرم.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    إذا (خودڪار It = سرگرم.ڳنڍ->Children.ڳوليو(پهريون_Char);
        It == سرگرم.ڳنڍ->Children.پڇاڙي()) {
      // If not, then we can just insert a leaf and move to the next step.
      وجهو_پن(*سرگرم.ڳنڍ, پڇاڙي_Idx, پهريون_Char);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      إذا (NeedsLink) {
        NeedsLink->سيٽ_Link(سرگرم.ڳنڍ);
        NeedsLink = خالي_اشارو;
      }
    } وإلا {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_وڻ_ڳنڍ *ايندڙ_ڳنڍ = It->ٻيو_2;

      بنانشان SubstringLen = انگ_Elements_اندريون_Substring(ايندڙ_ڳنڍ);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      إذا (سرگرم.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        أكد(isa<Suffix_وڻ_اندروني_ڳنڍ>(ايندڙ_ڳنڍ) &&
               "Expected an internal node?");
        سرگرم.Idx += SubstringLen;
        سرگرم.Len -= SubstringLen;
        سرگرم.ڳنڍ = cast<Suffix_وڻ_اندروني_ڳنڍ>(ايندڙ_ڳنڍ);
        استمر;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      بنانشان آخري_Char = Str[پڇاڙي_Idx];

      // Is the string we're trying to insert a substring of the next node?
      إذا (Str[ايندڙ_ڳنڍ->get_شروعات_Idx() + سرگرم.Len] == آخري_Char) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        إذا (NeedsLink && !سرگرم.ڳنڍ->is_پاڙ()) {
          NeedsLink->سيٽ_Link(سرگرم.ڳنڍ);
          NeedsLink = خالي_اشارو;
        }

        سرگرم.Len++;
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
      Suffix_وڻ_اندروني_ڳنڍ *ورهايو_ڳنڍ = وجهو_اندروني_ڳنڍ(
          سرگرم.ڳنڍ, ايندڙ_ڳنڍ->get_شروعات_Idx(),
          ايندڙ_ڳنڍ->get_شروعات_Idx() + سرگرم.Len - 1, پهريون_Char);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      وجهو_پن(*ورهايو_ڳنڍ, پڇاڙي_Idx, آخري_Char);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      ايندڙ_ڳنڍ->increment_شروعات_Idx(سرگرم.Len);
      ورهايو_ڳنڍ->Children[Str[ايندڙ_ڳنڍ->get_شروعات_Idx()]] = ايندڙ_ڳنڍ;

      // SplitNode is an internal node, update the suffix link.
      إذا (NeedsLink)
        NeedsLink->سيٽ_Link(ورهايو_ڳنڍ);

      NeedsLink = ورهايو_ڳنڍ;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    Suffixes_منزل_جوڙيو--;

    إذا (سرگرم.ڳنڍ->is_پاڙ()) {
      إذا (سرگرم.Len > 0) {
        سرگرم.Len--;
        سرگرم.Idx = پڇاڙي_Idx - Suffixes_منزل_جوڙيو + 1;
      }
    } وإلا {
      // Start the next phase at the next smallest suffix.
      سرگرم.ڳنڍ = سرگرم.ڳنڍ->getLink();
    }
  }

  أرجع Suffixes_منزل_جوڙيو;
}

فراغ Suffix_وڻ::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = خالي_اشارو;

  // Each leaf node represents a repeat of a string.
  Small_ويڪٽر<بنانشان> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  طالما (!اندروني_Nodes_منزل_گهمو.خالي_جايون()) {
    RepeatedSubstringStarts.clear();
    خودڪار *Curr = اندروني_Nodes_منزل_گهمو.ڪڍو_پٺ_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    بنانشان ڊيگهائي = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    لكل (خودڪار &ٻار_Pair : Curr->Children)
      // Save all of this node's children for processing.
      إذا (خودڪار *اندروني_ٻار =
              dyn_cast<Suffix_وڻ_اندروني_ڳنڍ>(ٻار_Pair.ٻيو_2))
        اندروني_Nodes_منزل_گهمو.ڌڪو_پٺ(اندروني_ٻار);

    // If length of repeated substring is below threshold, then skip it.
    إذا (ڊيگهائي < Min_ڊيگهائي)
      استمر;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    إذا (Curr->is_پاڙ())
      استمر;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    إذا (Outliner_پن_Descendants) {
      لكل (بنانشان I = Curr->get_کاٻو_پن_Idx(); I <= Curr->get_ساڄو_پن_Idx();
           ++I)
        RepeatedSubstringStarts.ڌڪو_پٺ(پن_Nodes[I]->getSuffixIdx());
    } وإلا {
      لكل (خودڪار &ٻار_Pair : Curr->Children)
        إذا (خودڪار *پن = dyn_cast<Suffix_وڻ_پن_ڳنڍ>(ٻار_Pair.ٻيو_2))
          RepeatedSubstringStarts.ڌڪو_پٺ(پن->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    إذا (RepeatedSubstringStarts.سائيز_2() < 2)
      استمر;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.ڊيگهائي = ڊيگهائي;
    llvm::append_range(RS.شروعات_Indices, RepeatedSubstringStarts);
    اكسر;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
