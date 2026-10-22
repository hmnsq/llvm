//===- DeltaTree.cpp - B-Tree for Rewrite Delta tracking ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the DeltaTree and related classes.
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/ADT/DeltaTree.h"
#समावेश "llvm/Support/Casting.h"
#समावेश <cassert>
#समावेश <cstring>

using namespace llvm;

/// The DeltaTree class is a multiway search tree (BTree) structure with some
/// fancy features.  B-Trees are generally more memory and cache efficient
/// than binary trees, because they store multiple keys/values in each node.
///
/// DeltaTree implements a key/value mapping from FileIndex to Delta, allowing
/// fast lookup by FileIndex.  However, an added (important) bonus is that it
/// can also efficiently tell us the full accumulated delta for a specific
/// file offset as well, without traversing the whole tree.
///
/// The nodes of the tree are made up of instances of two classes:
/// DeltaTreeNode and DeltaTreeInteriorNode.  The later subclasses the
/// former and adds children pointers.  Each node knows the full delta of all
/// entries (recursively) contained inside of it, which allows us to get the
/// full delta implied by a whole subtree in constant time.

namespace {

/// SourceDelta - As code in the original input buffer is added and deleted,
/// SourceDelta records are used to keep track of how the input SourceLocation
/// object is mapped into the output buffer.
काष्ठा SourceDelta {
  अचिन्हित FileLoc;
  पूर्णांक Delta;

  अटल SourceDelta लो(अचिन्हित Loc, पूर्णांक D) {
    SourceDelta Delta;
    Delta.FileLoc = Loc;
    Delta.Delta = D;
    वापस Delta;
  }
};

/// DeltaTreeNode - The common part of all nodes.
///
श्रेणी Delta_वृक्ष_गाँठ {
खुला:
  काष्ठा जोड़ो_परिणाम {
    Delta_वृक्ष_गाँठ *LHS, *RHS;
    SourceDelta Split;
  };

निजी:
  मित्र श्रेणी Delta_वृक्ष_Interior_गाँठ;

  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// WidthFactor-1 K/V pairs (except the root) and may have at most
  /// 2*WidthFactor-1 K/V pairs.
  क्रमागत { WidthFactor = 8 };

  /// Values - This tracks the SourceDelta's currently in this node.
  SourceDelta मूल्यावली[2 * WidthFactor - 1];

  /// NumValuesUsed - This tracks the number of values this node currently
  /// holds.
  अचिन्हित अक्षर अंक_मूल्यावली_Used = 0;

  /// IsLeaf - This is true if this is a leaf of the btree.  If false, this is
  /// an interior node, and is actually an instance of DeltaTreeInteriorNode.
  bool IsLeaf;

  /// FullDelta - This is the full delta of all the values in this node and
  /// all children nodes.
  पूर्णांक FullDelta = 0;

खुला:
  Delta_वृक्ष_गाँठ(bool isLeaf = true) : IsLeaf(isLeaf) {}

  bool isLeaf() स्थिर { वापस IsLeaf; }
  पूर्णांक getFullDelta() स्थिर { वापस FullDelta; }
  bool isFull() स्थिर { वापस अंक_मूल्यावली_Used == 2 * WidthFactor - 1; }

  अचिन्हित get_अंक_मूल्यावली_Used() स्थिर { वापस अंक_मूल्यावली_Used; }

  स्थिर SourceDelta &get_मूल्य(अचिन्हित i) स्थिर {
    निश्चित(i < अंक_मूल्यावली_Used && "Invalid value #");
    वापस मूल्यावली[i];
  }

  SourceDelta &get_मूल्य(अचिन्हित i) {
    निश्चित(i < अंक_मूल्यावली_Used && "Invalid value #");
    वापस मूल्यावली[i];
  }

  /// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
  /// this node.  If insertion is easy, do it and return false.  Otherwise,
  /// split the node, populate InsertRes with info about the split, and return
  /// true.
  bool DoInsertion(अचिन्हित File_सूचकांक, पूर्णांक Delta, जोड़ो_परिणाम *जोड़ो_Res);

  व्योम DoSplit(जोड़ो_परिणाम &जोड़ो_Res);

  /// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
  /// local walk over our contained deltas.
  व्योम RecomputeFullDeltaLocally();

  व्योम Destroy();
};

/// DeltaTreeInteriorNode - When isLeaf = false, a node has child pointers.
/// This class tracks them.
श्रेणी Delta_वृक्ष_Interior_गाँठ : खुला Delta_वृक्ष_गाँठ {
  मित्र श्रेणी Delta_वृक्ष_गाँठ;

  Delta_वृक्ष_गाँठ *Children[2 * WidthFactor];

  ~Delta_वृक्ष_Interior_गाँठ() {
    क्रम (अचिन्हित i = 0, e = अंक_मूल्यावली_Used + 1; i != e; ++i)
      Children[i]->Destroy();
  }

खुला:
  Delta_वृक्ष_Interior_गाँठ() : Delta_वृक्ष_गाँठ(false /*nonleaf*/) {}

  Delta_वृक्ष_Interior_गाँठ(स्थिर जोड़ो_परिणाम &IR)
      : Delta_वृक्ष_गाँठ(false /*nonleaf*/) {
    Children[0] = IR.LHS;
    Children[1] = IR.RHS;
    मूल्यावली[0] = IR.Split;
    FullDelta =
        IR.LHS->getFullDelta() + IR.RHS->getFullDelta() + IR.Split.Delta;
    अंक_मूल्यावली_Used = 1;
  }

  स्थिर Delta_वृक्ष_गाँठ *getChild(अचिन्हित i) स्थिर {
    निश्चित(i < get_अंक_मूल्यावली_Used() + 1 && "Invalid child");
    वापस Children[i];
  }

  Delta_वृक्ष_गाँठ *getChild(अचिन्हित i) {
    निश्चित(i < get_अंक_मूल्यावली_Used() + 1 && "Invalid child");
    वापस Children[i];
  }

  अटल bool classof(स्थिर Delta_वृक्ष_गाँठ *N) { वापस !N->isLeaf(); }
};

} // namespace

/// Destroy - A 'virtual' destructor.
व्योम Delta_वृक्ष_गाँठ::Destroy() {
  अगर (isLeaf())
    मिटाओ यह;
  अन्यथा
    मिटाओ cast<Delta_वृक्ष_Interior_गाँठ>(यह);
}

/// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
/// local walk over our contained deltas.
व्योम Delta_वृक्ष_गाँठ::RecomputeFullDeltaLocally() {
  पूर्णांक NewFullDelta = 0;
  क्रम (अचिन्हित i = 0, e = get_अंक_मूल्यावली_Used(); i != e; ++i)
    NewFullDelta += मूल्यावली[i].Delta;
  अगर (स्वतः *IN = dyn_cast<Delta_वृक्ष_Interior_गाँठ>(यह))
    क्रम (अचिन्हित i = 0, e = get_अंक_मूल्यावली_Used() + 1; i != e; ++i)
      NewFullDelta += IN->getChild(i)->getFullDelta();
  FullDelta = NewFullDelta;
}

/// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
/// this node.  If insertion is easy, do it and return false.  Otherwise,
/// split the node, populate InsertRes with info about the split, and return
/// true.
bool Delta_वृक्ष_गाँठ::DoInsertion(अचिन्हित File_सूचकांक, पूर्णांक Delta,
                                जोड़ो_परिणाम *जोड़ो_Res) {
  // Maintain full delta for this node.
  FullDelta += Delta;

  // Find the insertion point, the first delta whose index is >= FileIndex.
  अचिन्हित i = 0, e = get_अंक_मूल्यावली_Used();
  जबतक (i != e && File_सूचकांक > get_मूल्य(i).FileLoc)
    ++i;

  // If we found an a record for exactly this file index, just merge this
  // value into the pre-existing record and finish early.
  अगर (i != e && get_मूल्य(i).FileLoc == File_सूचकांक) {
    // NOTE: Delta could drop to zero here.  This means that the delta entry is
    // useless and could be removed.  Supporting erases is more complex than
    // leaving an entry with Delta=0, so we just leave an entry with Delta=0 in
    // the tree.
    मूल्यावली[i].Delta += Delta;
    वापस false;
  }

  // Otherwise, we found an insertion point, and we know that the value at the
  // specified index is > FileIndex.  Handle the leaf case first.
  अगर (isLeaf()) {
    अगर (!isFull()) {
      // For an insertion into a non-full leaf node, just insert the value in
      // its sorted position.  This requires moving later values over.
      अगर (i != e)
        स_हटाओ(&मूल्यावली[i + 1], &मूल्यावली[i], माप(मूल्यावली[0]) * (e - i));
      मूल्यावली[i] = SourceDelta::लो(File_सूचकांक, Delta);
      ++अंक_मूल्यावली_Used;
      वापस false;
    }

    // Otherwise, if this is leaf is full, split the node at its median, insert
    // the value into one of the children, and return the result.
    निश्चित(जोड़ो_Res && "No result location specified");
    DoSplit(*जोड़ो_Res);

    अगर (जोड़ो_Res->Split.FileLoc > File_सूचकांक)
      जोड़ो_Res->LHS->DoInsertion(File_सूचकांक, Delta, nullptr /*can't fail*/);
    अन्यथा
      जोड़ो_Res->RHS->DoInsertion(File_सूचकांक, Delta, nullptr /*can't fail*/);
    वापस true;
  }

  // Otherwise, this is an interior node.  Send the request down the tree.
  स्वतः *IN = cast<Delta_वृक्ष_Interior_गाँठ>(यह);
  अगर (!IN->Children[i]->DoInsertion(File_सूचकांक, Delta, जोड़ो_Res))
    वापस false; // If there was space in the child, just return.

  // Okay, this split the subtree, producing a new value and two children to
  // insert here.  If this node is non-full, we can just insert it directly.
  अगर (!isFull()) {
    // Now that we have two nodes and a new element, insert the perclated value
    // into ourself by moving all the later values/children down, then inserting
    // the new one.
    अगर (i != e)
      स_हटाओ(&IN->Children[i + 2], &IN->Children[i + 1],
              (e - i) * माप(IN->Children[0]));
    IN->Children[i] = जोड़ो_Res->LHS;
    IN->Children[i + 1] = जोड़ो_Res->RHS;

    अगर (e != i)
      स_हटाओ(&मूल्यावली[i + 1], &मूल्यावली[i], (e - i) * माप(मूल्यावली[0]));
    मूल्यावली[i] = जोड़ो_Res->Split;
    ++अंक_मूल्यावली_Used;
    वापस false;
  }

  // Finally, if this interior node was full and a node is percolated up, split
  // ourself and return that up the chain.  Start by saving all our info to
  // avoid having the split clobber it.
  IN->Children[i] = जोड़ो_Res->LHS;
  Delta_वृक्ष_गाँठ *SubRHS = जोड़ो_Res->RHS;
  SourceDelta SubSplit = जोड़ो_Res->Split;

  // Do the split.
  DoSplit(*जोड़ो_Res);

  // Figure out where to insert SubRHS/NewSplit.
  Delta_वृक्ष_Interior_गाँठ *जोड़ो_Side;
  अगर (SubSplit.FileLoc < जोड़ो_Res->Split.FileLoc)
    जोड़ो_Side = cast<Delta_वृक्ष_Interior_गाँठ>(जोड़ो_Res->LHS);
  अन्यथा
    जोड़ो_Side = cast<Delta_वृक्ष_Interior_गाँठ>(जोड़ो_Res->RHS);

  // We now have a non-empty interior node 'InsertSide' to insert
  // SubRHS/SubSplit into.  Find out where to insert SubSplit.

  // Find the insertion point, the first delta whose index is >SubSplit.FileLoc.
  i = 0;
  e = जोड़ो_Side->get_अंक_मूल्यावली_Used();
  जबतक (i != e && SubSplit.FileLoc > जोड़ो_Side->get_मूल्य(i).FileLoc)
    ++i;

  // Now we know that i is the place to insert the split value into.  Insert it
  // and the child right after it.
  अगर (i != e)
    स_हटाओ(&जोड़ो_Side->Children[i + 2], &जोड़ो_Side->Children[i + 1],
            (e - i) * माप(IN->Children[0]));
  जोड़ो_Side->Children[i + 1] = SubRHS;

  अगर (e != i)
    स_हटाओ(&जोड़ो_Side->मूल्यावली[i + 1], &जोड़ो_Side->मूल्यावली[i],
            (e - i) * माप(मूल्यावली[0]));
  जोड़ो_Side->मूल्यावली[i] = SubSplit;
  ++जोड़ो_Side->अंक_मूल्यावली_Used;
  जोड़ो_Side->FullDelta += SubSplit.Delta + SubRHS->getFullDelta();
  वापस true;
}

/// DoSplit - Split the currently full node (which has 2*WidthFactor-1 values)
/// into two subtrees each with "WidthFactor-1" values and a pivot value.
/// Return the pieces in InsertRes.
व्योम Delta_वृक्ष_गाँठ::DoSplit(जोड़ो_परिणाम &जोड़ो_Res) {
  निश्चित(isFull() && "Why split a non-full node?");

  // Since this node is full, it contains 2*WidthFactor-1 values.  We move
  // the first 'WidthFactor-1' values to the LHS child (which we leave in this
  // node), propagate one value up, and move the last 'WidthFactor-1' values
  // into the RHS child.

  // Create the new child node.
  Delta_वृक्ष_गाँठ *New_गाँठ;
  अगर (स्वतः *IN = dyn_cast<Delta_वृक्ष_Interior_गाँठ>(यह)) {
    // If this is an interior node, also move over 'WidthFactor' children
    // into the new node.
    Delta_वृक्ष_Interior_गाँठ *New = नया Delta_वृक्ष_Interior_गाँठ();
    स_नकल(&New->Children[0], &IN->Children[WidthFactor],
           WidthFactor * माप(IN->Children[0]));
    New_गाँठ = New;
  } अन्यथा {
    // Just create the new leaf node.
    New_गाँठ = नया Delta_वृक्ष_गाँठ();
  }

  // Move over the last 'WidthFactor-1' values from here to NewNode.
  स_नकल(&New_गाँठ->मूल्यावली[0], &मूल्यावली[WidthFactor],
         (WidthFactor - 1) * माप(मूल्यावली[0]));

  // Decrease the number of values in the two nodes.
  New_गाँठ->अंक_मूल्यावली_Used = अंक_मूल्यावली_Used = WidthFactor - 1;

  // Recompute the two nodes' full delta.
  New_गाँठ->RecomputeFullDeltaLocally();
  RecomputeFullDeltaLocally();

  जोड़ो_Res.LHS = यह;
  जोड़ो_Res.RHS = New_गाँठ;
  जोड़ो_Res.Split = मूल्यावली[WidthFactor - 1];
}

//===----------------------------------------------------------------------===//
//                        DeltaTree Implementation
//===----------------------------------------------------------------------===//

// #define VERIFY_TREE

#अगर_घोषित VERIFY_वृक्ष
/// VerifyTree - Walk the btree performing assertions on various properties to
/// verify consistency.  This is useful for debugging new changes to the tree.
अटल व्योम Verify_वृक्ष(स्थिर Delta_वृक्ष_गाँठ *N) {
  स्थिर स्वतः *IN = dyn_cast<Delta_वृक्ष_Interior_गाँठ>(N);
  अगर (IN == 0) {
    // Verify leaves, just ensure that FullDelta matches up and the elements
    // are in proper order.
    पूर्णांक FullDelta = 0;
    क्रम (अचिन्हित i = 0, e = N->get_अंक_मूल्यावली_Used(); i != e; ++i) {
      अगर (i)
        निश्चित(N->get_मूल्य(i - 1).FileLoc < N->get_मूल्य(i).FileLoc);
      FullDelta += N->get_मूल्य(i).Delta;
    }
    निश्चित(FullDelta == N->getFullDelta());
    वापस;
  }

  // Verify interior nodes: Ensure that FullDelta matches up and the
  // elements are in proper order and the children are in proper order.
  पूर्णांक FullDelta = 0;
  क्रम (अचिन्हित i = 0, e = IN->get_अंक_मूल्यावली_Used(); i != e; ++i) {
    स्थिर SourceDelta &IVal = N->get_मूल्य(i);
    स्थिर Delta_वृक्ष_गाँठ *IChild = IN->getChild(i);
    अगर (i)
      निश्चित(IN->get_मूल्य(i - 1).FileLoc < IVal.FileLoc);
    FullDelta += IVal.Delta;
    FullDelta += IChild->getFullDelta();

    // The largest value in child #i should be smaller than FileLoc.
    निश्चित(IChild->get_मूल्य(IChild->get_अंक_मूल्यावली_Used() - 1).FileLoc <
           IVal.FileLoc);

    // The smallest value in child #i+1 should be larger than FileLoc.
    निश्चित(IN->getChild(i + 1)->get_मूल्य(0).FileLoc > IVal.FileLoc);
    Verify_वृक्ष(IChild);
  }

  FullDelta += IN->getChild(IN->get_अंक_मूल्यावली_Used())->getFullDelta();

  निश्चित(FullDelta == N->getFullDelta());
}
#पूर्ण_अगर // VERIFY_TREE

अटल Delta_वृक्ष_गाँठ *get_मूल(व्योम *मूल_2) { वापस (Delta_वृक्ष_गाँठ *)मूल_2; }

Delta_वृक्ष::Delta_वृक्ष() { मूल_2 = नया Delta_वृक्ष_गाँठ(); }

Delta_वृक्ष::Delta_वृक्ष(स्थिर Delta_वृक्ष &RHS) {
  // Currently we only support copying when the RHS is empty.
  निश्चित(get_मूल(RHS.मूल_2)->get_अंक_मूल्यावली_Used() == 0 &&
         "Can only copy empty tree");
  मूल_2 = नया Delta_वृक्ष_गाँठ();
}

Delta_वृक्ष::~Delta_वृक्ष() { get_मूल(मूल_2)->Destroy(); }

/// getDeltaAt - Return the accumulated delta at the specified file offset.
/// This includes all insertions or delections that occurred *before* the
/// specified file index.
पूर्णांक Delta_वृक्ष::getDeltaAt(अचिन्हित File_सूचकांक) स्थिर {
  स्थिर Delta_वृक्ष_गाँठ *गाँठ = get_मूल(मूल_2);

  पूर्णांक परिणाम = 0;

  // Walk down the tree.
  जबतक (true) {
    // For all nodes, include any local deltas before the specified file
    // index by summing them up directly.  Keep track of how many were
    // included.
    अचिन्हित अंक_Vals_Greater = 0;
    क्रम (अचिन्हित e = गाँठ->get_अंक_मूल्यावली_Used(); अंक_Vals_Greater != e;
         ++अंक_Vals_Greater) {
      स्थिर SourceDelta &Val = गाँठ->get_मूल्य(अंक_Vals_Greater);

      अगर (Val.FileLoc >= File_सूचकांक)
        अवरोध;
      परिणाम += Val.Delta;
    }

    // If we have an interior node, include information about children and
    // recurse.  Otherwise, if we have a leaf, we're done.
    स्थिर स्वतः *IN = dyn_cast<Delta_वृक्ष_Interior_गाँठ>(गाँठ);
    अगर (!IN)
      वापस परिणाम;

    // Include any children to the left of the values we skipped, all of
    // their deltas should be included as well.
    क्रम (अचिन्हित i = 0; i != अंक_Vals_Greater; ++i)
      परिणाम += IN->getChild(i)->getFullDelta();

    // If we found exactly the value we were looking for, break off the
    // search early.  There is no need to search the RHS of the value for
    // partial results.
    अगर (अंक_Vals_Greater != गाँठ->get_अंक_मूल्यावली_Used() &&
        गाँठ->get_मूल्य(अंक_Vals_Greater).FileLoc == File_सूचकांक)
      वापस परिणाम + IN->getChild(अंक_Vals_Greater)->getFullDelta();

    // Otherwise, traverse down the tree.  The selected subtree may be
    // partially included in the range.
    गाँठ = IN->getChild(अंक_Vals_Greater);
  }
  // NOT REACHED.
}

/// AddDelta - When a change is made that shifts around the text buffer,
/// this method is used to record that info.  It inserts a delta of 'Delta'
/// into the current DeltaTree at offset FileIndex.
व्योम Delta_वृक्ष::AddDelta(अचिन्हित File_सूचकांक, पूर्णांक Delta) {
  निश्चित(Delta && "Adding a noop?");
  Delta_वृक्ष_गाँठ *My_मूल = get_मूल(मूल_2);

  Delta_वृक्ष_गाँठ::जोड़ो_परिणाम जोड़ो_Res;
  अगर (My_मूल->DoInsertion(File_सूचकांक, Delta, &जोड़ो_Res)) {
    मूल_2 = नया Delta_वृक्ष_Interior_गाँठ(जोड़ो_Res);
#अगर_घोषित VERIFY_वृक्ष
    My_मूल = मूल_2;
#पूर्ण_अगर
  }

#अगर_घोषित VERIFY_वृक्ष
  Verify_वृक्ष(My_मूल);
#पूर्ण_अगर
}
