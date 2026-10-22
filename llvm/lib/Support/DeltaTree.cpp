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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/DeltaTree.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Casting.h"
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <cstring>

using ನಾಮಾಕಾಶ llvm;

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

ನಾಮಾಕಾಶ {

/// SourceDelta - As code in the original input buffer is added and deleted,
/// SourceDelta records are used to keep track of how the input SourceLocation
/// object is mapped into the output buffer.
ಸಂರಚನಾ SourceDelta {
  unsigned FileLoc;
  ಪೂರ್ಣಾಙ್ಕ Delta;

  ಸ್ಥಿರ SourceDelta get(unsigned Loc, ಪೂರ್ಣಾಙ್ಕ D) {
    SourceDelta Delta;
    Delta.FileLoc = Loc;
    Delta.Delta = D;
    ಪ್ರತ್ಯಾಗಮ Delta;
  }
};

/// DeltaTreeNode - The common part of all nodes.
///
ವರ್ಗ Delta_ವೃಕ್ಷ_ಗಂಟು {
ಸಾರ್ವಜನಿಕ:
  ಸಂರಚನಾ ಸೇರಿಸು_Result {
    Delta_ವೃಕ್ಷ_ಗಂಟು *LHS, *RHS;
    SourceDelta Split;
  };

ಗುಪ್ತ:
  friend ವರ್ಗ Delta_ವೃಕ್ಷ_Interior_ಗಂಟು;

  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// WidthFactor-1 K/V pairs (except the root) and may have at most
  /// 2*WidthFactor-1 K/V pairs.
  ಗಣನಾ { WidthFactor = 8 };

  /// Values - This tracks the SourceDelta's currently in this node.
  SourceDelta ಮೌಲ್ಯಗಳು[2 * WidthFactor - 1];

  /// NumValuesUsed - This tracks the number of values this node currently
  /// holds.
  unsigned ವರ್ಣ ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used = 0;

  /// IsLeaf - This is true if this is a leaf of the btree.  If false, this is
  /// an interior node, and is actually an instance of DeltaTreeInteriorNode.
  ತಾರ್ಕಿಕ IsLeaf;

  /// FullDelta - This is the full delta of all the values in this node and
  /// all children nodes.
  ಪೂರ್ಣಾಙ್ಕ FullDelta = 0;

ಸಾರ್ವಜನಿಕ:
  Delta_ವೃಕ್ಷ_ಗಂಟು(ತಾರ್ಕಿಕ isLeaf = ಸತ್ಯ) : IsLeaf(isLeaf) {}

  ತಾರ್ಕಿಕ isLeaf() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ IsLeaf; }
  ಪೂರ್ಣಾಙ್ಕ getFullDelta() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ FullDelta; }
  ತಾರ್ಕಿಕ isFull() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used == 2 * WidthFactor - 1; }

  unsigned get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used; }

  ಧ್ರುವ SourceDelta &get_ಮೌಲ್ಯ(unsigned i) ಧ್ರುವ {
    ಪ್ರತಿಜ್ಞಾ(i < ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used && "Invalid value #");
    ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯಗಳು[i];
  }

  SourceDelta &get_ಮೌಲ್ಯ(unsigned i) {
    ಪ್ರತಿಜ್ಞಾ(i < ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used && "Invalid value #");
    ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯಗಳು[i];
  }

  /// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
  /// this node.  If insertion is easy, do it and return false.  Otherwise,
  /// split the node, populate InsertRes with info about the split, and return
  /// true.
  ತಾರ್ಕಿಕ DoInsertion(unsigned FileIndex, ಪೂರ್ಣಾಙ್ಕ Delta, ಸೇರಿಸು_Result *ಸೇರಿಸು_Res);

  ಶೂನ್ಯ DoSplit(ಸೇರಿಸು_Result &ಸೇರಿಸು_Res);

  /// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
  /// local walk over our contained deltas.
  ಶೂನ್ಯ RecomputeFullDeltaLocally();

  ಶೂನ್ಯ Destroy();
};

/// DeltaTreeInteriorNode - When isLeaf = false, a node has child pointers.
/// This class tracks them.
ವರ್ಗ Delta_ವೃಕ್ಷ_Interior_ಗಂಟು : ಸಾರ್ವಜನಿಕ Delta_ವೃಕ್ಷ_ಗಂಟು {
  friend ವರ್ಗ Delta_ವೃಕ್ಷ_ಗಂಟು;

  Delta_ವೃಕ್ಷ_ಗಂಟು *Children[2 * WidthFactor];

  ~Delta_ವೃಕ್ಷ_Interior_ಗಂಟು() {
    ಪ್ರತಿ (unsigned i = 0, e = ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used + 1; i != e; ++i)
      Children[i]->Destroy();
  }

ಸಾರ್ವಜನಿಕ:
  Delta_ವೃಕ್ಷ_Interior_ಗಂಟು() : Delta_ವೃಕ್ಷ_ಗಂಟು(ಅಸತ್ಯ /*nonleaf*/) {}

  Delta_ವೃಕ್ಷ_Interior_ಗಂಟು(ಧ್ರುವ ಸೇರಿಸು_Result &IR)
      : Delta_ವೃಕ್ಷ_ಗಂಟು(ಅಸತ್ಯ /*nonleaf*/) {
    Children[0] = IR.LHS;
    Children[1] = IR.RHS;
    ಮೌಲ್ಯಗಳು[0] = IR.Split;
    FullDelta =
        IR.LHS->getFullDelta() + IR.RHS->getFullDelta() + IR.Split.Delta;
    ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used = 1;
  }

  ಧ್ರುವ Delta_ವೃಕ್ಷ_ಗಂಟು *getChild(unsigned i) ಧ್ರುವ {
    ಪ್ರತಿಜ್ಞಾ(i < get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used() + 1 && "Invalid child");
    ಪ್ರತ್ಯಾಗಮ Children[i];
  }

  Delta_ವೃಕ್ಷ_ಗಂಟು *getChild(unsigned i) {
    ಪ್ರತಿಜ್ಞಾ(i < get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used() + 1 && "Invalid child");
    ಪ್ರತ್ಯಾಗಮ Children[i];
  }

  ಸ್ಥಿರ ತಾರ್ಕಿಕ classof(ಧ್ರುವ Delta_ವೃಕ್ಷ_ಗಂಟು *N) { ಪ್ರತ್ಯಾಗಮ !N->isLeaf(); }
};

} // namespace

/// Destroy - A 'virtual' destructor.
ಶೂನ್ಯ Delta_ವೃಕ್ಷ_ಗಂಟು::Destroy() {
  ಯದಿ (isLeaf())
    delete ಏತತ್;
  ಅನ್ಯಥಾ
    delete cast<Delta_ವೃಕ್ಷ_Interior_ಗಂಟು>(ಏತತ್);
}

/// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
/// local walk over our contained deltas.
ಶೂನ್ಯ Delta_ವೃಕ್ಷ_ಗಂಟು::RecomputeFullDeltaLocally() {
  ಪೂರ್ಣಾಙ್ಕ NewFullDelta = 0;
  ಪ್ರತಿ (unsigned i = 0, e = get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used(); i != e; ++i)
    NewFullDelta += ಮೌಲ್ಯಗಳು[i].Delta;
  ಯದಿ (auto *IN = dyn_cast<Delta_ವೃಕ್ಷ_Interior_ಗಂಟು>(ಏತತ್))
    ಪ್ರತಿ (unsigned i = 0, e = get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used() + 1; i != e; ++i)
      NewFullDelta += IN->getChild(i)->getFullDelta();
  FullDelta = NewFullDelta;
}

/// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
/// this node.  If insertion is easy, do it and return false.  Otherwise,
/// split the node, populate InsertRes with info about the split, and return
/// true.
ತಾರ್ಕಿಕ Delta_ವೃಕ್ಷ_ಗಂಟು::DoInsertion(unsigned FileIndex, ಪೂರ್ಣಾಙ್ಕ Delta,
                                ಸೇರಿಸು_Result *ಸೇರಿಸು_Res) {
  // Maintain full delta for this node.
  FullDelta += Delta;

  // Find the insertion point, the first delta whose index is >= FileIndex.
  unsigned i = 0, e = get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used();
  ಯಾವತ್ (i != e && FileIndex > get_ಮೌಲ್ಯ(i).FileLoc)
    ++i;

  // If we found an a record for exactly this file index, just merge this
  // value into the pre-existing record and finish early.
  ಯದಿ (i != e && get_ಮೌಲ್ಯ(i).FileLoc == FileIndex) {
    // NOTE: Delta could drop to zero here.  This means that the delta entry is
    // useless and could be removed.  Supporting erases is more complex than
    // leaving an entry with Delta=0, so we just leave an entry with Delta=0 in
    // the tree.
    ಮೌಲ್ಯಗಳು[i].Delta += Delta;
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  // Otherwise, we found an insertion point, and we know that the value at the
  // specified index is > FileIndex.  Handle the leaf case first.
  ಯದಿ (isLeaf()) {
    ಯದಿ (!isFull()) {
      // For an insertion into a non-full leaf node, just insert the value in
      // its sorted position.  This requires moving later values over.
      ಯದಿ (i != e)
        memmove(&ಮೌಲ್ಯಗಳು[i + 1], &ಮೌಲ್ಯಗಳು[i], ಪರಿಮಾಣ(ಮೌಲ್ಯಗಳು[0]) * (e - i));
      ಮೌಲ್ಯಗಳು[i] = SourceDelta::get(FileIndex, Delta);
      ++ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used;
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }

    // Otherwise, if this is leaf is full, split the node at its median, insert
    // the value into one of the children, and return the result.
    ಪ್ರತಿಜ್ಞಾ(ಸೇರಿಸು_Res && "No result location specified");
    DoSplit(*ಸೇರಿಸು_Res);

    ಯದಿ (ಸೇರಿಸು_Res->Split.FileLoc > FileIndex)
      ಸೇರಿಸು_Res->LHS->DoInsertion(FileIndex, Delta, nullptr /*can't fail*/);
    ಅನ್ಯಥಾ
      ಸೇರಿಸು_Res->RHS->DoInsertion(FileIndex, Delta, nullptr /*can't fail*/);
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }

  // Otherwise, this is an interior node.  Send the request down the tree.
  auto *IN = cast<Delta_ವೃಕ್ಷ_Interior_ಗಂಟು>(ಏತತ್);
  ಯದಿ (!IN->Children[i]->DoInsertion(FileIndex, Delta, ಸೇರಿಸು_Res))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ; // If there was space in the child, just return.

  // Okay, this split the subtree, producing a new value and two children to
  // insert here.  If this node is non-full, we can just insert it directly.
  ಯದಿ (!isFull()) {
    // Now that we have two nodes and a new element, insert the perclated value
    // into ourself by moving all the later values/children down, then inserting
    // the new one.
    ಯದಿ (i != e)
      memmove(&IN->Children[i + 2], &IN->Children[i + 1],
              (e - i) * ಪರಿಮಾಣ(IN->Children[0]));
    IN->Children[i] = ಸೇರಿಸು_Res->LHS;
    IN->Children[i + 1] = ಸೇರಿಸು_Res->RHS;

    ಯದಿ (e != i)
      memmove(&ಮೌಲ್ಯಗಳು[i + 1], &ಮೌಲ್ಯಗಳು[i], (e - i) * ಪರಿಮಾಣ(ಮೌಲ್ಯಗಳು[0]));
    ಮೌಲ್ಯಗಳು[i] = ಸೇರಿಸು_Res->Split;
    ++ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used;
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  // Finally, if this interior node was full and a node is percolated up, split
  // ourself and return that up the chain.  Start by saving all our info to
  // avoid having the split clobber it.
  IN->Children[i] = ಸೇರಿಸು_Res->LHS;
  Delta_ವೃಕ್ಷ_ಗಂಟು *SubRHS = ಸೇರಿಸು_Res->RHS;
  SourceDelta SubSplit = ಸೇರಿಸು_Res->Split;

  // Do the split.
  DoSplit(*ಸೇರಿಸು_Res);

  // Figure out where to insert SubRHS/NewSplit.
  Delta_ವೃಕ್ಷ_Interior_ಗಂಟು *ಸೇರಿಸು_Side;
  ಯದಿ (SubSplit.FileLoc < ಸೇರಿಸು_Res->Split.FileLoc)
    ಸೇರಿಸು_Side = cast<Delta_ವೃಕ್ಷ_Interior_ಗಂಟು>(ಸೇರಿಸು_Res->LHS);
  ಅನ್ಯಥಾ
    ಸೇರಿಸು_Side = cast<Delta_ವೃಕ್ಷ_Interior_ಗಂಟು>(ಸೇರಿಸು_Res->RHS);

  // We now have a non-empty interior node 'InsertSide' to insert
  // SubRHS/SubSplit into.  Find out where to insert SubSplit.

  // Find the insertion point, the first delta whose index is >SubSplit.FileLoc.
  i = 0;
  e = ಸೇರಿಸು_Side->get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used();
  ಯಾವತ್ (i != e && SubSplit.FileLoc > ಸೇರಿಸು_Side->get_ಮೌಲ್ಯ(i).FileLoc)
    ++i;

  // Now we know that i is the place to insert the split value into.  Insert it
  // and the child right after it.
  ಯದಿ (i != e)
    memmove(&ಸೇರಿಸು_Side->Children[i + 2], &ಸೇರಿಸು_Side->Children[i + 1],
            (e - i) * ಪರಿಮಾಣ(IN->Children[0]));
  ಸೇರಿಸು_Side->Children[i + 1] = SubRHS;

  ಯದಿ (e != i)
    memmove(&ಸೇರಿಸು_Side->ಮೌಲ್ಯಗಳು[i + 1], &ಸೇರಿಸು_Side->ಮೌಲ್ಯಗಳು[i],
            (e - i) * ಪರಿಮಾಣ(ಮೌಲ್ಯಗಳು[0]));
  ಸೇರಿಸು_Side->ಮೌಲ್ಯಗಳು[i] = SubSplit;
  ++ಸೇರಿಸು_Side->ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used;
  ಸೇರಿಸು_Side->FullDelta += SubSplit.Delta + SubRHS->getFullDelta();
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

/// DoSplit - Split the currently full node (which has 2*WidthFactor-1 values)
/// into two subtrees each with "WidthFactor-1" values and a pivot value.
/// Return the pieces in InsertRes.
ಶೂನ್ಯ Delta_ವೃಕ್ಷ_ಗಂಟು::DoSplit(ಸೇರಿಸು_Result &ಸೇರಿಸು_Res) {
  ಪ್ರತಿಜ್ಞಾ(isFull() && "Why split a non-full node?");

  // Since this node is full, it contains 2*WidthFactor-1 values.  We move
  // the first 'WidthFactor-1' values to the LHS child (which we leave in this
  // node), propagate one value up, and move the last 'WidthFactor-1' values
  // into the RHS child.

  // Create the new child node.
  Delta_ವೃಕ್ಷ_ಗಂಟು *New_ಗಂಟು;
  ಯದಿ (auto *IN = dyn_cast<Delta_ವೃಕ್ಷ_Interior_ಗಂಟು>(ಏತತ್)) {
    // If this is an interior node, also move over 'WidthFactor' children
    // into the new node.
    Delta_ವೃಕ್ಷ_Interior_ಗಂಟು *New = ನೂತನ Delta_ವೃಕ್ಷ_Interior_ಗಂಟು();
    memcpy(&New->Children[0], &IN->Children[WidthFactor],
           WidthFactor * ಪರಿಮಾಣ(IN->Children[0]));
    New_ಗಂಟು = New;
  } ಅನ್ಯಥಾ {
    // Just create the new leaf node.
    New_ಗಂಟು = ನೂತನ Delta_ವೃಕ್ಷ_ಗಂಟು();
  }

  // Move over the last 'WidthFactor-1' values from here to NewNode.
  memcpy(&New_ಗಂಟು->ಮೌಲ್ಯಗಳು[0], &ಮೌಲ್ಯಗಳು[WidthFactor],
         (WidthFactor - 1) * ಪರಿಮಾಣ(ಮೌಲ್ಯಗಳು[0]));

  // Decrease the number of values in the two nodes.
  New_ಗಂಟು->ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used = ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used = WidthFactor - 1;

  // Recompute the two nodes' full delta.
  New_ಗಂಟು->RecomputeFullDeltaLocally();
  RecomputeFullDeltaLocally();

  ಸೇರಿಸು_Res.LHS = ಏತತ್;
  ಸೇರಿಸು_Res.RHS = New_ಗಂಟು;
  ಸೇರಿಸು_Res.Split = ಮೌಲ್ಯಗಳು[WidthFactor - 1];
}

//===----------------------------------------------------------------------===//
//                        DeltaTree Implementation
//===----------------------------------------------------------------------===//

// #define VERIFY_TREE

#ifdef VERIFY_ವೃಕ್ಷ
/// VerifyTree - Walk the btree performing assertions on various properties to
/// verify consistency.  This is useful for debugging new changes to the tree.
ಸ್ಥಿರ ಶೂನ್ಯ Verify_ವೃಕ್ಷ(ಧ್ರುವ Delta_ವೃಕ್ಷ_ಗಂಟು *N) {
  ಧ್ರುವ auto *IN = dyn_cast<Delta_ವೃಕ್ಷ_Interior_ಗಂಟು>(N);
  ಯದಿ (IN == 0) {
    // Verify leaves, just ensure that FullDelta matches up and the elements
    // are in proper order.
    ಪೂರ್ಣಾಙ್ಕ FullDelta = 0;
    ಪ್ರತಿ (unsigned i = 0, e = N->get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used(); i != e; ++i) {
      ಯದಿ (i)
        ಪ್ರತಿಜ್ಞಾ(N->get_ಮೌಲ್ಯ(i - 1).FileLoc < N->get_ಮೌಲ್ಯ(i).FileLoc);
      FullDelta += N->get_ಮೌಲ್ಯ(i).Delta;
    }
    ಪ್ರತಿಜ್ಞಾ(FullDelta == N->getFullDelta());
    ಪ್ರತ್ಯಾಗಮ;
  }

  // Verify interior nodes: Ensure that FullDelta matches up and the
  // elements are in proper order and the children are in proper order.
  ಪೂರ್ಣಾಙ್ಕ FullDelta = 0;
  ಪ್ರತಿ (unsigned i = 0, e = IN->get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used(); i != e; ++i) {
    ಧ್ರುವ SourceDelta &IVal = N->get_ಮೌಲ್ಯ(i);
    ಧ್ರುವ Delta_ವೃಕ್ಷ_ಗಂಟು *IChild = IN->getChild(i);
    ಯದಿ (i)
      ಪ್ರತಿಜ್ಞಾ(IN->get_ಮೌಲ್ಯ(i - 1).FileLoc < IVal.FileLoc);
    FullDelta += IVal.Delta;
    FullDelta += IChild->getFullDelta();

    // The largest value in child #i should be smaller than FileLoc.
    ಪ್ರತಿಜ್ಞಾ(IChild->get_ಮೌಲ್ಯ(IChild->get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used() - 1).FileLoc <
           IVal.FileLoc);

    // The smallest value in child #i+1 should be larger than FileLoc.
    ಪ್ರತಿಜ್ಞಾ(IN->getChild(i + 1)->get_ಮೌಲ್ಯ(0).FileLoc > IVal.FileLoc);
    Verify_ವೃಕ್ಷ(IChild);
  }

  FullDelta += IN->getChild(IN->get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used())->getFullDelta();

  ಪ್ರತಿಜ್ಞಾ(FullDelta == N->getFullDelta());
}
#endif // VERIFY_TREE

ಸ್ಥಿರ Delta_ವೃಕ್ಷ_ಗಂಟು *get_ಬೇರು(ಶೂನ್ಯ *ಬೇರು_2) { ಪ್ರತ್ಯಾಗಮ (Delta_ವೃಕ್ಷ_ಗಂಟು *)ಬೇರು_2; }

Delta_ವೃಕ್ಷ::Delta_ವೃಕ್ಷ() { ಬೇರು_2 = ನೂತನ Delta_ವೃಕ್ಷ_ಗಂಟು(); }

Delta_ವೃಕ್ಷ::Delta_ವೃಕ್ಷ(ಧ್ರುವ Delta_ವೃಕ್ಷ &RHS) {
  // Currently we only support copying when the RHS is empty.
  ಪ್ರತಿಜ್ಞಾ(get_ಬೇರು(RHS.ಬೇರು_2)->get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used() == 0 &&
         "Can only copy empty tree");
  ಬೇರು_2 = ನೂತನ Delta_ವೃಕ್ಷ_ಗಂಟು();
}

Delta_ವೃಕ್ಷ::~Delta_ವೃಕ್ಷ() { get_ಬೇರು(ಬೇರು_2)->Destroy(); }

/// getDeltaAt - Return the accumulated delta at the specified file offset.
/// This includes all insertions or delections that occurred *before* the
/// specified file index.
ಪೂರ್ಣಾಙ್ಕ Delta_ವೃಕ್ಷ::getDeltaAt(unsigned FileIndex) ಧ್ರುವ {
  ಧ್ರುವ Delta_ವೃಕ್ಷ_ಗಂಟು *ಗಂಟು = get_ಬೇರು(ಬೇರು_2);

  ಪೂರ್ಣಾಙ್ಕ Result = 0;

  // Walk down the tree.
  ಯಾವತ್ (ಸತ್ಯ) {
    // For all nodes, include any local deltas before the specified file
    // index by summing them up directly.  Keep track of how many were
    // included.
    unsigned ಅಂಕೆ_Vals_Greater = 0;
    ಪ್ರತಿ (unsigned e = ಗಂಟು->get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used(); ಅಂಕೆ_Vals_Greater != e;
         ++ಅಂಕೆ_Vals_Greater) {
      ಧ್ರುವ SourceDelta &Val = ಗಂಟು->get_ಮೌಲ್ಯ(ಅಂಕೆ_Vals_Greater);

      ಯದಿ (Val.FileLoc >= FileIndex)
        ಭಙ್ಗ;
      Result += Val.Delta;
    }

    // If we have an interior node, include information about children and
    // recurse.  Otherwise, if we have a leaf, we're done.
    ಧ್ರುವ auto *IN = dyn_cast<Delta_ವೃಕ್ಷ_Interior_ಗಂಟು>(ಗಂಟು);
    ಯದಿ (!IN)
      ಪ್ರತ್ಯಾಗಮ Result;

    // Include any children to the left of the values we skipped, all of
    // their deltas should be included as well.
    ಪ್ರತಿ (unsigned i = 0; i != ಅಂಕೆ_Vals_Greater; ++i)
      Result += IN->getChild(i)->getFullDelta();

    // If we found exactly the value we were looking for, break off the
    // search early.  There is no need to search the RHS of the value for
    // partial results.
    ಯದಿ (ಅಂಕೆ_Vals_Greater != ಗಂಟು->get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_Used() &&
        ಗಂಟು->get_ಮೌಲ್ಯ(ಅಂಕೆ_Vals_Greater).FileLoc == FileIndex)
      ಪ್ರತ್ಯಾಗಮ Result + IN->getChild(ಅಂಕೆ_Vals_Greater)->getFullDelta();

    // Otherwise, traverse down the tree.  The selected subtree may be
    // partially included in the range.
    ಗಂಟು = IN->getChild(ಅಂಕೆ_Vals_Greater);
  }
  // NOT REACHED.
}

/// AddDelta - When a change is made that shifts around the text buffer,
/// this method is used to record that info.  It inserts a delta of 'Delta'
/// into the current DeltaTree at offset FileIndex.
ಶೂನ್ಯ Delta_ವೃಕ್ಷ::AddDelta(unsigned FileIndex, ಪೂರ್ಣಾಙ್ಕ Delta) {
  ಪ್ರತಿಜ್ಞಾ(Delta && "Adding a noop?");
  Delta_ವೃಕ್ಷ_ಗಂಟು *My_ಬೇರು = get_ಬೇರು(ಬೇರು_2);

  Delta_ವೃಕ್ಷ_ಗಂಟು::ಸೇರಿಸು_Result ಸೇರಿಸು_Res;
  ಯದಿ (My_ಬೇರು->DoInsertion(FileIndex, Delta, &ಸೇರಿಸು_Res)) {
    ಬೇರು_2 = ನೂತನ Delta_ವೃಕ್ಷ_Interior_ಗಂಟು(ಸೇರಿಸು_Res);
#ifdef VERIFY_ವೃಕ್ಷ
    My_ಬೇರು = ಬೇರು_2;
#endif
  }

#ifdef VERIFY_ವೃಕ್ಷ
  Verify_ವೃಕ್ಷ(My_ಬೇರು);
#endif
}
