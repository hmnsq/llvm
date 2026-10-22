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

ورتوں نطاق llvm;

/// \returns the number of elements in the substring associated with \p N.
ساكن size_t عدد_Elements_وچ_Substring(ثابت Suffix_درخت_گرہ *N) {
  أكد(N && "Got a null node?");
  إذا (خودکار *اندرونی = dyn_cast<Suffix_درخت_اندرونی_گرہ>(N))
    إذا (اندرونی->is_جڑ())
      أرجع 0;
  أرجع N->get_سرا_Idx() - N->get_شروع_Idx() + 1;
}

Suffix_درخت::Suffix_درخت(ثابت ArrayRef<بنانشانی> &Str,
                       منطقي Outliner_پتی_Descendants)
    : Str(Str), Outliner_پتی_Descendants(Outliner_پتی_Descendants) {
  جڑ_2 = داخل_کرو_جڑ();
  سرگرم.گرہ = جڑ_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  بنانشانی Suffixes_منزل_جوڑو = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  لكل (بنانشانی Pfx_سرا_Idx = 0, سرا_2 = Str.ناپ_2(); Pfx_سرا_Idx < سرا_2; Pfx_سرا_Idx++) {
    Suffixes_منزل_جوڑو++;
    پتی_سرا_Idx = Pfx_سرا_Idx; // Extend each of the leaves.
    Suffixes_منزل_جوڑو = extend(Pfx_سرا_Idx, Suffixes_منزل_جوڑو);
  }

  // Set the suffix indices of each leaf.
  أكد(جڑ_2 && "Root node can't be nullptr!");
  سیٹ_Suffix_Indices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  إذا (Outliner_پتی_Descendants)
    سیٹ_پتی_Nodes();
}

Suffix_درخت_گرہ *Suffix_درخت::داخل_کرو_پتی(Suffix_درخت_اندرونی_گرہ &ماپا_2,
                                       بنانشانی شروع_Idx, بنانشانی کنارا) {
  أكد(شروع_Idx <= پتی_سرا_Idx && "String can't start after it ends!");
  خودکار *N = جديد (پتی_گرہ_Allocator.Allocate())
      Suffix_درخت_پتی_گرہ(شروع_Idx, &پتی_سرا_Idx);
  ماپا_2.Children[کنارا] = N;
  أرجع N;
}

Suffix_درخت_اندرونی_گرہ *
Suffix_درخت::داخل_کرو_اندرونی_گرہ(Suffix_درخت_اندرونی_گرہ *ماپا_2,
                               بنانشانی شروع_Idx, بنانشانی سرا_Idx,
                               بنانشانی کنارا) {
  أكد(شروع_Idx <= سرا_Idx && "String can't start after it ends!");
  أكد(!(!ماپا_2 && شروع_Idx != Suffix_درخت_گرہ::خالی_تھاواں_Idx) &&
         "Non-root internal nodes must have parents!");
  خودکار *N = جديد (اندرونی_گرہ_Allocator.Allocate())
      Suffix_درخت_اندرونی_گرہ(شروع_Idx, سرا_Idx, جڑ_2);
  إذا (ماپا_2)
    ماپا_2->Children[کنارا] = N;
  أرجع N;
}

Suffix_درخت_اندرونی_گرہ *Suffix_درخت::داخل_کرو_جڑ() {
  أرجع داخل_کرو_اندرونی_گرہ(/*Parent = */ خالی_سوچک, Suffix_درخت_گرہ::خالی_تھاواں_Idx,
                            Suffix_درخت_گرہ::خالی_تھاواں_Idx, /*Edge = */ 0);
}

فراغ Suffix_درخت::سیٹ_Suffix_Indices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  Small_سمتیہ<std::pair<Suffix_درخت_گرہ *, بنانشانی>> منزل_ویکھو;

  // Current node being visited.
  Suffix_درخت_گرہ *Curr_گرہ = جڑ_2;

  // Sum of the lengths of the nodes down the path to the current one.
  بنانشانی Curr_گرہ_Len = 0;
  منزل_ویکھو.ڈالو_پٹھ({Curr_گرہ, Curr_گرہ_Len});
  طالما (!منزل_ویکھو.خالی_تھاواں()) {
    std::tie(Curr_گرہ, Curr_گرہ_Len) = منزل_ویکھو.نکالو_پٹھ_val();
    // Length of the current node from the root down to here.
    Curr_گرہ->سیٹ_Concat_Len(Curr_گرہ_Len);
    إذا (خودکار *اندرونی_گرہ = dyn_cast<Suffix_درخت_اندرونی_گرہ>(Curr_گرہ))
      لكل (خودکار &بچہ_Pair : اندرونی_گرہ->Children) {
        أكد(بچہ_Pair.دوجا_2 && "Node had a null child!");
        منزل_ویکھو.ڈالو_پٹھ(
            {بچہ_Pair.دوجا_2,
             Curr_گرہ_Len + عدد_Elements_وچ_Substring(بچہ_Pair.دوجا_2)});
      }
    // No children, so we are at the end of the string.
    إذا (خودکار *پتی_گرہ = dyn_cast<Suffix_درخت_پتی_گرہ>(Curr_گرہ))
      پتی_گرہ->سیٹ_Suffix_Idx(Str.ناپ_2() - Curr_گرہ_Len);
  }
}

فراغ Suffix_درخت::سیٹ_پتی_Nodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  Small_سمتیہ<Suffix_درخت_گرہ *> منزل_ویکھو;
  منزل_ویکھو.ڈالو_پٹھ(جڑ_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  بنانشانی پتی_Counter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_درخت_اندرونی_گرہ *,
           std::pair<Suffix_درخت_گرہ *, Suffix_درخت_گرہ *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  طالما (!منزل_ویکھو.خالی_تھاواں()) {
    Suffix_درخت_گرہ *Curr_گرہ = منزل_ویکھو.نکالو_پٹھ_val();
    إذا (خودکار *Curr_اندرونی_گرہ = dyn_cast<Suffix_درخت_اندرونی_گرہ>(Curr_گرہ)) {
      // The current node is an internal node.
      خودکار I = ChildrenMap.لبھو(Curr_اندرونی_گرہ);
      إذا (I == ChildrenMap.سرا()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        خودکار J = Curr_اندرونی_گرہ->Children.begin();
        إذا (J != Curr_اندرونی_گرہ->Children.سرا()) {
          منزل_ویکھو.ڈالو_پٹھ(Curr_گرہ);
          Suffix_درخت_گرہ *پہلا_بچہ = J->دوجا_2;
          Suffix_درخت_گرہ *آخری_بچہ = خالی_سوچک;
          لكل (; J != Curr_اندرونی_گرہ->Children.سرا(); ++J) {
            آخری_بچہ = J->دوجا_2;
            منزل_ویکھو.ڈالو_پٹھ(آخری_بچہ);
          }
          ChildrenMap[Curr_اندرونی_گرہ] = {پہلا_بچہ, آخری_بچہ};
        }
      } وإلا {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        خودکار [پہلا_بچہ, آخری_بچہ] = I->دوجا_2;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_گرہ->سیٹ_دایاں_پتی_Idx(پہلا_بچہ->get_دایاں_پتی_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_گرہ->سیٹ_بایاں_پتی_Idx(آخری_بچہ->get_بایاں_پتی_Idx());
        أكد(Curr_گرہ->get_بایاں_پتی_Idx() <= Curr_گرہ->get_دایاں_پتی_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } وإلا {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_گرہ->سیٹ_بایاں_پتی_Idx(پتی_Counter);
      Curr_گرہ->سیٹ_دایاں_پتی_Idx(پتی_Counter);
      ++پتی_Counter;
      خودکار *Curr_پتی_گرہ = cast<Suffix_درخت_پتی_گرہ>(Curr_گرہ);
      پتی_Nodes.ڈالو_پٹھ(Curr_پتی_گرہ);
    }
  }
}

بنانشانی Suffix_درخت::extend(بنانشانی سرا_Idx, بنانشانی Suffixes_منزل_جوڑو) {
  Suffix_درخت_اندرونی_گرہ *NeedsLink = خالی_سوچک;

  طالما (Suffixes_منزل_جوڑو > 0) {

    // Are we waiting to add anything other than just the last character?
    إذا (سرگرم.Len == 0) {
      // If not, then say the active index is the end index.
      سرگرم.Idx = سرا_Idx;
    }

    أكد(سرگرم.Idx <= سرا_Idx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    بنانشانی پہلا_Char = Str[سرگرم.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    إذا (خودکار It = سرگرم.گرہ->Children.لبھو(پہلا_Char);
        It == سرگرم.گرہ->Children.سرا()) {
      // If not, then we can just insert a leaf and move to the next step.
      داخل_کرو_پتی(*سرگرم.گرہ, سرا_Idx, پہلا_Char);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      إذا (NeedsLink) {
        NeedsLink->سیٹ_Link(سرگرم.گرہ);
        NeedsLink = خالی_سوچک;
      }
    } وإلا {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_درخت_گرہ *اگلا_گرہ = It->دوجا_2;

      بنانشانی SubstringLen = عدد_Elements_وچ_Substring(اگلا_گرہ);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      إذا (سرگرم.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        أكد(isa<Suffix_درخت_اندرونی_گرہ>(اگلا_گرہ) &&
               "Expected an internal node?");
        سرگرم.Idx += SubstringLen;
        سرگرم.Len -= SubstringLen;
        سرگرم.گرہ = cast<Suffix_درخت_اندرونی_گرہ>(اگلا_گرہ);
        استمر;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      بنانشانی آخری_Char = Str[سرا_Idx];

      // Is the string we're trying to insert a substring of the next node?
      إذا (Str[اگلا_گرہ->get_شروع_Idx() + سرگرم.Len] == آخری_Char) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        إذا (NeedsLink && !سرگرم.گرہ->is_جڑ()) {
          NeedsLink->سیٹ_Link(سرگرم.گرہ);
          NeedsLink = خالی_سوچک;
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
      Suffix_درخت_اندرونی_گرہ *ونڈو_گرہ = داخل_کرو_اندرونی_گرہ(
          سرگرم.گرہ, اگلا_گرہ->get_شروع_Idx(),
          اگلا_گرہ->get_شروع_Idx() + سرگرم.Len - 1, پہلا_Char);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      داخل_کرو_پتی(*ونڈو_گرہ, سرا_Idx, آخری_Char);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      اگلا_گرہ->increment_شروع_Idx(سرگرم.Len);
      ونڈو_گرہ->Children[Str[اگلا_گرہ->get_شروع_Idx()]] = اگلا_گرہ;

      // SplitNode is an internal node, update the suffix link.
      إذا (NeedsLink)
        NeedsLink->سیٹ_Link(ونڈو_گرہ);

      NeedsLink = ونڈو_گرہ;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    Suffixes_منزل_جوڑو--;

    إذا (سرگرم.گرہ->is_جڑ()) {
      إذا (سرگرم.Len > 0) {
        سرگرم.Len--;
        سرگرم.Idx = سرا_Idx - Suffixes_منزل_جوڑو + 1;
      }
    } وإلا {
      // Start the next phase at the next smallest suffix.
      سرگرم.گرہ = سرگرم.گرہ->getLink();
    }
  }

  أرجع Suffixes_منزل_جوڑو;
}

فراغ Suffix_درخت::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = خالی_سوچک;

  // Each leaf node represents a repeat of a string.
  Small_سمتیہ<بنانشانی> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  طالما (!اندرونی_Nodes_منزل_ویکھو.خالی_تھاواں()) {
    RepeatedSubstringStarts.clear();
    خودکار *Curr = اندرونی_Nodes_منزل_ویکھو.نکالو_پٹھ_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    بنانشانی لمائی = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    لكل (خودکار &بچہ_Pair : Curr->Children)
      // Save all of this node's children for processing.
      إذا (خودکار *اندرونی_بچہ =
              dyn_cast<Suffix_درخت_اندرونی_گرہ>(بچہ_Pair.دوجا_2))
        اندرونی_Nodes_منزل_ویکھو.ڈالو_پٹھ(اندرونی_بچہ);

    // If length of repeated substring is below threshold, then skip it.
    إذا (لمائی < Min_لمائی)
      استمر;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    إذا (Curr->is_جڑ())
      استمر;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    إذا (Outliner_پتی_Descendants) {
      لكل (بنانشانی I = Curr->get_بایاں_پتی_Idx(); I <= Curr->get_دایاں_پتی_Idx();
           ++I)
        RepeatedSubstringStarts.ڈالو_پٹھ(پتی_Nodes[I]->getSuffixIdx());
    } وإلا {
      لكل (خودکار &بچہ_Pair : Curr->Children)
        إذا (خودکار *پتی = dyn_cast<Suffix_درخت_پتی_گرہ>(بچہ_Pair.دوجا_2))
          RepeatedSubstringStarts.ڈالو_پٹھ(پتی->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    إذا (RepeatedSubstringStarts.ناپ_2() < 2)
      استمر;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.لمائی = لمائی;
    llvm::append_range(RS.شروع_Indices, RepeatedSubstringStarts);
    اكسر;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
