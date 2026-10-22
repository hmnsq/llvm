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

उपयुज्य नामाकाश llvm;

/// \returns the number of elements in the substring associated with \p N.
स्थिर size_t अङ्कः_Elements_अन्तर्_Substring(ध्रुव Suffix_वृक्षः_ग्रन्थिः *N) {
  प्रतिज्ञा(N && "Got a null node?");
  यदि (स्वचालित *आन्तरिकम् = dyn_cast<Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः>(N))
    यदि (आन्तरिकम्->is_मूलम्())
      प्रत्यागम 0;
  प्रत्यागम N->get_अन्तः_Idx() - N->getStartIdx() + 1;
}

Suffix_वृक्षः::Suffix_वृक्षः(ध्रुव ArrayRef<अचिह्नित> &Str,
                       तार्किक Outliner_पर्णम्_Descendants)
    : Str(Str), Outliner_पर्णम्_Descendants(Outliner_पर्णम्_Descendants) {
  मूलम्_2 = निवेशय_मूलम्();
  सक्रिय.ग्रन्थिः = मूलम्_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  अचिह्नित Suffixes_गन्तव्यम्_सङ्कलय = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  प्रति (अचिह्नित Pfx_अन्तः_Idx = 0, अन्तः_2 = Str.आकार_2(); Pfx_अन्तः_Idx < अन्तः_2; Pfx_अन्तः_Idx++) {
    Suffixes_गन्तव्यम्_सङ्कलय++;
    पर्णम्_अन्तः_Idx = Pfx_अन्तः_Idx; // Extend each of the leaves.
    Suffixes_गन्तव्यम्_सङ्कलय = extend(Pfx_अन्तः_Idx, Suffixes_गन्तव्यम्_सङ्कलय);
  }

  // Set the suffix indices of each leaf.
  प्रतिज्ञा(मूलम्_2 && "Root node can't be nullptr!");
  समुच्चयः_Suffix_Indices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  यदि (Outliner_पर्णम्_Descendants)
    समुच्चयः_पर्णम्_Nodes();
}

Suffix_वृक्षः_ग्रन्थिः *Suffix_वृक्षः::निवेशय_पर्णम्(Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः &Parent,
                                       अचिह्नित StartIdx, अचिह्नित Edge) {
  प्रतिज्ञा(StartIdx <= पर्णम्_अन्तः_Idx && "String can't start after it ends!");
  स्वचालित *N = नूतन (पर्णम्_ग्रन्थिः_Allocator.Allocate())
      Suffix_वृक्षः_पर्णम्_ग्रन्थिः(StartIdx, &पर्णम्_अन्तः_Idx);
  Parent.Children[Edge] = N;
  प्रत्यागम N;
}

Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः *
Suffix_वृक्षः::निवेशय_आन्तरिकम्_ग्रन्थिः(Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः *Parent,
                               अचिह्नित StartIdx, अचिह्नित अन्तः_Idx,
                               अचिह्नित Edge) {
  प्रतिज्ञा(StartIdx <= अन्तः_Idx && "String can't start after it ends!");
  प्रतिज्ञा(!(!Parent && StartIdx != Suffix_वृक्षः_ग्रन्थिः::रिक्तानि_Idx) &&
         "Non-root internal nodes must have parents!");
  स्वचालित *N = नूतन (आन्तरिकम्_ग्रन्थिः_Allocator.Allocate())
      Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः(StartIdx, अन्तः_Idx, मूलम्_2);
  यदि (Parent)
    Parent->Children[Edge] = N;
  प्रत्यागम N;
}

Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः *Suffix_वृक्षः::निवेशय_मूलम्() {
  प्रत्यागम निवेशय_आन्तरिकम्_ग्रन्थिः(/*Parent = */ शून्यसूचकः, Suffix_वृक्षः_ग्रन्थिः::रिक्तानि_Idx,
                            Suffix_वृक्षः_ग्रन्थिः::रिक्तानि_Idx, /*Edge = */ 0);
}

शून्य Suffix_वृक्षः::समुच्चयः_Suffix_Indices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  Small_सदिशम्<std::pair<Suffix_वृक्षः_ग्रन्थिः *, अचिह्नित>> गन्तव्यम्_दर्शय;

  // Current node being visited.
  Suffix_वृक्षः_ग्रन्थिः *Curr_ग्रन्थिः = मूलम्_2;

  // Sum of the lengths of the nodes down the path to the current one.
  अचिह्नित Curr_ग्रन्थिः_Len = 0;
  गन्तव्यम्_दर्शय.प्रक्षिप_पृष्ठभागः({Curr_ग्रन्थिः, Curr_ग्रन्थिः_Len});
  यावत् (!गन्तव्यम्_दर्शय.रिक्तानि()) {
    std::tie(Curr_ग्रन्थिः, Curr_ग्रन्थिः_Len) = गन्तव्यम्_दर्शय.निष्कासय_पृष्ठभागः_val();
    // Length of the current node from the root down to here.
    Curr_ग्रन्थिः->समुच्चयः_Concat_Len(Curr_ग्रन्थिः_Len);
    यदि (स्वचालित *आन्तरिकम्_ग्रन्थिः = dyn_cast<Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः>(Curr_ग्रन्थिः))
      प्रति (स्वचालित &ChildPair : आन्तरिकम्_ग्रन्थिः->Children) {
        प्रतिज्ञा(ChildPair.second && "Node had a null child!");
        गन्तव्यम्_दर्शय.प्रक्षिप_पृष्ठभागः(
            {ChildPair.second,
             Curr_ग्रन्थिः_Len + अङ्कः_Elements_अन्तर्_Substring(ChildPair.second)});
      }
    // No children, so we are at the end of the string.
    यदि (स्वचालित *पर्णम्_ग्रन्थिः = dyn_cast<Suffix_वृक्षः_पर्णम्_ग्रन्थिः>(Curr_ग्रन्थिः))
      पर्णम्_ग्रन्थिः->समुच्चयः_Suffix_Idx(Str.आकार_2() - Curr_ग्रन्थिः_Len);
  }
}

शून्य Suffix_वृक्षः::समुच्चयः_पर्णम्_Nodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  Small_सदिशम्<Suffix_वृक्षः_ग्रन्थिः *> गन्तव्यम्_दर्शय;
  गन्तव्यम्_दर्शय.प्रक्षिप_पृष्ठभागः(मूलम्_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  अचिह्नित पर्णम्_गणक = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः *,
           std::pair<Suffix_वृक्षः_ग्रन्थिः *, Suffix_वृक्षः_ग्रन्थिः *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  यावत् (!गन्तव्यम्_दर्शय.रिक्तानि()) {
    Suffix_वृक्षः_ग्रन्थिः *Curr_ग्रन्थिः = गन्तव्यम्_दर्शय.निष्कासय_पृष्ठभागः_val();
    यदि (स्वचालित *Curr_आन्तरिकम्_ग्रन्थिः = dyn_cast<Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः>(Curr_ग्रन्थिः)) {
      // The current node is an internal node.
      स्वचालित I = ChildrenMap.find(Curr_आन्तरिकम्_ग्रन्थिः);
      यदि (I == ChildrenMap.अन्तः()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        स्वचालित J = Curr_आन्तरिकम्_ग्रन्थिः->Children.begin();
        यदि (J != Curr_आन्तरिकम्_ग्रन्थिः->Children.अन्तः()) {
          गन्तव्यम्_दर्शय.प्रक्षिप_पृष्ठभागः(Curr_ग्रन्थिः);
          Suffix_वृक्षः_ग्रन्थिः *FirstChild = J->second;
          Suffix_वृक्षः_ग्रन्थिः *चरमः_Child = शून्यसूचकः;
          प्रति (; J != Curr_आन्तरिकम्_ग्रन्थिः->Children.अन्तः(); ++J) {
            चरमः_Child = J->second;
            गन्तव्यम्_दर्शय.प्रक्षिप_पृष्ठभागः(चरमः_Child);
          }
          ChildrenMap[Curr_आन्तरिकम्_ग्रन्थिः] = {FirstChild, चरमः_Child};
        }
      } अन्यथा {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        स्वचालित [FirstChild, चरमः_Child] = I->second;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_ग्रन्थिः->समुच्चयः_दक्षिणः_पर्णम्_Idx(FirstChild->get_दक्षिणः_पर्णम्_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_ग्रन्थिः->समुच्चयः_वामः_पर्णम्_Idx(चरमः_Child->get_वामः_पर्णम्_Idx());
        प्रतिज्ञा(Curr_ग्रन्थिः->get_वामः_पर्णम्_Idx() <= Curr_ग्रन्थिः->get_दक्षिणः_पर्णम्_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } अन्यथा {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_ग्रन्थिः->समुच्चयः_वामः_पर्णम्_Idx(पर्णम्_गणक);
      Curr_ग्रन्थिः->समुच्चयः_दक्षिणः_पर्णम्_Idx(पर्णम्_गणक);
      ++पर्णम्_गणक;
      स्वचालित *Curr_पर्णम्_ग्रन्थिः = cast<Suffix_वृक्षः_पर्णम्_ग्रन्थिः>(Curr_ग्रन्थिः);
      पर्णम्_Nodes.प्रक्षिप_पृष्ठभागः(Curr_पर्णम्_ग्रन्थिः);
    }
  }
}

अचिह्नित Suffix_वृक्षः::extend(अचिह्नित अन्तः_Idx, अचिह्नित Suffixes_गन्तव्यम्_सङ्कलय) {
  Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः *NeedsLink = शून्यसूचकः;

  यावत् (Suffixes_गन्तव्यम्_सङ्कलय > 0) {

    // Are we waiting to add anything other than just the last character?
    यदि (सक्रिय.Len == 0) {
      // If not, then say the active index is the end index.
      सक्रिय.Idx = अन्तः_Idx;
    }

    प्रतिज्ञा(सक्रिय.Idx <= अन्तः_Idx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    अचिह्नित FirstChar = Str[सक्रिय.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    यदि (स्वचालित It = सक्रिय.ग्रन्थिः->Children.find(FirstChar);
        It == सक्रिय.ग्रन्थिः->Children.अन्तः()) {
      // If not, then we can just insert a leaf and move to the next step.
      निवेशय_पर्णम्(*सक्रिय.ग्रन्थिः, अन्तः_Idx, FirstChar);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      यदि (NeedsLink) {
        NeedsLink->समुच्चयः_Link(सक्रिय.ग्रन्थिः);
        NeedsLink = शून्यसूचकः;
      }
    } अन्यथा {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_वृक्षः_ग्रन्थिः *अग्र_ग्रन्थिः = It->second;

      अचिह्नित SubstringLen = अङ्कः_Elements_अन्तर्_Substring(अग्र_ग्रन्थिः);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      यदि (सक्रिय.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        प्रतिज्ञा(isa<Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः>(अग्र_ग्रन्थिः) &&
               "Expected an internal node?");
        सक्रिय.Idx += SubstringLen;
        सक्रिय.Len -= SubstringLen;
        सक्रिय.ग्रन्थिः = cast<Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः>(अग्र_ग्रन्थिः);
        अनुवर्त;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      अचिह्नित चरमः_Char = Str[अन्तः_Idx];

      // Is the string we're trying to insert a substring of the next node?
      यदि (Str[अग्र_ग्रन्थिः->getStartIdx() + सक्रिय.Len] == चरमः_Char) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        यदि (NeedsLink && !सक्रिय.ग्रन्थिः->is_मूलम्()) {
          NeedsLink->समुच्चयः_Link(सक्रिय.ग्रन्थिः);
          NeedsLink = शून्यसूचकः;
        }

        सक्रिय.Len++;
        भङ्ग;
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
      Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः *विभज_ग्रन्थिः = निवेशय_आन्तरिकम्_ग्रन्थिः(
          सक्रिय.ग्रन्थिः, अग्र_ग्रन्थिः->getStartIdx(),
          अग्र_ग्रन्थिः->getStartIdx() + सक्रिय.Len - 1, FirstChar);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      निवेशय_पर्णम्(*विभज_ग्रन्थिः, अन्तः_Idx, चरमः_Char);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      अग्र_ग्रन्थिः->incrementStartIdx(सक्रिय.Len);
      विभज_ग्रन्थिः->Children[Str[अग्र_ग्रन्थिः->getStartIdx()]] = अग्र_ग्रन्थिः;

      // SplitNode is an internal node, update the suffix link.
      यदि (NeedsLink)
        NeedsLink->समुच्चयः_Link(विभज_ग्रन्थिः);

      NeedsLink = विभज_ग्रन्थिः;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    Suffixes_गन्तव्यम्_सङ्कलय--;

    यदि (सक्रिय.ग्रन्थिः->is_मूलम्()) {
      यदि (सक्रिय.Len > 0) {
        सक्रिय.Len--;
        सक्रिय.Idx = अन्तः_Idx - Suffixes_गन्तव्यम्_सङ्कलय + 1;
      }
    } अन्यथा {
      // Start the next phase at the next smallest suffix.
      सक्रिय.ग्रन्थिः = सक्रिय.ग्रन्थिः->getLink();
    }
  }

  प्रत्यागम Suffixes_गन्तव्यम्_सङ्कलय;
}

शून्य Suffix_वृक्षः::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = शून्यसूचकः;

  // Each leaf node represents a repeat of a string.
  Small_सदिशम्<अचिह्नित> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  यावत् (!आन्तरिकम्_Nodes_गन्तव्यम्_दर्शय.रिक्तानि()) {
    RepeatedSubstringStarts.clear();
    स्वचालित *Curr = आन्तरिकम्_Nodes_गन्तव्यम्_दर्शय.निष्कासय_पृष्ठभागः_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    अचिह्नित दीर्घता = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    प्रति (स्वचालित &ChildPair : Curr->Children)
      // Save all of this node's children for processing.
      यदि (स्वचालित *आन्तरिकम्_Child =
              dyn_cast<Suffix_वृक्षः_आन्तरिकम्_ग्रन्थिः>(ChildPair.second))
        आन्तरिकम्_Nodes_गन्तव्यम्_दर्शय.प्रक्षिप_पृष्ठभागः(आन्तरिकम्_Child);

    // If length of repeated substring is below threshold, then skip it.
    यदि (दीर्घता < Min_दीर्घता)
      अनुवर्त;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    यदि (Curr->is_मूलम्())
      अनुवर्त;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    यदि (Outliner_पर्णम्_Descendants) {
      प्रति (अचिह्नित I = Curr->get_वामः_पर्णम्_Idx(); I <= Curr->get_दक्षिणः_पर्णम्_Idx();
           ++I)
        RepeatedSubstringStarts.प्रक्षिप_पृष्ठभागः(पर्णम्_Nodes[I]->getSuffixIdx());
    } अन्यथा {
      प्रति (स्वचालित &ChildPair : Curr->Children)
        यदि (स्वचालित *पर्णम् = dyn_cast<Suffix_वृक्षः_पर्णम्_ग्रन्थिः>(ChildPair.second))
          RepeatedSubstringStarts.प्रक्षिप_पृष्ठभागः(पर्णम्->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    यदि (RepeatedSubstringStarts.आकार_2() < 2)
      अनुवर्त;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.दीर्घता = दीर्घता;
    llvm::append_range(RS.StartIndices, RepeatedSubstringStarts);
    भङ्ग;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
