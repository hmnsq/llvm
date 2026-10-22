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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

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
ರಚನೆ ಮೂಲ_Delta {
  ಚಿಹ್ನೆರಹಿತ FileLoc;
  ಪೂರ್ಣಾಂಕ Delta;

  ಸ್ಥಿರ ಮೂಲ_Delta get(ಚಿಹ್ನೆರಹಿತ Loc, ಪೂರ್ಣಾಂಕ D) {
    ಮೂಲ_Delta Delta;
    Delta.FileLoc = Loc;
    Delta.Delta = D;
    ಹಿಂತಿರುಗಿ Delta;
  }
};

/// DeltaTreeNode - The common part of all nodes.
///
ವರ್ಗ Delta_ವೃಕ್ಷ_ಗಂಟು {
ಸಾರ್ವಜನಿಕ:
  ರಚನೆ ಸೇರಿಸು_ಫಲಿತಾಂಶ {
    Delta_ವೃಕ್ಷ_ಗಂಟು *LHS, *RHS;
    ಮೂಲ_Delta ವಿಭಜಿಸು_2;
  };

ಖಾಸಗಿ:
  ಮಿತ್ರ ವರ್ಗ Delta_ವೃಕ್ಷ_Interior_ಗಂಟು;

  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// WidthFactor-1 K/V pairs (except the root) and may have at most
  /// 2*WidthFactor-1 K/V pairs.
  ಎಣಿಕೆ { ಅಗಲ_ಗುಣಕ = 8 };

  /// Values - This tracks the SourceDelta's currently in this node.
  ಮೂಲ_Delta ಮೌಲ್ಯಗಳು[2 * ಅಗಲ_ಗುಣಕ - 1];

  /// NumValuesUsed - This tracks the number of values this node currently
  /// holds.
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ = 0;

  /// IsLeaf - This is true if this is a leaf of the btree.  If false, this is
  /// an interior node, and is actually an instance of DeltaTreeInteriorNode.
  ತಾರ್ಕಿಕ Is_ಎಲೆ;

  /// FullDelta - This is the full delta of all the values in this node and
  /// all children nodes.
  ಪೂರ್ಣಾಂಕ ತುಂಬಿದ_Delta = 0;

ಸಾರ್ವಜನಿಕ:
  Delta_ವೃಕ್ಷ_ಗಂಟು(ತಾರ್ಕಿಕ is_ಎಲೆ = ನಿಜ) : Is_ಎಲೆ(is_ಎಲೆ) {}

  ತಾರ್ಕಿಕ is_ಎಲೆ() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Is_ಎಲೆ; }
  ಪೂರ್ಣಾಂಕ get_ತುಂಬಿದ_Delta() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ತುಂಬಿದ_Delta; }
  ತಾರ್ಕಿಕ is_ತುಂಬಿದ() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ == 2 * ಅಗಲ_ಗುಣಕ - 1; }

  ಚಿಹ್ನೆರಹಿತ get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ; }

  ಸ್ಥಿರಾಂಕ ಮೂಲ_Delta &get_ಮೌಲ್ಯ(ಚಿಹ್ನೆರಹಿತ i) ಸ್ಥಿರಾಂಕ {
    ಪ್ರತಿಜ್ಞಾ(i < ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ && "Invalid value #");
    ಹಿಂತಿರುಗಿ ಮೌಲ್ಯಗಳು[i];
  }

  ಮೂಲ_Delta &get_ಮೌಲ್ಯ(ಚಿಹ್ನೆರಹಿತ i) {
    ಪ್ರತಿಜ್ಞಾ(i < ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ && "Invalid value #");
    ಹಿಂತಿರುಗಿ ಮೌಲ್ಯಗಳು[i];
  }

  /// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
  /// this node.  If insertion is easy, do it and return false.  Otherwise,
  /// split the node, populate InsertRes with info about the split, and return
  /// true.
  ತಾರ್ಕಿಕ DoInsertion(ಚಿಹ್ನೆರಹಿತ File_ಸೂಚ್ಯಂಕ, ಪೂರ್ಣಾಂಕ Delta, ಸೇರಿಸು_ಫಲಿತಾಂಶ *ಸೇರಿಸು_Res);

  ಶೂನ್ಯ Do_ವಿಭಜಿಸು(ಸೇರಿಸು_ಫಲಿತಾಂಶ &ಸೇರಿಸು_Res);

  /// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
  /// local walk over our contained deltas.
  ಶೂನ್ಯ Recompute_ತುಂಬಿದ_Delta_Locally();

  ಶೂನ್ಯ Destroy();
};

/// DeltaTreeInteriorNode - When isLeaf = false, a node has child pointers.
/// This class tracks them.
ವರ್ಗ Delta_ವೃಕ್ಷ_Interior_ಗಂಟು : ಸಾರ್ವಜನಿಕ Delta_ವೃಕ್ಷ_ಗಂಟು {
  ಮಿತ್ರ ವರ್ಗ Delta_ವೃಕ್ಷ_ಗಂಟು;

  Delta_ವೃಕ್ಷ_ಗಂಟು *Children[2 * ಅಗಲ_ಗುಣಕ];

  ~Delta_ವೃಕ್ಷ_Interior_ಗಂಟು() {
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ + 1; i != e; ++i)
      Children[i]->Destroy();
  }

ಸಾರ್ವಜನಿಕ:
  Delta_ವೃಕ್ಷ_Interior_ಗಂಟು() : Delta_ವೃಕ್ಷ_ಗಂಟು(ಸುಳ್ಳು /*nonleaf*/) {}

  Delta_ವೃಕ್ಷ_Interior_ಗಂಟು(ಸ್ಥಿರಾಂಕ ಸೇರಿಸು_ಫಲಿತಾಂಶ &IR)
      : Delta_ವೃಕ್ಷ_ಗಂಟು(ಸುಳ್ಳು /*nonleaf*/) {
    Children[0] = IR.LHS;
    Children[1] = IR.RHS;
    ಮೌಲ್ಯಗಳು[0] = IR.ವಿಭಜಿಸು_2;
    ತುಂಬಿದ_Delta =
        IR.LHS->get_ತುಂಬಿದ_Delta() + IR.RHS->get_ತುಂಬಿದ_Delta() + IR.ವಿಭಜಿಸು_2.Delta;
    ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ = 1;
  }

  ಸ್ಥಿರಾಂಕ Delta_ವೃಕ್ಷ_ಗಂಟು *get_ಮಗು(ಚಿಹ್ನೆರಹಿತ i) ಸ್ಥಿರಾಂಕ {
    ಪ್ರತಿಜ್ಞಾ(i < get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ() + 1 && "Invalid child");
    ಹಿಂತಿರುಗಿ Children[i];
  }

  Delta_ವೃಕ್ಷ_ಗಂಟು *get_ಮಗು(ಚಿಹ್ನೆರಹಿತ i) {
    ಪ್ರತಿಜ್ಞಾ(i < get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ() + 1 && "Invalid child");
    ಹಿಂತಿರುಗಿ Children[i];
  }

  ಸ್ಥಿರ ತಾರ್ಕಿಕ classof(ಸ್ಥಿರಾಂಕ Delta_ವೃಕ್ಷ_ಗಂಟು *N) { ಹಿಂತಿರುಗಿ !N->is_ಎಲೆ(); }
};

} // namespace

/// Destroy - A 'virtual' destructor.
ಶೂನ್ಯ Delta_ವೃಕ್ಷ_ಗಂಟು::Destroy() {
  ಆದರೆ (is_ಎಲೆ())
    ಅಳಿಸು ಇದು;
  ಇಲ್ಲದಿದ್ದರೆ
    ಅಳಿಸು cast<Delta_ವೃಕ್ಷ_Interior_ಗಂಟು>(ಇದು);
}

/// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
/// local walk over our contained deltas.
ಶೂನ್ಯ Delta_ವೃಕ್ಷ_ಗಂಟು::Recompute_ತುಂಬಿದ_Delta_Locally() {
  ಪೂರ್ಣಾಂಕ New_ತುಂಬಿದ_Delta = 0;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ(); i != e; ++i)
    New_ತುಂಬಿದ_Delta += ಮೌಲ್ಯಗಳು[i].Delta;
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *ಒಳ = dyn_cast<Delta_ವೃಕ್ಷ_Interior_ಗಂಟು>(ಇದು))
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ() + 1; i != e; ++i)
      New_ತುಂಬಿದ_Delta += ಒಳ->get_ಮಗು(i)->get_ತುಂಬಿದ_Delta();
  ತುಂಬಿದ_Delta = New_ತುಂಬಿದ_Delta;
}

/// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
/// this node.  If insertion is easy, do it and return false.  Otherwise,
/// split the node, populate InsertRes with info about the split, and return
/// true.
ತಾರ್ಕಿಕ Delta_ವೃಕ್ಷ_ಗಂಟು::DoInsertion(ಚಿಹ್ನೆರಹಿತ File_ಸೂಚ್ಯಂಕ, ಪೂರ್ಣಾಂಕ Delta,
                                ಸೇರಿಸು_ಫಲಿತಾಂಶ *ಸೇರಿಸು_Res) {
  // Maintain full delta for this node.
  ತುಂಬಿದ_Delta += Delta;

  // Find the insertion point, the first delta whose index is >= FileIndex.
  ಚಿಹ್ನೆರಹಿತ i = 0, e = get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ();
  ಆಗ (i != e && File_ಸೂಚ್ಯಂಕ > get_ಮೌಲ್ಯ(i).FileLoc)
    ++i;

  // If we found an a record for exactly this file index, just merge this
  // value into the pre-existing record and finish early.
  ಆದರೆ (i != e && get_ಮೌಲ್ಯ(i).FileLoc == File_ಸೂಚ್ಯಂಕ) {
    // NOTE: Delta could drop to zero here.  This means that the delta entry is
    // useless and could be removed.  Supporting erases is more complex than
    // leaving an entry with Delta=0, so we just leave an entry with Delta=0 in
    // the tree.
    ಮೌಲ್ಯಗಳು[i].Delta += Delta;
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  // Otherwise, we found an insertion point, and we know that the value at the
  // specified index is > FileIndex.  Handle the leaf case first.
  ಆದರೆ (is_ಎಲೆ()) {
    ಆದರೆ (!is_ತುಂಬಿದ()) {
      // For an insertion into a non-full leaf node, just insert the value in
      // its sorted position.  This requires moving later values over.
      ಆದರೆ (i != e)
        memmove(&ಮೌಲ್ಯಗಳು[i + 1], &ಮೌಲ್ಯಗಳು[i], ಗಾತ್ರ(ಮೌಲ್ಯಗಳು[0]) * (e - i));
      ಮೌಲ್ಯಗಳು[i] = ಮೂಲ_Delta::get(File_ಸೂಚ್ಯಂಕ, Delta);
      ++ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ;
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }

    // Otherwise, if this is leaf is full, split the node at its median, insert
    // the value into one of the children, and return the result.
    ಪ್ರತಿಜ್ಞಾ(ಸೇರಿಸು_Res && "No result location specified");
    Do_ವಿಭಜಿಸು(*ಸೇರಿಸು_Res);

    ಆದರೆ (ಸೇರಿಸು_Res->ವಿಭಜಿಸು_2.FileLoc > File_ಸೂಚ್ಯಂಕ)
      ಸೇರಿಸು_Res->LHS->DoInsertion(File_ಸೂಚ್ಯಂಕ, Delta, ಶೂನ್ಯಸೂಚಕ /*can't fail*/);
    ಇಲ್ಲದಿದ್ದರೆ
      ಸೇರಿಸು_Res->RHS->DoInsertion(File_ಸೂಚ್ಯಂಕ, Delta, ಶೂನ್ಯಸೂಚಕ /*can't fail*/);
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }

  // Otherwise, this is an interior node.  Send the request down the tree.
  ಸ್ವಯಂಚಾಲಿತ *ಒಳ = cast<Delta_ವೃಕ್ಷ_Interior_ಗಂಟು>(ಇದು);
  ಆದರೆ (!ಒಳ->Children[i]->DoInsertion(File_ಸೂಚ್ಯಂಕ, Delta, ಸೇರಿಸು_Res))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು; // If there was space in the child, just return.

  // Okay, this split the subtree, producing a new value and two children to
  // insert here.  If this node is non-full, we can just insert it directly.
  ಆದರೆ (!is_ತುಂಬಿದ()) {
    // Now that we have two nodes and a new element, insert the perclated value
    // into ourself by moving all the later values/children down, then inserting
    // the new one.
    ಆದರೆ (i != e)
      memmove(&ಒಳ->Children[i + 2], &ಒಳ->Children[i + 1],
              (e - i) * ಗಾತ್ರ(ಒಳ->Children[0]));
    ಒಳ->Children[i] = ಸೇರಿಸು_Res->LHS;
    ಒಳ->Children[i + 1] = ಸೇರಿಸು_Res->RHS;

    ಆದರೆ (e != i)
      memmove(&ಮೌಲ್ಯಗಳು[i + 1], &ಮೌಲ್ಯಗಳು[i], (e - i) * ಗಾತ್ರ(ಮೌಲ್ಯಗಳು[0]));
    ಮೌಲ್ಯಗಳು[i] = ಸೇರಿಸು_Res->ವಿಭಜಿಸು_2;
    ++ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ;
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  // Finally, if this interior node was full and a node is percolated up, split
  // ourself and return that up the chain.  Start by saving all our info to
  // avoid having the split clobber it.
  ಒಳ->Children[i] = ಸೇರಿಸು_Res->LHS;
  Delta_ವೃಕ್ಷ_ಗಂಟು *SubRHS = ಸೇರಿಸು_Res->RHS;
  ಮೂಲ_Delta Sub_ವಿಭಜಿಸು = ಸೇರಿಸು_Res->ವಿಭಜಿಸು_2;

  // Do the split.
  Do_ವಿಭಜಿಸು(*ಸೇರಿಸು_Res);

  // Figure out where to insert SubRHS/NewSplit.
  Delta_ವೃಕ್ಷ_Interior_ಗಂಟು *ಸೇರಿಸು_Side;
  ಆದರೆ (Sub_ವಿಭಜಿಸು.FileLoc < ಸೇರಿಸು_Res->ವಿಭಜಿಸು_2.FileLoc)
    ಸೇರಿಸು_Side = cast<Delta_ವೃಕ್ಷ_Interior_ಗಂಟು>(ಸೇರಿಸು_Res->LHS);
  ಇಲ್ಲದಿದ್ದರೆ
    ಸೇರಿಸು_Side = cast<Delta_ವೃಕ್ಷ_Interior_ಗಂಟು>(ಸೇರಿಸು_Res->RHS);

  // We now have a non-empty interior node 'InsertSide' to insert
  // SubRHS/SubSplit into.  Find out where to insert SubSplit.

  // Find the insertion point, the first delta whose index is >SubSplit.FileLoc.
  i = 0;
  e = ಸೇರಿಸು_Side->get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ();
  ಆಗ (i != e && Sub_ವಿಭಜಿಸು.FileLoc > ಸೇರಿಸು_Side->get_ಮೌಲ್ಯ(i).FileLoc)
    ++i;

  // Now we know that i is the place to insert the split value into.  Insert it
  // and the child right after it.
  ಆದರೆ (i != e)
    memmove(&ಸೇರಿಸು_Side->Children[i + 2], &ಸೇರಿಸು_Side->Children[i + 1],
            (e - i) * ಗಾತ್ರ(ಒಳ->Children[0]));
  ಸೇರಿಸು_Side->Children[i + 1] = SubRHS;

  ಆದರೆ (e != i)
    memmove(&ಸೇರಿಸು_Side->ಮೌಲ್ಯಗಳು[i + 1], &ಸೇರಿಸು_Side->ಮೌಲ್ಯಗಳು[i],
            (e - i) * ಗಾತ್ರ(ಮೌಲ್ಯಗಳು[0]));
  ಸೇರಿಸು_Side->ಮೌಲ್ಯಗಳು[i] = Sub_ವಿಭಜಿಸು;
  ++ಸೇರಿಸು_Side->ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ;
  ಸೇರಿಸು_Side->ತುಂಬಿದ_Delta += Sub_ವಿಭಜಿಸು.Delta + SubRHS->get_ತುಂಬಿದ_Delta();
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

/// DoSplit - Split the currently full node (which has 2*WidthFactor-1 values)
/// into two subtrees each with "WidthFactor-1" values and a pivot value.
/// Return the pieces in InsertRes.
ಶೂನ್ಯ Delta_ವೃಕ್ಷ_ಗಂಟು::Do_ವಿಭಜಿಸು(ಸೇರಿಸು_ಫಲಿತಾಂಶ &ಸೇರಿಸು_Res) {
  ಪ್ರತಿಜ್ಞಾ(is_ತುಂಬಿದ() && "Why split a non-full node?");

  // Since this node is full, it contains 2*WidthFactor-1 values.  We move
  // the first 'WidthFactor-1' values to the LHS child (which we leave in this
  // node), propagate one value up, and move the last 'WidthFactor-1' values
  // into the RHS child.

  // Create the new child node.
  Delta_ವೃಕ್ಷ_ಗಂಟು *New_ಗಂಟು;
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *ಒಳ = dyn_cast<Delta_ವೃಕ್ಷ_Interior_ಗಂಟು>(ಇದು)) {
    // If this is an interior node, also move over 'WidthFactor' children
    // into the new node.
    Delta_ವೃಕ್ಷ_Interior_ಗಂಟು *New = ಹೊಸ Delta_ವೃಕ್ಷ_Interior_ಗಂಟು();
    ಸ್ಮರಣೆ_ನಕಲು(&New->Children[0], &ಒಳ->Children[ಅಗಲ_ಗುಣಕ],
           ಅಗಲ_ಗುಣಕ * ಗಾತ್ರ(ಒಳ->Children[0]));
    New_ಗಂಟು = New;
  } ಇಲ್ಲದಿದ್ದರೆ {
    // Just create the new leaf node.
    New_ಗಂಟು = ಹೊಸ Delta_ವೃಕ್ಷ_ಗಂಟು();
  }

  // Move over the last 'WidthFactor-1' values from here to NewNode.
  ಸ್ಮರಣೆ_ನಕಲು(&New_ಗಂಟು->ಮೌಲ್ಯಗಳು[0], &ಮೌಲ್ಯಗಳು[ಅಗಲ_ಗುಣಕ],
         (ಅಗಲ_ಗುಣಕ - 1) * ಗಾತ್ರ(ಮೌಲ್ಯಗಳು[0]));

  // Decrease the number of values in the two nodes.
  New_ಗಂಟು->ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ = ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ = ಅಗಲ_ಗುಣಕ - 1;

  // Recompute the two nodes' full delta.
  New_ಗಂಟು->Recompute_ತುಂಬಿದ_Delta_Locally();
  Recompute_ತುಂಬಿದ_Delta_Locally();

  ಸೇರಿಸು_Res.LHS = ಇದು;
  ಸೇರಿಸು_Res.RHS = New_ಗಂಟು;
  ಸೇರಿಸು_Res.ವಿಭಜಿಸು_2 = ಮೌಲ್ಯಗಳು[ಅಗಲ_ಗುಣಕ - 1];
}

//===----------------------------------------------------------------------===//
//                        DeltaTree Implementation
//===----------------------------------------------------------------------===//

// #define VERIFY_TREE

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ VERIFY_ವೃಕ್ಷ
/// VerifyTree - Walk the btree performing assertions on various properties to
/// verify consistency.  This is useful for debugging new changes to the tree.
ಸ್ಥಿರ ಶೂನ್ಯ Verify_ವೃಕ್ಷ(ಸ್ಥಿರಾಂಕ Delta_ವೃಕ್ಷ_ಗಂಟು *N) {
  ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ *ಒಳ = dyn_cast<Delta_ವೃಕ್ಷ_Interior_ಗಂಟು>(N);
  ಆದರೆ (ಒಳ == 0) {
    // Verify leaves, just ensure that FullDelta matches up and the elements
    // are in proper order.
    ಪೂರ್ಣಾಂಕ ತುಂಬಿದ_Delta = 0;
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = N->get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ(); i != e; ++i) {
      ಆದರೆ (i)
        ಪ್ರತಿಜ್ಞಾ(N->get_ಮೌಲ್ಯ(i - 1).FileLoc < N->get_ಮೌಲ್ಯ(i).FileLoc);
      ತುಂಬಿದ_Delta += N->get_ಮೌಲ್ಯ(i).Delta;
    }
    ಪ್ರತಿಜ್ಞಾ(ತುಂಬಿದ_Delta == N->get_ತುಂಬಿದ_Delta());
    ಹಿಂತಿರುಗಿ;
  }

  // Verify interior nodes: Ensure that FullDelta matches up and the
  // elements are in proper order and the children are in proper order.
  ಪೂರ್ಣಾಂಕ ತುಂಬಿದ_Delta = 0;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = ಒಳ->get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ(); i != e; ++i) {
    ಸ್ಥಿರಾಂಕ ಮೂಲ_Delta &IVal = N->get_ಮೌಲ್ಯ(i);
    ಸ್ಥಿರಾಂಕ Delta_ವೃಕ್ಷ_ಗಂಟು *IChild = ಒಳ->get_ಮಗು(i);
    ಆದರೆ (i)
      ಪ್ರತಿಜ್ಞಾ(ಒಳ->get_ಮೌಲ್ಯ(i - 1).FileLoc < IVal.FileLoc);
    ತುಂಬಿದ_Delta += IVal.Delta;
    ತುಂಬಿದ_Delta += IChild->get_ತುಂಬಿದ_Delta();

    // The largest value in child #i should be smaller than FileLoc.
    ಪ್ರತಿಜ್ಞಾ(IChild->get_ಮೌಲ್ಯ(IChild->get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ() - 1).FileLoc <
           IVal.FileLoc);

    // The smallest value in child #i+1 should be larger than FileLoc.
    ಪ್ರತಿಜ್ಞಾ(ಒಳ->get_ಮಗು(i + 1)->get_ಮೌಲ್ಯ(0).FileLoc > IVal.FileLoc);
    Verify_ವೃಕ್ಷ(IChild);
  }

  ತುಂಬಿದ_Delta += ಒಳ->get_ಮಗು(ಒಳ->get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ())->get_ತುಂಬಿದ_Delta();

  ಪ್ರತಿಜ್ಞಾ(ತುಂಬಿದ_Delta == N->get_ತುಂಬಿದ_Delta());
}
#ಆದರೆ_ಅಂತ್ಯ // VERIFY_TREE

ಸ್ಥಿರ Delta_ವೃಕ್ಷ_ಗಂಟು *get_ಬೇರು(ಶೂನ್ಯ *ಬೇರು_2) { ಹಿಂತಿರುಗಿ (Delta_ವೃಕ್ಷ_ಗಂಟು *)ಬೇರು_2; }

Delta_ವೃಕ್ಷ::Delta_ವೃಕ್ಷ() { ಬೇರು_2 = ಹೊಸ Delta_ವೃಕ್ಷ_ಗಂಟು(); }

Delta_ವೃಕ್ಷ::Delta_ವೃಕ್ಷ(ಸ್ಥಿರಾಂಕ Delta_ವೃಕ್ಷ &RHS) {
  // Currently we only support copying when the RHS is empty.
  ಪ್ರತಿಜ್ಞಾ(get_ಬೇರು(RHS.ಬೇರು_2)->get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ() == 0 &&
         "Can only copy empty tree");
  ಬೇರು_2 = ಹೊಸ Delta_ವೃಕ್ಷ_ಗಂಟು();
}

Delta_ವೃಕ್ಷ::~Delta_ವೃಕ್ಷ() { get_ಬೇರು(ಬೇರು_2)->Destroy(); }

/// getDeltaAt - Return the accumulated delta at the specified file offset.
/// This includes all insertions or delections that occurred *before* the
/// specified file index.
ಪೂರ್ಣಾಂಕ Delta_ವೃಕ್ಷ::getDeltaAt(ಚಿಹ್ನೆರಹಿತ File_ಸೂಚ್ಯಂಕ) ಸ್ಥಿರಾಂಕ {
  ಸ್ಥಿರಾಂಕ Delta_ವೃಕ್ಷ_ಗಂಟು *ಗಂಟು = get_ಬೇರು(ಬೇರು_2);

  ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ = 0;

  // Walk down the tree.
  ಆಗ (ನಿಜ) {
    // For all nodes, include any local deltas before the specified file
    // index by summing them up directly.  Keep track of how many were
    // included.
    ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Vals_Greater = 0;
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ e = ಗಂಟು->get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ(); ಅಂಕೆ_Vals_Greater != e;
         ++ಅಂಕೆ_Vals_Greater) {
      ಸ್ಥಿರಾಂಕ ಮೂಲ_Delta &Val = ಗಂಟು->get_ಮೌಲ್ಯ(ಅಂಕೆ_Vals_Greater);

      ಆದರೆ (Val.FileLoc >= File_ಸೂಚ್ಯಂಕ)
        ನಿಲ್ಲಿಸು;
      ಫಲಿತಾಂಶ += Val.Delta;
    }

    // If we have an interior node, include information about children and
    // recurse.  Otherwise, if we have a leaf, we're done.
    ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ *ಒಳ = dyn_cast<Delta_ವೃಕ್ಷ_Interior_ಗಂಟು>(ಗಂಟು);
    ಆದರೆ (!ಒಳ)
      ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;

    // Include any children to the left of the values we skipped, all of
    // their deltas should be included as well.
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i != ಅಂಕೆ_Vals_Greater; ++i)
      ಫಲಿತಾಂಶ += ಒಳ->get_ಮಗು(i)->get_ತುಂಬಿದ_Delta();

    // If we found exactly the value we were looking for, break off the
    // search early.  There is no need to search the RHS of the value for
    // partial results.
    ಆದರೆ (ಅಂಕೆ_Vals_Greater != ಗಂಟು->get_ಅಂಕೆ_ಮೌಲ್ಯಗಳು_ಬಳಸಿದ() &&
        ಗಂಟು->get_ಮೌಲ್ಯ(ಅಂಕೆ_Vals_Greater).FileLoc == File_ಸೂಚ್ಯಂಕ)
      ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ + ಒಳ->get_ಮಗು(ಅಂಕೆ_Vals_Greater)->get_ತುಂಬಿದ_Delta();

    // Otherwise, traverse down the tree.  The selected subtree may be
    // partially included in the range.
    ಗಂಟು = ಒಳ->get_ಮಗು(ಅಂಕೆ_Vals_Greater);
  }
  // NOT REACHED.
}

/// AddDelta - When a change is made that shifts around the text buffer,
/// this method is used to record that info.  It inserts a delta of 'Delta'
/// into the current DeltaTree at offset FileIndex.
ಶೂನ್ಯ Delta_ವೃಕ್ಷ::ಕೂಡಿಸು_Delta(ಚಿಹ್ನೆರಹಿತ File_ಸೂಚ್ಯಂಕ, ಪೂರ್ಣಾಂಕ Delta) {
  ಪ್ರತಿಜ್ಞಾ(Delta && "Adding a noop?");
  Delta_ವೃಕ್ಷ_ಗಂಟು *My_ಬೇರು = get_ಬೇರು(ಬೇರು_2);

  Delta_ವೃಕ್ಷ_ಗಂಟು::ಸೇರಿಸು_ಫಲಿತಾಂಶ ಸೇರಿಸು_Res;
  ಆದರೆ (My_ಬೇರು->DoInsertion(File_ಸೂಚ್ಯಂಕ, Delta, &ಸೇರಿಸು_Res)) {
    ಬೇರು_2 = ಹೊಸ Delta_ವೃಕ್ಷ_Interior_ಗಂಟು(ಸೇರಿಸು_Res);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ VERIFY_ವೃಕ್ಷ
    My_ಬೇರು = ಬೇರು_2;
#ಆದರೆ_ಅಂತ್ಯ
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ VERIFY_ವೃಕ್ಷ
  Verify_ವೃಕ್ಷ(My_ಬೇರು);
#ಆದರೆ_ಅಂತ್ಯ
}
