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

کارول نطاق llvm;

/// \returns the number of elements in the substring associated with \p N.
ساكن size_t عدد_Elements_دننه_Substring(ثابت Suffix_ونه_غوټه *N) {
  أكد(N && "Got a null node?");
  إذا (خپلکاری *دننی = dyn_cast<Suffix_ونه_دننی_غوټه>(N))
    إذا (دننی->is_ریښه())
      أرجع 0;
  أرجع N->get_پای_ټکی_Idx() - N->get_پیل_Idx() + 1;
}

Suffix_ونه::Suffix_ونه(ثابت ArrayRef<بینښه> &Str,
                       منطقي Outliner_ونې_پاڼه_Descendants)
    : Str(Str), Outliner_ونې_پاڼه_Descendants(Outliner_ونې_پاڼه_Descendants) {
  ریښه_2 = ورننباسه_ریښه();
  فعال.غوټه = ریښه_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  بینښه Suffixes_منزل_جمع_کړه = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  لكل (بینښه Pfx_پای_ټکی_Idx = 0, پای_ټکی_2 = Str.اندازه_2(); Pfx_پای_ټکی_Idx < پای_ټکی_2; Pfx_پای_ټکی_Idx++) {
    Suffixes_منزل_جمع_کړه++;
    ونې_پاڼه_پای_ټکی_Idx = Pfx_پای_ټکی_Idx; // Extend each of the leaves.
    Suffixes_منزل_جمع_کړه = extend(Pfx_پای_ټکی_Idx, Suffixes_منزل_جمع_کړه);
  }

  // Set the suffix indices of each leaf.
  أكد(ریښه_2 && "Root node can't be nullptr!");
  سېټ_Suffix_Indices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  إذا (Outliner_ونې_پاڼه_Descendants)
    سېټ_ونې_پاڼه_Nodes();
}

Suffix_ونه_غوټه *Suffix_ونه::ورننباسه_ونې_پاڼه(Suffix_ونه_دننی_غوټه &پلار_2,
                                       بینښه پیل_Idx, بینښه څنډه) {
  أكد(پیل_Idx <= ونې_پاڼه_پای_ټکی_Idx && "String can't start after it ends!");
  خپلکاری *N = جديد (ونې_پاڼه_غوټه_Allocator.Allocate())
      Suffix_ونه_ونې_پاڼه_غوټه(پیل_Idx, &ونې_پاڼه_پای_ټکی_Idx);
  پلار_2.Children[څنډه] = N;
  أرجع N;
}

Suffix_ونه_دننی_غوټه *
Suffix_ونه::ورننباسه_دننی_غوټه(Suffix_ونه_دننی_غوټه *پلار_2,
                               بینښه پیل_Idx, بینښه پای_ټکی_Idx,
                               بینښه څنډه) {
  أكد(پیل_Idx <= پای_ټکی_Idx && "String can't start after it ends!");
  أكد(!(!پلار_2 && پیل_Idx != Suffix_ونه_غوټه::تشه_Idx) &&
         "Non-root internal nodes must have parents!");
  خپلکاری *N = جديد (دننی_غوټه_Allocator.Allocate())
      Suffix_ونه_دننی_غوټه(پیل_Idx, پای_ټکی_Idx, ریښه_2);
  إذا (پلار_2)
    پلار_2->Children[څنډه] = N;
  أرجع N;
}

Suffix_ونه_دننی_غوټه *Suffix_ونه::ورننباسه_ریښه() {
  أرجع ورننباسه_دننی_غوټه(/*Parent = */ تش_اشاره, Suffix_ونه_غوټه::تشه_Idx,
                            Suffix_ونه_غوټه::تشه_Idx, /*Edge = */ 0);
}

فراغ Suffix_ونه::سېټ_Suffix_Indices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  Small_ویکټور<std::pair<Suffix_ونه_غوټه *, بینښه>> منزل_لیدنه;

  // Current node being visited.
  Suffix_ونه_غوټه *Curr_غوټه = ریښه_2;

  // Sum of the lengths of the nodes down the path to the current one.
  بینښه Curr_غوټه_Len = 0;
  منزل_لیدنه.ټېل_شا({Curr_غوټه, Curr_غوټه_Len});
  طالما (!منزل_لیدنه.تشه()) {
    std::tie(Curr_غوټه, Curr_غوټه_Len) = منزل_لیدنه.راوباسه_شا_val();
    // Length of the current node from the root down to here.
    Curr_غوټه->سېټ_Concat_Len(Curr_غوټه_Len);
    إذا (خپلکاری *دننی_غوټه = dyn_cast<Suffix_ونه_دننی_غوټه>(Curr_غوټه))
      لكل (خپلکاری &ماشوم_Pair : دننی_غوټه->Children) {
        أكد(ماشوم_Pair.دوهم_2 && "Node had a null child!");
        منزل_لیدنه.ټېل_شا(
            {ماشوم_Pair.دوهم_2,
             Curr_غوټه_Len + عدد_Elements_دننه_Substring(ماشوم_Pair.دوهم_2)});
      }
    // No children, so we are at the end of the string.
    إذا (خپلکاری *ونې_پاڼه_غوټه = dyn_cast<Suffix_ونه_ونې_پاڼه_غوټه>(Curr_غوټه))
      ونې_پاڼه_غوټه->سېټ_Suffix_Idx(Str.اندازه_2() - Curr_غوټه_Len);
  }
}

فراغ Suffix_ونه::سېټ_ونې_پاڼه_Nodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  Small_ویکټور<Suffix_ونه_غوټه *> منزل_لیدنه;
  منزل_لیدنه.ټېل_شا(ریښه_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  بینښه ونې_پاڼه_Counter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_ونه_دننی_غوټه *,
           std::pair<Suffix_ونه_غوټه *, Suffix_ونه_غوټه *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  طالما (!منزل_لیدنه.تشه()) {
    Suffix_ونه_غوټه *Curr_غوټه = منزل_لیدنه.راوباسه_شا_val();
    إذا (خپلکاری *Curr_دننی_غوټه = dyn_cast<Suffix_ونه_دننی_غوټه>(Curr_غوټه)) {
      // The current node is an internal node.
      خپلکاری I = ChildrenMap.پیدا_کړه(Curr_دننی_غوټه);
      إذا (I == ChildrenMap.پای_ټکی()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        خپلکاری J = Curr_دننی_غوټه->Children.begin();
        إذا (J != Curr_دننی_غوټه->Children.پای_ټکی()) {
          منزل_لیدنه.ټېل_شا(Curr_غوټه);
          Suffix_ونه_غوټه *لومړی_ماشوم = J->دوهم_2;
          Suffix_ونه_غوټه *وروستی_ماشوم = تش_اشاره;
          لكل (; J != Curr_دننی_غوټه->Children.پای_ټکی(); ++J) {
            وروستی_ماشوم = J->دوهم_2;
            منزل_لیدنه.ټېل_شا(وروستی_ماشوم);
          }
          ChildrenMap[Curr_دننی_غوټه] = {لومړی_ماشوم, وروستی_ماشوم};
        }
      } وإلا {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        خپلکاری [لومړی_ماشوم, وروستی_ماشوم] = I->دوهم_2;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_غوټه->سېټ_ښی_ونې_پاڼه_Idx(لومړی_ماشوم->get_ښی_ونې_پاڼه_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_غوټه->سېټ_کیڼ_ونې_پاڼه_Idx(وروستی_ماشوم->get_کیڼ_ونې_پاڼه_Idx());
        أكد(Curr_غوټه->get_کیڼ_ونې_پاڼه_Idx() <= Curr_غوټه->get_ښی_ونې_پاڼه_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } وإلا {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_غوټه->سېټ_کیڼ_ونې_پاڼه_Idx(ونې_پاڼه_Counter);
      Curr_غوټه->سېټ_ښی_ونې_پاڼه_Idx(ونې_پاڼه_Counter);
      ++ونې_پاڼه_Counter;
      خپلکاری *Curr_ونې_پاڼه_غوټه = cast<Suffix_ونه_ونې_پاڼه_غوټه>(Curr_غوټه);
      ونې_پاڼه_Nodes.ټېل_شا(Curr_ونې_پاڼه_غوټه);
    }
  }
}

بینښه Suffix_ونه::extend(بینښه پای_ټکی_Idx, بینښه Suffixes_منزل_جمع_کړه) {
  Suffix_ونه_دننی_غوټه *NeedsLink = تش_اشاره;

  طالما (Suffixes_منزل_جمع_کړه > 0) {

    // Are we waiting to add anything other than just the last character?
    إذا (فعال.Len == 0) {
      // If not, then say the active index is the end index.
      فعال.Idx = پای_ټکی_Idx;
    }

    أكد(فعال.Idx <= پای_ټکی_Idx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    بینښه لومړی_Char = Str[فعال.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    إذا (خپلکاری It = فعال.غوټه->Children.پیدا_کړه(لومړی_Char);
        It == فعال.غوټه->Children.پای_ټکی()) {
      // If not, then we can just insert a leaf and move to the next step.
      ورننباسه_ونې_پاڼه(*فعال.غوټه, پای_ټکی_Idx, لومړی_Char);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      إذا (NeedsLink) {
        NeedsLink->سېټ_Link(فعال.غوټه);
        NeedsLink = تش_اشاره;
      }
    } وإلا {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_ونه_غوټه *بل_غوټه = It->دوهم_2;

      بینښه SubstringLen = عدد_Elements_دننه_Substring(بل_غوټه);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      إذا (فعال.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        أكد(isa<Suffix_ونه_دننی_غوټه>(بل_غوټه) &&
               "Expected an internal node?");
        فعال.Idx += SubstringLen;
        فعال.Len -= SubstringLen;
        فعال.غوټه = cast<Suffix_ونه_دننی_غوټه>(بل_غوټه);
        استمر;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      بینښه وروستی_Char = Str[پای_ټکی_Idx];

      // Is the string we're trying to insert a substring of the next node?
      إذا (Str[بل_غوټه->get_پیل_Idx() + فعال.Len] == وروستی_Char) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        إذا (NeedsLink && !فعال.غوټه->is_ریښه()) {
          NeedsLink->سېټ_Link(فعال.غوټه);
          NeedsLink = تش_اشاره;
        }

        فعال.Len++;
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
      Suffix_ونه_دننی_غوټه *وېشل_غوټه = ورننباسه_دننی_غوټه(
          فعال.غوټه, بل_غوټه->get_پیل_Idx(),
          بل_غوټه->get_پیل_Idx() + فعال.Len - 1, لومړی_Char);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      ورننباسه_ونې_پاڼه(*وېشل_غوټه, پای_ټکی_Idx, وروستی_Char);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      بل_غوټه->increment_پیل_Idx(فعال.Len);
      وېشل_غوټه->Children[Str[بل_غوټه->get_پیل_Idx()]] = بل_غوټه;

      // SplitNode is an internal node, update the suffix link.
      إذا (NeedsLink)
        NeedsLink->سېټ_Link(وېشل_غوټه);

      NeedsLink = وېشل_غوټه;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    Suffixes_منزل_جمع_کړه--;

    إذا (فعال.غوټه->is_ریښه()) {
      إذا (فعال.Len > 0) {
        فعال.Len--;
        فعال.Idx = پای_ټکی_Idx - Suffixes_منزل_جمع_کړه + 1;
      }
    } وإلا {
      // Start the next phase at the next smallest suffix.
      فعال.غوټه = فعال.غوټه->getLink();
    }
  }

  أرجع Suffixes_منزل_جمع_کړه;
}

فراغ Suffix_ونه::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = تش_اشاره;

  // Each leaf node represents a repeat of a string.
  Small_ویکټور<بینښه> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  طالما (!دننی_Nodes_منزل_لیدنه.تشه()) {
    RepeatedSubstringStarts.clear();
    خپلکاری *Curr = دننی_Nodes_منزل_لیدنه.راوباسه_شا_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    بینښه اوږدتیا = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    لكل (خپلکاری &ماشوم_Pair : Curr->Children)
      // Save all of this node's children for processing.
      إذا (خپلکاری *دننی_ماشوم =
              dyn_cast<Suffix_ونه_دننی_غوټه>(ماشوم_Pair.دوهم_2))
        دننی_Nodes_منزل_لیدنه.ټېل_شا(دننی_ماشوم);

    // If length of repeated substring is below threshold, then skip it.
    إذا (اوږدتیا < Min_اوږدتیا)
      استمر;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    إذا (Curr->is_ریښه())
      استمر;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    إذا (Outliner_ونې_پاڼه_Descendants) {
      لكل (بینښه I = Curr->get_کیڼ_ونې_پاڼه_Idx(); I <= Curr->get_ښی_ونې_پاڼه_Idx();
           ++I)
        RepeatedSubstringStarts.ټېل_شا(ونې_پاڼه_Nodes[I]->getSuffixIdx());
    } وإلا {
      لكل (خپلکاری &ماشوم_Pair : Curr->Children)
        إذا (خپلکاری *ونې_پاڼه = dyn_cast<Suffix_ونه_ونې_پاڼه_غوټه>(ماشوم_Pair.دوهم_2))
          RepeatedSubstringStarts.ټېل_شا(ونې_پاڼه->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    إذا (RepeatedSubstringStarts.اندازه_2() < 2)
      استمر;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.اوږدتیا = اوږدتیا;
    llvm::append_range(RS.پیل_Indices, RepeatedSubstringStarts);
    اكسر;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
