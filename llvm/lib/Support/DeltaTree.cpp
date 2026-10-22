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

#અન્તર્ભાવ "llvm/ADT/DeltaTree.h"
#અન્તર્ભાવ "llvm/Support/Casting.h"
#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <cstring>

using નામાકાશ llvm;

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

નામાકાશ {

/// SourceDelta - As code in the original input buffer is added and deleted,
/// SourceDelta records are used to keep track of how the input SourceLocation
/// object is mapped into the output buffer.
સંરચના SourceDelta {
  unsigned FileLoc;
  પૂર્ણાંક Delta;

  સ્થિર SourceDelta get(unsigned Loc, પૂર્ણાંક D) {
    SourceDelta Delta;
    Delta.FileLoc = Loc;
    Delta.Delta = D;
    પાછા Delta;
  }
};

/// DeltaTreeNode - The common part of all nodes.
///
વર્ગ Delta_વૃક્ષ_ગાંઠ {
જાહેર:
  સંરચના ઉમેરો_Result {
    Delta_વૃક્ષ_ગાંઠ *LHS, *RHS;
    SourceDelta Split;
  };

ખાનગી:
  friend વર્ગ Delta_વૃક્ષ_Interior_ગાંઠ;

  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// WidthFactor-1 K/V pairs (except the root) and may have at most
  /// 2*WidthFactor-1 K/V pairs.
  ગણના { WidthFactor = 8 };

  /// Values - This tracks the SourceDelta's currently in this node.
  SourceDelta કિંમતો[2 * WidthFactor - 1];

  /// NumValuesUsed - This tracks the number of values this node currently
  /// holds.
  unsigned અક્ષર અંક_કિંમતો_Used = 0;

  /// IsLeaf - This is true if this is a leaf of the btree.  If false, this is
  /// an interior node, and is actually an instance of DeltaTreeInteriorNode.
  તાર્કિક IsLeaf;

  /// FullDelta - This is the full delta of all the values in this node and
  /// all children nodes.
  પૂર્ણાંક FullDelta = 0;

જાહેર:
  Delta_વૃક્ષ_ગાંઠ(તાર્કિક isLeaf = સાચું) : IsLeaf(isLeaf) {}

  તાર્કિક isLeaf() અચલ { પાછા IsLeaf; }
  પૂર્ણાંક getFullDelta() અચલ { પાછા FullDelta; }
  તાર્કિક isFull() અચલ { પાછા અંક_કિંમતો_Used == 2 * WidthFactor - 1; }

  unsigned get_અંક_કિંમતો_Used() અચલ { પાછા અંક_કિંમતો_Used; }

  અચલ SourceDelta &get_કિંમત(unsigned i) અચલ {
    પ્રતિજ્ઞા(i < અંક_કિંમતો_Used && "Invalid value #");
    પાછા કિંમતો[i];
  }

  SourceDelta &get_કિંમત(unsigned i) {
    પ્રતિજ્ઞા(i < અંક_કિંમતો_Used && "Invalid value #");
    પાછા કિંમતો[i];
  }

  /// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
  /// this node.  If insertion is easy, do it and return false.  Otherwise,
  /// split the node, populate InsertRes with info about the split, and return
  /// true.
  તાર્કિક DoInsertion(unsigned FileIndex, પૂર્ણાંક Delta, ઉમેરો_Result *ઉમેરો_Res);

  શૂન્ય DoSplit(ઉમેરો_Result &ઉમેરો_Res);

  /// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
  /// local walk over our contained deltas.
  શૂન્ય RecomputeFullDeltaLocally();

  શૂન્ય Destroy();
};

/// DeltaTreeInteriorNode - When isLeaf = false, a node has child pointers.
/// This class tracks them.
વર્ગ Delta_વૃક્ષ_Interior_ગાંઠ : જાહેર Delta_વૃક્ષ_ગાંઠ {
  friend વર્ગ Delta_વૃક્ષ_ગાંઠ;

  Delta_વૃક્ષ_ગાંઠ *Children[2 * WidthFactor];

  ~Delta_વૃક્ષ_Interior_ગાંઠ() {
    માટે (unsigned i = 0, e = અંક_કિંમતો_Used + 1; i != e; ++i)
      Children[i]->Destroy();
  }

જાહેર:
  Delta_વૃક્ષ_Interior_ગાંઠ() : Delta_વૃક્ષ_ગાંઠ(ખોટું /*nonleaf*/) {}

  Delta_વૃક્ષ_Interior_ગાંઠ(અચલ ઉમેરો_Result &IR)
      : Delta_વૃક્ષ_ગાંઠ(ખોટું /*nonleaf*/) {
    Children[0] = IR.LHS;
    Children[1] = IR.RHS;
    કિંમતો[0] = IR.Split;
    FullDelta =
        IR.LHS->getFullDelta() + IR.RHS->getFullDelta() + IR.Split.Delta;
    અંક_કિંમતો_Used = 1;
  }

  અચલ Delta_વૃક્ષ_ગાંઠ *getChild(unsigned i) અચલ {
    પ્રતિજ્ઞા(i < get_અંક_કિંમતો_Used() + 1 && "Invalid child");
    પાછા Children[i];
  }

  Delta_વૃક્ષ_ગાંઠ *getChild(unsigned i) {
    પ્રતિજ્ઞા(i < get_અંક_કિંમતો_Used() + 1 && "Invalid child");
    પાછા Children[i];
  }

  સ્થિર તાર્કિક classof(અચલ Delta_વૃક્ષ_ગાંઠ *N) { પાછા !N->isLeaf(); }
};

} // namespace

/// Destroy - A 'virtual' destructor.
શૂન્ય Delta_વૃક્ષ_ગાંઠ::Destroy() {
  જો (isLeaf())
    delete આ;
  નહીંતર
    delete cast<Delta_વૃક્ષ_Interior_ગાંઠ>(આ);
}

/// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
/// local walk over our contained deltas.
શૂન્ય Delta_વૃક્ષ_ગાંઠ::RecomputeFullDeltaLocally() {
  પૂર્ણાંક NewFullDelta = 0;
  માટે (unsigned i = 0, e = get_અંક_કિંમતો_Used(); i != e; ++i)
    NewFullDelta += કિંમતો[i].Delta;
  જો (auto *IN = dyn_cast<Delta_વૃક્ષ_Interior_ગાંઠ>(આ))
    માટે (unsigned i = 0, e = get_અંક_કિંમતો_Used() + 1; i != e; ++i)
      NewFullDelta += IN->getChild(i)->getFullDelta();
  FullDelta = NewFullDelta;
}

/// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
/// this node.  If insertion is easy, do it and return false.  Otherwise,
/// split the node, populate InsertRes with info about the split, and return
/// true.
તાર્કિક Delta_વૃક્ષ_ગાંઠ::DoInsertion(unsigned FileIndex, પૂર્ણાંક Delta,
                                ઉમેરો_Result *ઉમેરો_Res) {
  // Maintain full delta for this node.
  FullDelta += Delta;

  // Find the insertion point, the first delta whose index is >= FileIndex.
  unsigned i = 0, e = get_અંક_કિંમતો_Used();
  જ્યારે (i != e && FileIndex > get_કિંમત(i).FileLoc)
    ++i;

  // If we found an a record for exactly this file index, just merge this
  // value into the pre-existing record and finish early.
  જો (i != e && get_કિંમત(i).FileLoc == FileIndex) {
    // NOTE: Delta could drop to zero here.  This means that the delta entry is
    // useless and could be removed.  Supporting erases is more complex than
    // leaving an entry with Delta=0, so we just leave an entry with Delta=0 in
    // the tree.
    કિંમતો[i].Delta += Delta;
    પાછા ખોટું;
  }

  // Otherwise, we found an insertion point, and we know that the value at the
  // specified index is > FileIndex.  Handle the leaf case first.
  જો (isLeaf()) {
    જો (!isFull()) {
      // For an insertion into a non-full leaf node, just insert the value in
      // its sorted position.  This requires moving later values over.
      જો (i != e)
        memmove(&કિંમતો[i + 1], &કિંમતો[i], કદ(કિંમતો[0]) * (e - i));
      કિંમતો[i] = SourceDelta::get(FileIndex, Delta);
      ++અંક_કિંમતો_Used;
      પાછા ખોટું;
    }

    // Otherwise, if this is leaf is full, split the node at its median, insert
    // the value into one of the children, and return the result.
    પ્રતિજ્ઞા(ઉમેરો_Res && "No result location specified");
    DoSplit(*ઉમેરો_Res);

    જો (ઉમેરો_Res->Split.FileLoc > FileIndex)
      ઉમેરો_Res->LHS->DoInsertion(FileIndex, Delta, nullptr /*can't fail*/);
    નહીંતર
      ઉમેરો_Res->RHS->DoInsertion(FileIndex, Delta, nullptr /*can't fail*/);
    પાછા સાચું;
  }

  // Otherwise, this is an interior node.  Send the request down the tree.
  auto *IN = cast<Delta_વૃક્ષ_Interior_ગાંઠ>(આ);
  જો (!IN->Children[i]->DoInsertion(FileIndex, Delta, ઉમેરો_Res))
    પાછા ખોટું; // If there was space in the child, just return.

  // Okay, this split the subtree, producing a new value and two children to
  // insert here.  If this node is non-full, we can just insert it directly.
  જો (!isFull()) {
    // Now that we have two nodes and a new element, insert the perclated value
    // into ourself by moving all the later values/children down, then inserting
    // the new one.
    જો (i != e)
      memmove(&IN->Children[i + 2], &IN->Children[i + 1],
              (e - i) * કદ(IN->Children[0]));
    IN->Children[i] = ઉમેરો_Res->LHS;
    IN->Children[i + 1] = ઉમેરો_Res->RHS;

    જો (e != i)
      memmove(&કિંમતો[i + 1], &કિંમતો[i], (e - i) * કદ(કિંમતો[0]));
    કિંમતો[i] = ઉમેરો_Res->Split;
    ++અંક_કિંમતો_Used;
    પાછા ખોટું;
  }

  // Finally, if this interior node was full and a node is percolated up, split
  // ourself and return that up the chain.  Start by saving all our info to
  // avoid having the split clobber it.
  IN->Children[i] = ઉમેરો_Res->LHS;
  Delta_વૃક્ષ_ગાંઠ *SubRHS = ઉમેરો_Res->RHS;
  SourceDelta SubSplit = ઉમેરો_Res->Split;

  // Do the split.
  DoSplit(*ઉમેરો_Res);

  // Figure out where to insert SubRHS/NewSplit.
  Delta_વૃક્ષ_Interior_ગાંઠ *ઉમેરો_Side;
  જો (SubSplit.FileLoc < ઉમેરો_Res->Split.FileLoc)
    ઉમેરો_Side = cast<Delta_વૃક્ષ_Interior_ગાંઠ>(ઉમેરો_Res->LHS);
  નહીંતર
    ઉમેરો_Side = cast<Delta_વૃક્ષ_Interior_ગાંઠ>(ઉમેરો_Res->RHS);

  // We now have a non-empty interior node 'InsertSide' to insert
  // SubRHS/SubSplit into.  Find out where to insert SubSplit.

  // Find the insertion point, the first delta whose index is >SubSplit.FileLoc.
  i = 0;
  e = ઉમેરો_Side->get_અંક_કિંમતો_Used();
  જ્યારે (i != e && SubSplit.FileLoc > ઉમેરો_Side->get_કિંમત(i).FileLoc)
    ++i;

  // Now we know that i is the place to insert the split value into.  Insert it
  // and the child right after it.
  જો (i != e)
    memmove(&ઉમેરો_Side->Children[i + 2], &ઉમેરો_Side->Children[i + 1],
            (e - i) * કદ(IN->Children[0]));
  ઉમેરો_Side->Children[i + 1] = SubRHS;

  જો (e != i)
    memmove(&ઉમેરો_Side->કિંમતો[i + 1], &ઉમેરો_Side->કિંમતો[i],
            (e - i) * કદ(કિંમતો[0]));
  ઉમેરો_Side->કિંમતો[i] = SubSplit;
  ++ઉમેરો_Side->અંક_કિંમતો_Used;
  ઉમેરો_Side->FullDelta += SubSplit.Delta + SubRHS->getFullDelta();
  પાછા સાચું;
}

/// DoSplit - Split the currently full node (which has 2*WidthFactor-1 values)
/// into two subtrees each with "WidthFactor-1" values and a pivot value.
/// Return the pieces in InsertRes.
શૂન્ય Delta_વૃક્ષ_ગાંઠ::DoSplit(ઉમેરો_Result &ઉમેરો_Res) {
  પ્રતિજ્ઞા(isFull() && "Why split a non-full node?");

  // Since this node is full, it contains 2*WidthFactor-1 values.  We move
  // the first 'WidthFactor-1' values to the LHS child (which we leave in this
  // node), propagate one value up, and move the last 'WidthFactor-1' values
  // into the RHS child.

  // Create the new child node.
  Delta_વૃક્ષ_ગાંઠ *New_ગાંઠ;
  જો (auto *IN = dyn_cast<Delta_વૃક્ષ_Interior_ગાંઠ>(આ)) {
    // If this is an interior node, also move over 'WidthFactor' children
    // into the new node.
    Delta_વૃક્ષ_Interior_ગાંઠ *New = નવું Delta_વૃક્ષ_Interior_ગાંઠ();
    memcpy(&New->Children[0], &IN->Children[WidthFactor],
           WidthFactor * કદ(IN->Children[0]));
    New_ગાંઠ = New;
  } નહીંતર {
    // Just create the new leaf node.
    New_ગાંઠ = નવું Delta_વૃક્ષ_ગાંઠ();
  }

  // Move over the last 'WidthFactor-1' values from here to NewNode.
  memcpy(&New_ગાંઠ->કિંમતો[0], &કિંમતો[WidthFactor],
         (WidthFactor - 1) * કદ(કિંમતો[0]));

  // Decrease the number of values in the two nodes.
  New_ગાંઠ->અંક_કિંમતો_Used = અંક_કિંમતો_Used = WidthFactor - 1;

  // Recompute the two nodes' full delta.
  New_ગાંઠ->RecomputeFullDeltaLocally();
  RecomputeFullDeltaLocally();

  ઉમેરો_Res.LHS = આ;
  ઉમેરો_Res.RHS = New_ગાંઠ;
  ઉમેરો_Res.Split = કિંમતો[WidthFactor - 1];
}

//===----------------------------------------------------------------------===//
//                        DeltaTree Implementation
//===----------------------------------------------------------------------===//

// #define VERIFY_TREE

#ifdef VERIFY_વૃક્ષ
/// VerifyTree - Walk the btree performing assertions on various properties to
/// verify consistency.  This is useful for debugging new changes to the tree.
સ્થિર શૂન્ય Verify_વૃક્ષ(અચલ Delta_વૃક્ષ_ગાંઠ *N) {
  અચલ auto *IN = dyn_cast<Delta_વૃક્ષ_Interior_ગાંઠ>(N);
  જો (IN == 0) {
    // Verify leaves, just ensure that FullDelta matches up and the elements
    // are in proper order.
    પૂર્ણાંક FullDelta = 0;
    માટે (unsigned i = 0, e = N->get_અંક_કિંમતો_Used(); i != e; ++i) {
      જો (i)
        પ્રતિજ્ઞા(N->get_કિંમત(i - 1).FileLoc < N->get_કિંમત(i).FileLoc);
      FullDelta += N->get_કિંમત(i).Delta;
    }
    પ્રતિજ્ઞા(FullDelta == N->getFullDelta());
    પાછા;
  }

  // Verify interior nodes: Ensure that FullDelta matches up and the
  // elements are in proper order and the children are in proper order.
  પૂર્ણાંક FullDelta = 0;
  માટે (unsigned i = 0, e = IN->get_અંક_કિંમતો_Used(); i != e; ++i) {
    અચલ SourceDelta &IVal = N->get_કિંમત(i);
    અચલ Delta_વૃક્ષ_ગાંઠ *IChild = IN->getChild(i);
    જો (i)
      પ્રતિજ્ઞા(IN->get_કિંમત(i - 1).FileLoc < IVal.FileLoc);
    FullDelta += IVal.Delta;
    FullDelta += IChild->getFullDelta();

    // The largest value in child #i should be smaller than FileLoc.
    પ્રતિજ્ઞા(IChild->get_કિંમત(IChild->get_અંક_કિંમતો_Used() - 1).FileLoc <
           IVal.FileLoc);

    // The smallest value in child #i+1 should be larger than FileLoc.
    પ્રતિજ્ઞા(IN->getChild(i + 1)->get_કિંમત(0).FileLoc > IVal.FileLoc);
    Verify_વૃક્ષ(IChild);
  }

  FullDelta += IN->getChild(IN->get_અંક_કિંમતો_Used())->getFullDelta();

  પ્રતિજ્ઞા(FullDelta == N->getFullDelta());
}
#endif // VERIFY_TREE

સ્થિર Delta_વૃક્ષ_ગાંઠ *get_મૂળ(શૂન્ય *મૂળ_2) { પાછા (Delta_વૃક્ષ_ગાંઠ *)મૂળ_2; }

Delta_વૃક્ષ::Delta_વૃક્ષ() { મૂળ_2 = નવું Delta_વૃક્ષ_ગાંઠ(); }

Delta_વૃક્ષ::Delta_વૃક્ષ(અચલ Delta_વૃક્ષ &RHS) {
  // Currently we only support copying when the RHS is empty.
  પ્રતિજ્ઞા(get_મૂળ(RHS.મૂળ_2)->get_અંક_કિંમતો_Used() == 0 &&
         "Can only copy empty tree");
  મૂળ_2 = નવું Delta_વૃક્ષ_ગાંઠ();
}

Delta_વૃક્ષ::~Delta_વૃક્ષ() { get_મૂળ(મૂળ_2)->Destroy(); }

/// getDeltaAt - Return the accumulated delta at the specified file offset.
/// This includes all insertions or delections that occurred *before* the
/// specified file index.
પૂર્ણાંક Delta_વૃક્ષ::getDeltaAt(unsigned FileIndex) અચલ {
  અચલ Delta_વૃક્ષ_ગાંઠ *ગાંઠ = get_મૂળ(મૂળ_2);

  પૂર્ણાંક Result = 0;

  // Walk down the tree.
  જ્યારે (સાચું) {
    // For all nodes, include any local deltas before the specified file
    // index by summing them up directly.  Keep track of how many were
    // included.
    unsigned અંક_Vals_Greater = 0;
    માટે (unsigned e = ગાંઠ->get_અંક_કિંમતો_Used(); અંક_Vals_Greater != e;
         ++અંક_Vals_Greater) {
      અચલ SourceDelta &Val = ગાંઠ->get_કિંમત(અંક_Vals_Greater);

      જો (Val.FileLoc >= FileIndex)
        તોડો;
      Result += Val.Delta;
    }

    // If we have an interior node, include information about children and
    // recurse.  Otherwise, if we have a leaf, we're done.
    અચલ auto *IN = dyn_cast<Delta_વૃક્ષ_Interior_ગાંઠ>(ગાંઠ);
    જો (!IN)
      પાછા Result;

    // Include any children to the left of the values we skipped, all of
    // their deltas should be included as well.
    માટે (unsigned i = 0; i != અંક_Vals_Greater; ++i)
      Result += IN->getChild(i)->getFullDelta();

    // If we found exactly the value we were looking for, break off the
    // search early.  There is no need to search the RHS of the value for
    // partial results.
    જો (અંક_Vals_Greater != ગાંઠ->get_અંક_કિંમતો_Used() &&
        ગાંઠ->get_કિંમત(અંક_Vals_Greater).FileLoc == FileIndex)
      પાછા Result + IN->getChild(અંક_Vals_Greater)->getFullDelta();

    // Otherwise, traverse down the tree.  The selected subtree may be
    // partially included in the range.
    ગાંઠ = IN->getChild(અંક_Vals_Greater);
  }
  // NOT REACHED.
}

/// AddDelta - When a change is made that shifts around the text buffer,
/// this method is used to record that info.  It inserts a delta of 'Delta'
/// into the current DeltaTree at offset FileIndex.
શૂન્ય Delta_વૃક્ષ::AddDelta(unsigned FileIndex, પૂર્ણાંક Delta) {
  પ્રતિજ્ઞા(Delta && "Adding a noop?");
  Delta_વૃક્ષ_ગાંઠ *My_મૂળ = get_મૂળ(મૂળ_2);

  Delta_વૃક્ષ_ગાંઠ::ઉમેરો_Result ઉમેરો_Res;
  જો (My_મૂળ->DoInsertion(FileIndex, Delta, &ઉમેરો_Res)) {
    મૂળ_2 = નવું Delta_વૃક્ષ_Interior_ગાંઠ(ઉમેરો_Res);
#ifdef VERIFY_વૃક્ષ
    My_મૂળ = મૂળ_2;
#endif
  }

#ifdef VERIFY_વૃક્ષ
  Verify_વૃક્ષ(My_મૂળ);
#endif
}
