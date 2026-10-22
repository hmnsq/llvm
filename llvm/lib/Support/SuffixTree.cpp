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

#అన్తర్భావ "llvm/Support/SuffixTree.h"
#అన్తర్భావ "llvm/Support/Allocator.h"
#అన్తర్భావ "llvm/Support/Casting.h"
#అన్తర్భావ "llvm/Support/SuffixTreeNode.h"

ఉపయోగించి నామాకాశ llvm;

/// \returns the number of elements in the substring associated with \p N.
స్థిర size_t అంకె_Elements_లోపల_Substring(స్థిరరాశి Suffix_వృక్షం_కణుపు *N) {
  ప్రతిజ్ఞా(N && "Got a null node?");
  ఒకవేళ (స్వయంచాలక *అంతర్గత = dyn_cast<Suffix_వృక్షం_అంతర్గత_కణుపు>(N))
    ఒకవేళ (అంతర్గత->is_మూలం())
      తిరిగి 0;
  తిరిగి N->get_చివర_Idx() - N->get_ప్రారంభం_Idx() + 1;
}

Suffix_వృక్షం::Suffix_వృక్షం(స్థిరరాశి ArrayRef<గుర్తులేని> &Str,
                       తార్కిక Outliner_ఆకు_Descendants)
    : Str(Str), Outliner_ఆకు_Descendants(Outliner_ఆకు_Descendants) {
  మూలం_2 = చొప్పించు_మూలం();
  చురుకైన.కణుపు = మూలం_2;

  // Keep track of the number of suffixes we have to add of the current
  // prefix.
  గుర్తులేని Suffixes_గమ్యం_కూడు = 0;

  // Construct the suffix tree iteratively on each prefix of the string.
  // PfxEndIdx is the end index of the current prefix.
  // End is one past the last element in the string.
  కొరకు (గుర్తులేని Pfx_చివర_Idx = 0, చివర_2 = Str.కొలత_2(); Pfx_చివర_Idx < చివర_2; Pfx_చివర_Idx++) {
    Suffixes_గమ్యం_కూడు++;
    ఆకు_చివర_Idx = Pfx_చివర_Idx; // Extend each of the leaves.
    Suffixes_గమ్యం_కూడు = extend(Pfx_చివర_Idx, Suffixes_గమ్యం_కూడు);
  }

  // Set the suffix indices of each leaf.
  ప్రతిజ్ఞా(మూలం_2 && "Root node can't be nullptr!");
  సమితి_Suffix_Indices();

  // Collect all leaf nodes of the suffix tree. And for each internal node,
  // record the range of leaf nodes that are descendants of it.
  ఒకవేళ (Outliner_ఆకు_Descendants)
    సమితి_ఆకు_Nodes();
}

Suffix_వృక్షం_కణుపు *Suffix_వృక్షం::చొప్పించు_ఆకు(Suffix_వృక్షం_అంతర్గత_కణుపు &మాతృ_2,
                                       గుర్తులేని ప్రారంభం_Idx, గుర్తులేని అంచు) {
  ప్రతిజ్ఞా(ప్రారంభం_Idx <= ఆకు_చివర_Idx && "String can't start after it ends!");
  స్వయంచాలక *N = కొత్త (ఆకు_కణుపు_Allocator.Allocate())
      Suffix_వృక్షం_ఆకు_కణుపు(ప్రారంభం_Idx, &ఆకు_చివర_Idx);
  మాతృ_2.Children[అంచు] = N;
  తిరిగి N;
}

Suffix_వృక్షం_అంతర్గత_కణుపు *
Suffix_వృక్షం::చొప్పించు_అంతర్గత_కణుపు(Suffix_వృక్షం_అంతర్గత_కణుపు *మాతృ_2,
                               గుర్తులేని ప్రారంభం_Idx, గుర్తులేని చివర_Idx,
                               గుర్తులేని అంచు) {
  ప్రతిజ్ఞా(ప్రారంభం_Idx <= చివర_Idx && "String can't start after it ends!");
  ప్రతిజ్ఞా(!(!మాతృ_2 && ప్రారంభం_Idx != Suffix_వృక్షం_కణుపు::ఖాళీలు_Idx) &&
         "Non-root internal nodes must have parents!");
  స్వయంచాలక *N = కొత్త (అంతర్గత_కణుపు_Allocator.Allocate())
      Suffix_వృక్షం_అంతర్గత_కణుపు(ప్రారంభం_Idx, చివర_Idx, మూలం_2);
  ఒకవేళ (మాతృ_2)
    మాతృ_2->Children[అంచు] = N;
  తిరిగి N;
}

Suffix_వృక్షం_అంతర్గత_కణుపు *Suffix_వృక్షం::చొప్పించు_మూలం() {
  తిరిగి చొప్పించు_అంతర్గత_కణుపు(/*Parent = */ శూన్యసూచిక, Suffix_వృక్షం_కణుపు::ఖాళీలు_Idx,
                            Suffix_వృక్షం_కణుపు::ఖాళీలు_Idx, /*Edge = */ 0);
}

శూన్య Suffix_వృక్షం::సమితి_Suffix_Indices() {
  // List of nodes we need to visit along with the current length of the
  // string.
  Small_సదిశ<std::pair<Suffix_వృక్షం_కణుపు *, గుర్తులేని>> గమ్యం_సందర్శించు;

  // Current node being visited.
  Suffix_వృక్షం_కణుపు *Curr_కణుపు = మూలం_2;

  // Sum of the lengths of the nodes down the path to the current one.
  గుర్తులేని Curr_కణుపు_Len = 0;
  గమ్యం_సందర్శించు.నెట్టు_వెనుక({Curr_కణుపు, Curr_కణుపు_Len});
  అయితే (!గమ్యం_సందర్శించు.ఖాళీలు()) {
    std::tie(Curr_కణుపు, Curr_కణుపు_Len) = గమ్యం_సందర్శించు.తీయి_వెనుక_val();
    // Length of the current node from the root down to here.
    Curr_కణుపు->సమితి_Concat_Len(Curr_కణుపు_Len);
    ఒకవేళ (స్వయంచాలక *అంతర్గత_కణుపు = dyn_cast<Suffix_వృక్షం_అంతర్గత_కణుపు>(Curr_కణుపు))
      కొరకు (స్వయంచాలక &బిడ్డ_Pair : అంతర్గత_కణుపు->Children) {
        ప్రతిజ్ఞా(బిడ్డ_Pair.రెండవ_2 && "Node had a null child!");
        గమ్యం_సందర్శించు.నెట్టు_వెనుక(
            {బిడ్డ_Pair.రెండవ_2,
             Curr_కణుపు_Len + అంకె_Elements_లోపల_Substring(బిడ్డ_Pair.రెండవ_2)});
      }
    // No children, so we are at the end of the string.
    ఒకవేళ (స్వయంచాలక *ఆకు_కణుపు = dyn_cast<Suffix_వృక్షం_ఆకు_కణుపు>(Curr_కణుపు))
      ఆకు_కణుపు->సమితి_Suffix_Idx(Str.కొలత_2() - Curr_కణుపు_Len);
  }
}

శూన్య Suffix_వృక్షం::సమితి_ఆకు_Nodes() {
  // A stack that keeps track of nodes to visit for post-order DFS traversal.
  Small_సదిశ<Suffix_వృక్షం_కణుపు *> గమ్యం_సందర్శించు;
  గమ్యం_సందర్శించు.నెట్టు_వెనుక(మూలం_2);

  // This keeps track of the index of the next leaf node to be added to
  // the LeafNodes vector of the suffix tree.
  గుర్తులేని ఆకు_Counter = 0;

  // This keeps track of nodes whose children have been added to the stack.
  // The value is a pair, representing a node's first and last children.
  DenseMap<Suffix_వృక్షం_అంతర్గత_కణుపు *,
           std::pair<Suffix_వృక్షం_కణుపు *, Suffix_వృక్షం_కణుపు *>>
      ChildrenMap;

  // Traverse the tree in post-order.
  అయితే (!గమ్యం_సందర్శించు.ఖాళీలు()) {
    Suffix_వృక్షం_కణుపు *Curr_కణుపు = గమ్యం_సందర్శించు.తీయి_వెనుక_val();
    ఒకవేళ (స్వయంచాలక *Curr_అంతర్గత_కణుపు = dyn_cast<Suffix_వృక్షం_అంతర్గత_కణుపు>(Curr_కణుపు)) {
      // The current node is an internal node.
      స్వయంచాలక I = ChildrenMap.కనుగొను(Curr_అంతర్గత_కణుపు);
      ఒకవేళ (I == ChildrenMap.చివర()) {
        // This is the first time we visit this node.
        // Its children have not been added to the stack yet.
        // We add current node back, and add its children to the stack.
        // We keep track of the first and last children of the current node.
        స్వయంచాలక J = Curr_అంతర్గత_కణుపు->Children.begin();
        ఒకవేళ (J != Curr_అంతర్గత_కణుపు->Children.చివర()) {
          గమ్యం_సందర్శించు.నెట్టు_వెనుక(Curr_కణుపు);
          Suffix_వృక్షం_కణుపు *మొదటి_బిడ్డ = J->రెండవ_2;
          Suffix_వృక్షం_కణుపు *చివరి_బిడ్డ = శూన్యసూచిక;
          కొరకు (; J != Curr_అంతర్గత_కణుపు->Children.చివర(); ++J) {
            చివరి_బిడ్డ = J->రెండవ_2;
            గమ్యం_సందర్శించు.నెట్టు_వెనుక(చివరి_బిడ్డ);
          }
          ChildrenMap[Curr_అంతర్గత_కణుపు] = {మొదటి_బిడ్డ, చివరి_బిడ్డ};
        }
      } లేకపోతే {
        // This is the second time we visit this node.
        // All of its children have already been processed.
        // Now, we can set its LeftLeafIdx and RightLeafIdx;
        స్వయంచాలక [మొదటి_బిడ్డ, చివరి_బిడ్డ] = I->రెండవ_2;
        // Get the first child to use its RightLeafIdx.
        // The first child is the first one added to the stack, so it is
        // the last one to be processed. Hence, the leaf descendants
        // of the first child are assigned the largest index numbers.
        Curr_కణుపు->సమితి_కుడి_ఆకు_Idx(మొదటి_బిడ్డ->get_కుడి_ఆకు_Idx());
        // Get the last child to use its LeftLeafIdx.
        Curr_కణుపు->సమితి_ఎడమ_ఆకు_Idx(చివరి_బిడ్డ->get_ఎడమ_ఆకు_Idx());
        ప్రతిజ్ఞా(Curr_కణుపు->get_ఎడమ_ఆకు_Idx() <= Curr_కణుపు->get_కుడి_ఆకు_Idx() &&
               "LeftLeafIdx should not be larger than RightLeafIdx");
      }
    } లేకపోతే {
      // The current node is a leaf node.
      // We can simply set its LeftLeafIdx and RightLeafIdx.
      Curr_కణుపు->సమితి_ఎడమ_ఆకు_Idx(ఆకు_Counter);
      Curr_కణుపు->సమితి_కుడి_ఆకు_Idx(ఆకు_Counter);
      ++ఆకు_Counter;
      స్వయంచాలక *Curr_ఆకు_కణుపు = cast<Suffix_వృక్షం_ఆకు_కణుపు>(Curr_కణుపు);
      ఆకు_Nodes.నెట్టు_వెనుక(Curr_ఆకు_కణుపు);
    }
  }
}

గుర్తులేని Suffix_వృక్షం::extend(గుర్తులేని చివర_Idx, గుర్తులేని Suffixes_గమ్యం_కూడు) {
  Suffix_వృక్షం_అంతర్గత_కణుపు *NeedsLink = శూన్యసూచిక;

  అయితే (Suffixes_గమ్యం_కూడు > 0) {

    // Are we waiting to add anything other than just the last character?
    ఒకవేళ (చురుకైన.Len == 0) {
      // If not, then say the active index is the end index.
      చురుకైన.Idx = చివర_Idx;
    }

    ప్రతిజ్ఞా(చురుకైన.Idx <= చివర_Idx && "Start index can't be after end index!");

    // The first character in the current substring we're looking at.
    గుర్తులేని మొదటి_Char = Str[చురుకైన.Idx];

    // Have we inserted anything starting with FirstChar at the current node?
    ఒకవేళ (స్వయంచాలక It = చురుకైన.కణుపు->Children.కనుగొను(మొదటి_Char);
        It == చురుకైన.కణుపు->Children.చివర()) {
      // If not, then we can just insert a leaf and move to the next step.
      చొప్పించు_ఆకు(*చురుకైన.కణుపు, చివర_Idx, మొదటి_Char);

      // The active node is an internal node, and we visited it, so it must
      // need a link if it doesn't have one.
      ఒకవేళ (NeedsLink) {
        NeedsLink->సమితి_Link(చురుకైన.కణుపు);
        NeedsLink = శూన్యసూచిక;
      }
    } లేకపోతే {
      // There's a match with FirstChar, so look for the point in the tree to
      // insert a new node.
      Suffix_వృక్షం_కణుపు *తదుపరి_కణుపు = It->రెండవ_2;

      గుర్తులేని SubstringLen = అంకె_Elements_లోపల_Substring(తదుపరి_కణుపు);

      // Is the current suffix we're trying to insert longer than the size of
      // the child we want to move to?
      ఒకవేళ (చురుకైన.Len >= SubstringLen) {
        // If yes, then consume the characters we've seen and move to the next
        // node.
        ప్రతిజ్ఞా(isa<Suffix_వృక్షం_అంతర్గత_కణుపు>(తదుపరి_కణుపు) &&
               "Expected an internal node?");
        చురుకైన.Idx += SubstringLen;
        చురుకైన.Len -= SubstringLen;
        చురుకైన.కణుపు = cast<Suffix_వృక్షం_అంతర్గత_కణుపు>(తదుపరి_కణుపు);
        కొనసాగు;
      }

      // Otherwise, the suffix we're trying to insert must be contained in the
      // next node we want to move to.
      గుర్తులేని చివరి_Char = Str[చివర_Idx];

      // Is the string we're trying to insert a substring of the next node?
      ఒకవేళ (Str[తదుపరి_కణుపు->get_ప్రారంభం_Idx() + చురుకైన.Len] == చివరి_Char) {
        // If yes, then we're done for this step. Remember our insertion point
        // and move to the next end index. At this point, we have an implicit
        // suffix tree.
        ఒకవేళ (NeedsLink && !చురుకైన.కణుపు->is_మూలం()) {
          NeedsLink->సమితి_Link(చురుకైన.కణుపు);
          NeedsLink = శూన్యసూచిక;
        }

        చురుకైన.Len++;
        ఆపు;
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
      Suffix_వృక్షం_అంతర్గత_కణుపు *విభజించు_కణుపు = చొప్పించు_అంతర్గత_కణుపు(
          చురుకైన.కణుపు, తదుపరి_కణుపు->get_ప్రారంభం_Idx(),
          తదుపరి_కణుపు->get_ప్రారంభం_Idx() + చురుకైన.Len - 1, మొదటి_Char);

      // Insert the new node representing the new substring into the tree as
      // a child of the split node. This is the node l from the diagram.
      చొప్పించు_ఆకు(*విభజించు_కణుపు, చివర_Idx, చివరి_Char);

      // Make the old node a child of the split node and update its start
      // index. This is the node n from the diagram.
      తదుపరి_కణుపు->increment_ప్రారంభం_Idx(చురుకైన.Len);
      విభజించు_కణుపు->Children[Str[తదుపరి_కణుపు->get_ప్రారంభం_Idx()]] = తదుపరి_కణుపు;

      // SplitNode is an internal node, update the suffix link.
      ఒకవేళ (NeedsLink)
        NeedsLink->సమితి_Link(విభజించు_కణుపు);

      NeedsLink = విభజించు_కణుపు;
    }

    // We've added something new to the tree, so there's one less suffix to
    // add.
    Suffixes_గమ్యం_కూడు--;

    ఒకవేళ (చురుకైన.కణుపు->is_మూలం()) {
      ఒకవేళ (చురుకైన.Len > 0) {
        చురుకైన.Len--;
        చురుకైన.Idx = చివర_Idx - Suffixes_గమ్యం_కూడు + 1;
      }
    } లేకపోతే {
      // Start the next phase at the next smallest suffix.
      చురుకైన.కణుపు = చురుకైన.కణుపు->getLink();
    }
  }

  తిరిగి Suffixes_గమ్యం_కూడు;
}

శూన్య Suffix_వృక్షం::RepeatedSubstringIterator::advance() {
  // Clear the current state. If we're at the end of the range, then this
  // is the state we want to be in.
  RS = RepeatedSubstring();
  N = శూన్యసూచిక;

  // Each leaf node represents a repeat of a string.
  Small_సదిశ<గుర్తులేని> RepeatedSubstringStarts;

  // Continue visiting nodes until we find one which repeats more than once.
  అయితే (!అంతర్గత_Nodes_గమ్యం_సందర్శించు.ఖాళీలు()) {
    RepeatedSubstringStarts.clear();
    స్వయంచాలక *Curr = అంతర్గత_Nodes_గమ్యం_సందర్శించు.తీయి_వెనుక_val();

    // Keep track of the length of the string associated with the node. If
    // it's too short, we'll quit.
    గుర్తులేని నిడివి = Curr->getConcatLen();

    // Iterate over each child, saving internal nodes for visiting.
    // Internal nodes represent individual strings, which may repeat.
    కొరకు (స్వయంచాలక &బిడ్డ_Pair : Curr->Children)
      // Save all of this node's children for processing.
      ఒకవేళ (స్వయంచాలక *అంతర్గత_బిడ్డ =
              dyn_cast<Suffix_వృక్షం_అంతర్గత_కణుపు>(బిడ్డ_Pair.రెండవ_2))
        అంతర్గత_Nodes_గమ్యం_సందర్శించు.నెట్టు_వెనుక(అంతర్గత_బిడ్డ);

    // If length of repeated substring is below threshold, then skip it.
    ఒకవేళ (నిడివి < Min_నిడివి)
      కొనసాగు;

    // The root never represents a repeated substring. If we're looking at
    // that, then skip it.
    ఒకవేళ (Curr->is_మూలం())
      కొనసాగు;

    // Collect leaf children or leaf descendants by OutlinerLeafDescendants.
    ఒకవేళ (Outliner_ఆకు_Descendants) {
      కొరకు (గుర్తులేని I = Curr->get_ఎడమ_ఆకు_Idx(); I <= Curr->get_కుడి_ఆకు_Idx();
           ++I)
        RepeatedSubstringStarts.నెట్టు_వెనుక(ఆకు_Nodes[I]->getSuffixIdx());
    } లేకపోతే {
      కొరకు (స్వయంచాలక &బిడ్డ_Pair : Curr->Children)
        ఒకవేళ (స్వయంచాలక *ఆకు = dyn_cast<Suffix_వృక్షం_ఆకు_కణుపు>(బిడ్డ_Pair.రెండవ_2))
          RepeatedSubstringStarts.నెట్టు_వెనుక(ఆకు->getSuffixIdx());
    }

    // Do we have any repeated substrings?
    ఒకవేళ (RepeatedSubstringStarts.కొలత_2() < 2)
      కొనసాగు;

    // Yes. Update the state to reflect this, and then bail out.
    N = Curr;
    RS.నిడివి = నిడివి;
    llvm::append_range(RS.ప్రారంభం_Indices, RepeatedSubstringStarts);
    ఆపు;
  }
  // At this point, either NewRS is an empty RepeatedSubstring, or it was
  // set in the above loop. Similarly, N is either nullptr, or the node
  // associated with NewRS.
}
