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

#شامل "llvm/Support/SuffixTree.h"
#شامل "llvm/Support/Allocator.h"
#شامل "llvm/Support/Casting.h"
#شامل "llvm/Support/SuffixTreeNode.h"

استعمال نامخلا llvm;

/// \returns the number of elements in the substring associated with \p N.
ساکن size_t عدد_Elements_اندر_Substring(ثابت Suffix_درخت_گرہ *N) {
  أكد(N && "Got a null node?");
  اگر (خودکار *اندرونی_2 = dyn_cast<Suffix_درخت_اندرونی_گرہ>(N))
    اگر (اندرونی_2->is_جڑ())
      واپس 0;
  واپس N->get_انتہا_Idx() - N->get_آغاز_Idx() + 1;
}

Suffix_درخت::Suffix_درخت(ثابت ArrayRef<غیرعلامتی> &Str,
                       منطقی Outliner_برگ_Descendants)
    : Str(Str), Outliner_برگ_Descendants(Outliner_برگ_Descendants) {
  جڑ_2 = داخل_کرو_جڑ();
  فعال.گرہ = جڑ_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  غیرعلامتی Suffixes_مقصد_جوڑو = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  برائے (غیرعلامتی Pfx_انتہا_Idx = 0, انتہا_2 = Str.سائز_2(); Pfx_انتہا_Idx < انتہا_2; Pfx_انتہا_Idx++) {
    Suffixes_مقصد_جوڑو++;
    برگ_انتہا_Idx = Pfx_انتہا_Idx; // Extend each of the leaves.
    Suffixes_مقصد_جوڑو = extend(Pfx_انتہا_Idx, Suffixes_مقصد_جوڑو);
  }

  // Set the suffix indices of each leaf.
  أكد(جڑ_2 && "Root node can't be nullptr!");
  سیٹ_Suffix_Indices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  اگر (Outliner_برگ_Descendants)
    سیٹ_برگ_Nodes();
}

Suffix_درخت_گرہ *Suffix_درخت::داخل_کرو_برگ(Suffix_درخت_اندرونی_گرہ &والد_2,
                                       غیرعلامتی آغاز_Idx, غیرعلامتی کنارہ) {
  أكد(آغاز_Idx <= برگ_انتہا_Idx && "String can't start after it ends!");
  خودکار *N = نیا (برگ_گرہ_Allocator.Allocate())
      Suffix_درخت_برگ_گرہ(آغاز_Idx, &برگ_انتہا_Idx);
  والد_2.Children[کنارہ] = N;
  واپس N;
}

Suffix_درخت_اندرونی_گرہ *
Suffix_درخت::داخل_کرو_اندرونی_گرہ(Suffix_درخت_اندرونی_گرہ *والد_2,
                               غیرعلامتی آغاز_Idx, غیرعلامتی انتہا_Idx,
                               غیرعلامتی کنارہ) {
  أكد(آغاز_Idx <= انتہا_Idx && "String can't start after it ends!");
  أكد(!(!والد_2 && آغاز_Idx != Suffix_درخت_گرہ::خالی_خانے_Idx) &&
         "Non-root internal nodes must have parents!");
  خودکار *N = نیا (اندرونی_گرہ_Allocator.Allocate())
      Suffix_درخت_اندرونی_گرہ(آغاز_Idx, انتہا_Idx, جڑ_2);
  اگر (والد_2)
    والد_2->Children[کنارہ] = N;
  واپس N;
}

Suffix_درخت_اندرونی_گرہ *Suffix_درخت::داخل_کرو_جڑ() {
  واپس داخل_کرو_اندرونی_گرہ(/*Parent = */ خالی_اشاریہ, Suffix_درخت_گرہ::خالی_خانے_Idx,
                            Suffix_درخت_گرہ::خالی_خانے_Idx, /*Edge = */ 0);
}

خالی Suffix_درخت::سیٹ_Suffix_Indices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  Small_سمتیہ<std::pair<Suffix_درخت_گرہ *, غیرعلامتی>> مقصد_دورہ;

  // Current node being visited.
  Suffix_درخت_گرہ *Curr_گرہ = جڑ_2;

  // Sum of the lengths of the nodes down the path to the current one.
  غیرعلامتی Curr_گرہ_Len = 0;
  مقصد_دورہ.ڈالو_عقب({Curr_گرہ, Curr_گرہ_Len});
  جب (!مقصد_دورہ.خالی_خانے()) {
    std::tie(Curr_گرہ, Curr_گرہ_Len) = مقصد_دورہ.نکالو_عقب_val();
    // Length of the current node from the root down to here.
    Curr_گرہ->سیٹ_Concat_Len(Curr_گرہ_Len);
    اگر (خودکار *اندرونی_گرہ = dyn_cast<Suffix_درخت_اندرونی_گرہ>(Curr_گرہ))
      برائے (خودکار &بچہ_Pair : اندرونی_گرہ->Children) {
        أكد(بچہ_Pair.دوسرا_2 && "Node had a null child!");
        مقصد_دورہ.ڈالو_عقب(
            {بچہ_Pair.دوسرا_2,
             Curr_گرہ_Len + عدد_Elements_اندر_Substring(بچہ_Pair.دوسرا_2)});
      }
    // No children, so we are at the end of the string.
    اگر (خودکار *برگ_گرہ = dyn_cast<Suffix_درخت_برگ_گرہ>(Curr_گرہ))
      برگ_گرہ->سیٹ_Suffix_Idx(Str.سائز_2() - Curr_گرہ_Len);
  }
}

خالی Suffix_درخت::سیٹ_برگ_Nodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  Small_سمتیہ<Suffix_درخت_گرہ *> مقصد_دورہ;
  مقصد_دورہ.ڈالو_عقب(جڑ_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  غیرعلامتی برگ_شمار = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_درخت_اندرونی_گرہ *,
           std::pair<Suffix_درخت_گرہ *, Suffix_درخت_گرہ *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  جب (!مقصد_دورہ.خالی_خانے()) {
    Suffix_درخت_گرہ *Curr_گرہ = مقصد_دورہ.نکالو_عقب_val();
    اگر (خودکار *Curr_اندرونی_گرہ = dyn_cast<Suffix_درخت_اندرونی_گرہ>(Curr_گرہ)) {
      // The current node is an internal node.
      خودکار I = ChildrenMap.ڈھونڈو(Curr_اندرونی_گرہ);
      اگر (I == ChildrenMap.انتہا()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        خودکار J = Curr_اندرونی_گرہ->Children.begin();
        اگر (J != Curr_اندرونی_گرہ->Children.انتہا()) {
          مقصد_دورہ.ڈالو_عقب(Curr_گرہ);
          Suffix_درخت_گرہ *پہلا_بچہ = J->دوسرا_2;
          Suffix_درخت_گرہ *آخری_بچہ = خالی_اشاریہ;
          برائے (; J != Curr_اندرونی_گرہ->Children.انتہا(); ++J) {
            آخری_بچہ = J->دوسرا_2;
            مقصد_دورہ.ڈالو_عقب(آخری_بچہ);
          }
          ChildrenMap[Curr_اندرونی_گرہ] = {پہلا_بچہ, آخری_بچہ};
        }
      } ورنہ {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        خودکار [پہلا_بچہ, آخری_بچہ] = I->دوسرا_2;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_گرہ->سیٹ_دایاں_برگ_Idx(پہلا_بچہ->get_دایاں_برگ_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_گرہ->سیٹ_بایاں_برگ_Idx(آخری_بچہ->get_بایاں_برگ_Idx());
        أكد(Curr_گرہ->get_بایاں_برگ_Idx() <= Curr_گرہ->get_دایاں_برگ_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } ورنہ {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_گرہ->سیٹ_بایاں_برگ_Idx(برگ_شمار);
      Curr_گرہ->سیٹ_دایاں_برگ_Idx(برگ_شمار);
      ++برگ_شمار;
      خودکار *Curr_برگ_گرہ = cast<Suffix_درخت_برگ_گرہ>(Curr_گرہ);
      برگ_Nodes.ڈالو_عقب(Curr_برگ_گرہ);
    }
  }
}

غیرعلامتی Suffix_درخت::extend(غیرعلامتی انتہا_Idx, غیرعلامتی Suffixes_مقصد_جوڑو) {
  Suffix_درخت_اندرونی_گرہ *NeedsLink = خالی_اشاریہ;

  جب (Suffixes_مقصد_جوڑو > 0) {

    // Are we waiting to add anything other than just the last character?
    اگر (فعال.Len == 0) {
      // If not, then say the active index is the end index.
      فعال.Idx = انتہا_Idx;
    }

    أكد(فعال.Idx <= انتہا_Idx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    غیرعلامتی پہلا_Char = Str[فعال.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    اگر (خودکار It = فعال.گرہ->Children.ڈھونڈو(پہلا_Char);
        It == فعال.گرہ->Children.انتہا()) {
      // If not, then we can just insert a leaf and move to the next step.
      داخل_کرو_برگ(*فعال.گرہ, انتہا_Idx, پہلا_Char);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      اگر (NeedsLink) {
        NeedsLink->سیٹ_Link(فعال.گرہ);
        NeedsLink = خالی_اشاریہ;
      }
    } ورنہ {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_درخت_گرہ *اگلا_گرہ = It->دوسرا_2;

      غیرعلامتی SubstringLen = عدد_Elements_اندر_Substring(اگلا_گرہ);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      اگر (فعال.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        أكد(isa<Suffix_درخت_اندرونی_گرہ>(اگلا_گرہ) &&
               "Expected an internal node?");
        فعال.Idx += SubstringLen;
        فعال.Len -= SubstringLen;
        فعال.گرہ = cast<Suffix_درخت_اندرونی_گرہ>(اگلا_گرہ);
        جاری;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      غیرعلامتی آخری_Char = Str[انتہا_Idx];

      // Is the string we're trying to insert a substring of the next node?
      اگر (Str[اگلا_گرہ->get_آغاز_Idx() + فعال.Len] == آخری_Char) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        اگر (NeedsLink && !فعال.گرہ->is_جڑ()) {
          NeedsLink->سیٹ_Link(فعال.گرہ);
          NeedsLink = خالی_اشاریہ;
        }

        فعال.Len++;
        توڑو;
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
      Suffix_درخت_اندرونی_گرہ *بانٹو_گرہ = داخل_کرو_اندرونی_گرہ(
          فعال.گرہ, اگلا_گرہ->get_آغاز_Idx(),
          اگلا_گرہ->get_آغاز_Idx() + فعال.Len - 1, پہلا_Char);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      داخل_کرو_برگ(*بانٹو_گرہ, انتہا_Idx, آخری_Char);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      اگلا_گرہ->increment_آغاز_Idx(فعال.Len);
      بانٹو_گرہ->Children[Str[اگلا_گرہ->get_آغاز_Idx()]] = اگلا_گرہ;

      // SplitNode is an internal node, update the suffix link.
      اگر (NeedsLink)
        NeedsLink->سیٹ_Link(بانٹو_گرہ);

      NeedsLink = بانٹو_گرہ;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    Suffixes_مقصد_جوڑو--;

    اگر (فعال.گرہ->is_جڑ()) {
      اگر (فعال.Len > 0) {
        فعال.Len--;
        فعال.Idx = انتہا_Idx - Suffixes_مقصد_جوڑو + 1;
      }
    } ورنہ {
      // Start the next phase at the next smallest suffix.
      فعال.گرہ = فعال.گرہ->getLink();
    }
  }

  واپس Suffixes_مقصد_جوڑو;
}

خالی Suffix_درخت::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = خالی_اشاریہ;

  // Each leaf node represents a repeat of a string.
  Small_سمتیہ<غیرعلامتی> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  جب (!اندرونی_Nodes_مقصد_دورہ.خالی_خانے()) {
    RepeatedSubstringStarts.clear();
    خودکار *Curr = اندرونی_Nodes_مقصد_دورہ.نکالو_عقب_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    غیرعلامتی لمبائی = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    برائے (خودکار &بچہ_Pair : Curr->Children)
      // Save all of this node's children for processing.
      اگر (خودکار *اندرونی_بچہ =
              dyn_cast<Suffix_درخت_اندرونی_گرہ>(بچہ_Pair.دوسرا_2))
        اندرونی_Nodes_مقصد_دورہ.ڈالو_عقب(اندرونی_بچہ);

    // If length of repeated substring is below threshold, then skip it.
    اگر (لمبائی < Min_لمبائی)
      جاری;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    اگر (Curr->is_جڑ())
      جاری;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    اگر (Outliner_برگ_Descendants) {
      برائے (غیرعلامتی I = Curr->get_بایاں_برگ_Idx(); I <= Curr->get_دایاں_برگ_Idx();
           ++I)
        RepeatedSubstringStarts.ڈالو_عقب(برگ_Nodes[I]->getSuffixIdx());
    } ورنہ {
      برائے (خودکار &بچہ_Pair : Curr->Children)
        اگر (خودکار *برگ = dyn_cast<Suffix_درخت_برگ_گرہ>(بچہ_Pair.دوسرا_2))
          RepeatedSubstringStarts.ڈالو_عقب(برگ->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    اگر (RepeatedSubstringStarts.سائز_2() < 2)
      جاری;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.لمبائی = لمبائی;
    llvm::append_range(RS.آغاز_Indices, RepeatedSubstringStarts);
    توڑو;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
