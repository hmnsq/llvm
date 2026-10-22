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

استفاده نطاق llvm;

/// \returns the number of elements in the substring associated with \p N.
ساكن size_t رقم_Elements_درون_Substring(ثابت Suffix_درخت_گره *N) {
  أكد(N && "Got a null node?");
  إذا (خودکار *داخلی = dyn_cast<Suffix_درخت_داخلی_گره>(N))
    إذا (داخلی->is_ریشه())
      أرجع 0;
  أرجع N->get_انتها_Idx() - N->get_شروع_Idx() + 1;
}

Suffix_درخت::Suffix_درخت(ثابت ArrayRef<بیعلامت> &Str,
                       منطقي Outliner_برگ_Descendants)
    : Str(Str), Outliner_برگ_Descendants(Outliner_برگ_Descendants) {
  ریشه_2 = درج_ریشه();
  فعال.گره = ریشه_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  بیعلامت Suffixes_مقصد_جمع_کن = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  لكل (بیعلامت Pfx_انتها_Idx = 0, انتها_2 = Str.اندازه_2(); Pfx_انتها_Idx < انتها_2; Pfx_انتها_Idx++) {
    Suffixes_مقصد_جمع_کن++;
    برگ_انتها_Idx = Pfx_انتها_Idx; // Extend each of the leaves.
    Suffixes_مقصد_جمع_کن = extend(Pfx_انتها_Idx, Suffixes_مقصد_جمع_کن);
  }

  // Set the suffix indices of each leaf.
  أكد(ریشه_2 && "Root node can't be nullptr!");
  مجموعه_Suffix_Indices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  إذا (Outliner_برگ_Descendants)
    مجموعه_برگ_Nodes();
}

Suffix_درخت_گره *Suffix_درخت::درج_برگ(Suffix_درخت_داخلی_گره &والد_2,
                                       بیعلامت شروع_Idx, بیعلامت یال) {
  أكد(شروع_Idx <= برگ_انتها_Idx && "String can't start after it ends!");
  خودکار *N = جديد (برگ_گره_Allocator.Allocate())
      Suffix_درخت_برگ_گره(شروع_Idx, &برگ_انتها_Idx);
  والد_2.Children[یال] = N;
  أرجع N;
}

Suffix_درخت_داخلی_گره *
Suffix_درخت::درج_داخلی_گره(Suffix_درخت_داخلی_گره *والد_2,
                               بیعلامت شروع_Idx, بیعلامت انتها_Idx,
                               بیعلامت یال) {
  أكد(شروع_Idx <= انتها_Idx && "String can't start after it ends!");
  أكد(!(!والد_2 && شروع_Idx != Suffix_درخت_گره::خالی_Idx) &&
         "Non-root internal nodes must have parents!");
  خودکار *N = جديد (داخلی_گره_Allocator.Allocate())
      Suffix_درخت_داخلی_گره(شروع_Idx, انتها_Idx, ریشه_2);
  إذا (والد_2)
    والد_2->Children[یال] = N;
  أرجع N;
}

Suffix_درخت_داخلی_گره *Suffix_درخت::درج_ریشه() {
  أرجع درج_داخلی_گره(/*Parent = */ اشاره_تهی, Suffix_درخت_گره::خالی_Idx,
                            Suffix_درخت_گره::خالی_Idx, /*Edge = */ 0);
}

فراغ Suffix_درخت::مجموعه_Suffix_Indices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  Small_وکتور<std::pair<Suffix_درخت_گره *, بیعلامت>> مقصد_بازدید;

  // Current node being visited.
  Suffix_درخت_گره *Curr_گره = ریشه_2;

  // Sum of the lengths of the nodes down the path to the current one.
  بیعلامت Curr_گره_Len = 0;
  مقصد_بازدید.افزودن_پشت({Curr_گره, Curr_گره_Len});
  طالما (!مقصد_بازدید.خالی()) {
    std::tie(Curr_گره, Curr_گره_Len) = مقصد_بازدید.برداشت_پشت_val();
    // Length of the current node from the root down to here.
    Curr_گره->مجموعه_Concat_Len(Curr_گره_Len);
    إذا (خودکار *داخلی_گره = dyn_cast<Suffix_درخت_داخلی_گره>(Curr_گره))
      لكل (خودکار &فرزند_Pair : داخلی_گره->Children) {
        أكد(فرزند_Pair.دومی_2 && "Node had a null child!");
        مقصد_بازدید.افزودن_پشت(
            {فرزند_Pair.دومی_2,
             Curr_گره_Len + رقم_Elements_درون_Substring(فرزند_Pair.دومی_2)});
      }
    // No children, so we are at the end of the string.
    إذا (خودکار *برگ_گره = dyn_cast<Suffix_درخت_برگ_گره>(Curr_گره))
      برگ_گره->مجموعه_Suffix_Idx(Str.اندازه_2() - Curr_گره_Len);
  }
}

فراغ Suffix_درخت::مجموعه_برگ_Nodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  Small_وکتور<Suffix_درخت_گره *> مقصد_بازدید;
  مقصد_بازدید.افزودن_پشت(ریشه_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  بیعلامت برگ_Counter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_درخت_داخلی_گره *,
           std::pair<Suffix_درخت_گره *, Suffix_درخت_گره *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  طالما (!مقصد_بازدید.خالی()) {
    Suffix_درخت_گره *Curr_گره = مقصد_بازدید.برداشت_پشت_val();
    إذا (خودکار *Curr_داخلی_گره = dyn_cast<Suffix_درخت_داخلی_گره>(Curr_گره)) {
      // The current node is an internal node.
      خودکار I = ChildrenMap.پیدا_کن(Curr_داخلی_گره);
      إذا (I == ChildrenMap.انتها()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        خودکار J = Curr_داخلی_گره->Children.begin();
        إذا (J != Curr_داخلی_گره->Children.انتها()) {
          مقصد_بازدید.افزودن_پشت(Curr_گره);
          Suffix_درخت_گره *اولی_فرزند = J->دومی_2;
          Suffix_درخت_گره *آخرین_فرزند = اشاره_تهی;
          لكل (; J != Curr_داخلی_گره->Children.انتها(); ++J) {
            آخرین_فرزند = J->دومی_2;
            مقصد_بازدید.افزودن_پشت(آخرین_فرزند);
          }
          ChildrenMap[Curr_داخلی_گره] = {اولی_فرزند, آخرین_فرزند};
        }
      } وإلا {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        خودکار [اولی_فرزند, آخرین_فرزند] = I->دومی_2;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_گره->مجموعه_راست_برگ_Idx(اولی_فرزند->get_راست_برگ_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_گره->مجموعه_چپ_برگ_Idx(آخرین_فرزند->get_چپ_برگ_Idx());
        أكد(Curr_گره->get_چپ_برگ_Idx() <= Curr_گره->get_راست_برگ_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } وإلا {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_گره->مجموعه_چپ_برگ_Idx(برگ_Counter);
      Curr_گره->مجموعه_راست_برگ_Idx(برگ_Counter);
      ++برگ_Counter;
      خودکار *Curr_برگ_گره = cast<Suffix_درخت_برگ_گره>(Curr_گره);
      برگ_Nodes.افزودن_پشت(Curr_برگ_گره);
    }
  }
}

بیعلامت Suffix_درخت::extend(بیعلامت انتها_Idx, بیعلامت Suffixes_مقصد_جمع_کن) {
  Suffix_درخت_داخلی_گره *NeedsLink = اشاره_تهی;

  طالما (Suffixes_مقصد_جمع_کن > 0) {

    // Are we waiting to add anything other than just the last character?
    إذا (فعال.Len == 0) {
      // If not, then say the active index is the end index.
      فعال.Idx = انتها_Idx;
    }

    أكد(فعال.Idx <= انتها_Idx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    بیعلامت اولی_Char = Str[فعال.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    إذا (خودکار It = فعال.گره->Children.پیدا_کن(اولی_Char);
        It == فعال.گره->Children.انتها()) {
      // If not, then we can just insert a leaf and move to the next step.
      درج_برگ(*فعال.گره, انتها_Idx, اولی_Char);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      إذا (NeedsLink) {
        NeedsLink->مجموعه_Link(فعال.گره);
        NeedsLink = اشاره_تهی;
      }
    } وإلا {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_درخت_گره *بعدی_گره = It->دومی_2;

      بیعلامت SubstringLen = رقم_Elements_درون_Substring(بعدی_گره);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      إذا (فعال.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        أكد(isa<Suffix_درخت_داخلی_گره>(بعدی_گره) &&
               "Expected an internal node?");
        فعال.Idx += SubstringLen;
        فعال.Len -= SubstringLen;
        فعال.گره = cast<Suffix_درخت_داخلی_گره>(بعدی_گره);
        استمر;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      بیعلامت آخرین_Char = Str[انتها_Idx];

      // Is the string we're trying to insert a substring of the next node?
      إذا (Str[بعدی_گره->get_شروع_Idx() + فعال.Len] == آخرین_Char) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        إذا (NeedsLink && !فعال.گره->is_ریشه()) {
          NeedsLink->مجموعه_Link(فعال.گره);
          NeedsLink = اشاره_تهی;
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
      Suffix_درخت_داخلی_گره *تقسیم_گره = درج_داخلی_گره(
          فعال.گره, بعدی_گره->get_شروع_Idx(),
          بعدی_گره->get_شروع_Idx() + فعال.Len - 1, اولی_Char);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      درج_برگ(*تقسیم_گره, انتها_Idx, آخرین_Char);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      بعدی_گره->increment_شروع_Idx(فعال.Len);
      تقسیم_گره->Children[Str[بعدی_گره->get_شروع_Idx()]] = بعدی_گره;

      // SplitNode is an internal node, update the suffix link.
      إذا (NeedsLink)
        NeedsLink->مجموعه_Link(تقسیم_گره);

      NeedsLink = تقسیم_گره;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    Suffixes_مقصد_جمع_کن--;

    إذا (فعال.گره->is_ریشه()) {
      إذا (فعال.Len > 0) {
        فعال.Len--;
        فعال.Idx = انتها_Idx - Suffixes_مقصد_جمع_کن + 1;
      }
    } وإلا {
      // Start the next phase at the next smallest suffix.
      فعال.گره = فعال.گره->getLink();
    }
  }

  أرجع Suffixes_مقصد_جمع_کن;
}

فراغ Suffix_درخت::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = اشاره_تهی;

  // Each leaf node represents a repeat of a string.
  Small_وکتور<بیعلامت> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  طالما (!داخلی_Nodes_مقصد_بازدید.خالی()) {
    RepeatedSubstringStarts.clear();
    خودکار *Curr = داخلی_Nodes_مقصد_بازدید.برداشت_پشت_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    بیعلامت درازی = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    لكل (خودکار &فرزند_Pair : Curr->Children)
      // Save all of this node's children for processing.
      إذا (خودکار *داخلی_فرزند =
              dyn_cast<Suffix_درخت_داخلی_گره>(فرزند_Pair.دومی_2))
        داخلی_Nodes_مقصد_بازدید.افزودن_پشت(داخلی_فرزند);

    // If length of repeated substring is below threshold, then skip it.
    إذا (درازی < Min_درازی)
      استمر;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    إذا (Curr->is_ریشه())
      استمر;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    إذا (Outliner_برگ_Descendants) {
      لكل (بیعلامت I = Curr->get_چپ_برگ_Idx(); I <= Curr->get_راست_برگ_Idx();
           ++I)
        RepeatedSubstringStarts.افزودن_پشت(برگ_Nodes[I]->getSuffixIdx());
    } وإلا {
      لكل (خودکار &فرزند_Pair : Curr->Children)
        إذا (خودکار *برگ = dyn_cast<Suffix_درخت_برگ_گره>(فرزند_Pair.دومی_2))
          RepeatedSubstringStarts.افزودن_پشت(برگ->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    إذا (RepeatedSubstringStarts.اندازه_2() < 2)
      استمر;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.درازی = درازی;
    llvm::append_range(RS.شروع_Indices, RepeatedSubstringStarts);
    اكسر;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
