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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/DeltaTree.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Casting.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstring>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

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

ਨਾਮਾਕਾਸ਼ {

/// SourceDelta - As code in the original input buffer is added and deleted,
/// SourceDelta records are used to keep track of how the input SourceLocation
/// object is mapped into the output buffer.
ਬਣਤਰ ਸਰੋਤ_Delta {
  ਬਿਨਾਨਿਸ਼ਾਨੀ FileLoc;
  ਪੂਰਨ Delta;

  ਸਥਿਰ ਸਰੋਤ_Delta get(ਬਿਨਾਨਿਸ਼ਾਨੀ Loc, ਪੂਰਨ D) {
    ਸਰੋਤ_Delta Delta;
    Delta.FileLoc = Loc;
    Delta.Delta = D;
    ਵਾਪਸ Delta;
  }
};

/// DeltaTreeNode - The common part of all nodes.
///
ਵਰਗ Delta_ਰੁੱਖ_ਗੰਢ {
ਜਨਤਕ:
  ਬਣਤਰ ਪਾਓ_ਨਤੀਜਾ {
    Delta_ਰੁੱਖ_ਗੰਢ *LHS, *RHS;
    ਸਰੋਤ_Delta ਵੰਡੋ_2;
  };

ਨਿਜੀ:
  ਮਿੱਤਰ ਵਰਗ Delta_ਰੁੱਖ_Interior_ਗੰਢ;

  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// WidthFactor-1 K/V pairs (except the root) and may have at most
  /// 2*WidthFactor-1 K/V pairs.
  ਗਿਣਤੀ { ਚੌੜਾਈ_ਗੁਣਕ = 8 };

  /// Values - This tracks the SourceDelta's currently in this node.
  ਸਰੋਤ_Delta ਮੁੱਲਾਂ[2 * ਚੌੜਾਈ_ਗੁਣਕ - 1];

  /// NumValuesUsed - This tracks the number of values this node currently
  /// holds.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ = 0;

  /// IsLeaf - This is true if this is a leaf of the btree.  If false, this is
  /// an interior node, and is actually an instance of DeltaTreeInteriorNode.
  ਤਾਰਕਿਕ Is_ਪੱਤੀ;

  /// FullDelta - This is the full delta of all the values in this node and
  /// all children nodes.
  ਪੂਰਨ ਭਰਿਆ_Delta = 0;

ਜਨਤਕ:
  Delta_ਰੁੱਖ_ਗੰਢ(ਤਾਰਕਿਕ is_ਪੱਤੀ = ਸੱਚ) : Is_ਪੱਤੀ(is_ਪੱਤੀ) {}

  ਤਾਰਕਿਕ is_ਪੱਤੀ() ਅਚਲ { ਵਾਪਸ Is_ਪੱਤੀ; }
  ਪੂਰਨ get_ਭਰਿਆ_Delta() ਅਚਲ { ਵਾਪਸ ਭਰਿਆ_Delta; }
  ਤਾਰਕਿਕ is_ਭਰਿਆ() ਅਚਲ { ਵਾਪਸ ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ == 2 * ਚੌੜਾਈ_ਗੁਣਕ - 1; }

  ਬਿਨਾਨਿਸ਼ਾਨੀ get_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ() ਅਚਲ { ਵਾਪਸ ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ; }

  ਅਚਲ ਸਰੋਤ_Delta &get_ਮੁੱਲ(ਬਿਨਾਨਿਸ਼ਾਨੀ i) ਅਚਲ {
    ਪ੍ਰਤਿਜ੍ਞਾ(i < ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ && "Invalid value #");
    ਵਾਪਸ ਮੁੱਲਾਂ[i];
  }

  ਸਰੋਤ_Delta &get_ਮੁੱਲ(ਬਿਨਾਨਿਸ਼ਾਨੀ i) {
    ਪ੍ਰਤਿਜ੍ਞਾ(i < ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ && "Invalid value #");
    ਵਾਪਸ ਮੁੱਲਾਂ[i];
  }

  /// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
  /// this node.  If insertion is easy, do it and return false.  Otherwise,
  /// split the node, populate InsertRes with info about the split, and return
  /// true.
  ਤਾਰਕਿਕ DoInsertion(ਬਿਨਾਨਿਸ਼ਾਨੀ File_ਸੂਚਕਾਂਕ, ਪੂਰਨ Delta, ਪਾਓ_ਨਤੀਜਾ *ਪਾਓ_Res);

  ਖਾਲੀ Do_ਵੰਡੋ(ਪਾਓ_ਨਤੀਜਾ &ਪਾਓ_Res);

  /// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
  /// local walk over our contained deltas.
  ਖਾਲੀ Recompute_ਭਰਿਆ_Delta_Locally();

  ਖਾਲੀ Destroy();
};

/// DeltaTreeInteriorNode - When isLeaf = false, a node has child pointers.
/// This class tracks them.
ਵਰਗ Delta_ਰੁੱਖ_Interior_ਗੰਢ : ਜਨਤਕ Delta_ਰੁੱਖ_ਗੰਢ {
  ਮਿੱਤਰ ਵਰਗ Delta_ਰੁੱਖ_ਗੰਢ;

  Delta_ਰੁੱਖ_ਗੰਢ *Children[2 * ਚੌੜਾਈ_ਗੁਣਕ];

  ~Delta_ਰੁੱਖ_Interior_ਗੰਢ() {
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ + 1; i != e; ++i)
      Children[i]->Destroy();
  }

ਜਨਤਕ:
  Delta_ਰੁੱਖ_Interior_ਗੰਢ() : Delta_ਰੁੱਖ_ਗੰਢ(ਝੂਠ /*nonleaf*/) {}

  Delta_ਰੁੱਖ_Interior_ਗੰਢ(ਅਚਲ ਪਾਓ_ਨਤੀਜਾ &IR)
      : Delta_ਰੁੱਖ_ਗੰਢ(ਝੂਠ /*nonleaf*/) {
    Children[0] = IR.LHS;
    Children[1] = IR.RHS;
    ਮੁੱਲਾਂ[0] = IR.ਵੰਡੋ_2;
    ਭਰਿਆ_Delta =
        IR.LHS->get_ਭਰਿਆ_Delta() + IR.RHS->get_ਭਰਿਆ_Delta() + IR.ਵੰਡੋ_2.Delta;
    ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ = 1;
  }

  ਅਚਲ Delta_ਰੁੱਖ_ਗੰਢ *get_ਬੱਚਾ(ਬਿਨਾਨਿਸ਼ਾਨੀ i) ਅਚਲ {
    ਪ੍ਰਤਿਜ੍ਞਾ(i < get_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ() + 1 && "Invalid child");
    ਵਾਪਸ Children[i];
  }

  Delta_ਰੁੱਖ_ਗੰਢ *get_ਬੱਚਾ(ਬਿਨਾਨਿਸ਼ਾਨੀ i) {
    ਪ੍ਰਤਿਜ੍ਞਾ(i < get_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ() + 1 && "Invalid child");
    ਵਾਪਸ Children[i];
  }

  ਸਥਿਰ ਤਾਰਕਿਕ classof(ਅਚਲ Delta_ਰੁੱਖ_ਗੰਢ *N) { ਵਾਪਸ !N->is_ਪੱਤੀ(); }
};

} // namespace

/// Destroy - A 'virtual' destructor.
ਖਾਲੀ Delta_ਰੁੱਖ_ਗੰਢ::Destroy() {
  ਜੇ (is_ਪੱਤੀ())
    ਮਿਟਾਓ ਇਹ;
  ਨਹੀਂਤਾਂ
    ਮਿਟਾਓ cast<Delta_ਰੁੱਖ_Interior_ਗੰਢ>(ਇਹ);
}

/// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
/// local walk over our contained deltas.
ਖਾਲੀ Delta_ਰੁੱਖ_ਗੰਢ::Recompute_ਭਰਿਆ_Delta_Locally() {
  ਪੂਰਨ New_ਭਰਿਆ_Delta = 0;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = get_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ(); i != e; ++i)
    New_ਭਰਿਆ_Delta += ਮੁੱਲਾਂ[i].Delta;
  ਜੇ (ਸਵੈਚਾਲਿਤ *ਵਿੱਚ_2 = dyn_cast<Delta_ਰੁੱਖ_Interior_ਗੰਢ>(ਇਹ))
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = get_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ() + 1; i != e; ++i)
      New_ਭਰਿਆ_Delta += ਵਿੱਚ_2->get_ਬੱਚਾ(i)->get_ਭਰਿਆ_Delta();
  ਭਰਿਆ_Delta = New_ਭਰਿਆ_Delta;
}

/// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
/// this node.  If insertion is easy, do it and return false.  Otherwise,
/// split the node, populate InsertRes with info about the split, and return
/// true.
ਤਾਰਕਿਕ Delta_ਰੁੱਖ_ਗੰਢ::DoInsertion(ਬਿਨਾਨਿਸ਼ਾਨੀ File_ਸੂਚਕਾਂਕ, ਪੂਰਨ Delta,
                                ਪਾਓ_ਨਤੀਜਾ *ਪਾਓ_Res) {
  // Maintain full delta for this node.
  ਭਰਿਆ_Delta += Delta;

  // Find the insertion point, the first delta whose index is >= FileIndex.
  ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = get_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ();
  ਜਦੋਂ (i != e && File_ਸੂਚਕਾਂਕ > get_ਮੁੱਲ(i).FileLoc)
    ++i;

  // If we found an a record for exactly this file index, just merge this
  // value into the pre-existing record and finish early.
  ਜੇ (i != e && get_ਮੁੱਲ(i).FileLoc == File_ਸੂਚਕਾਂਕ) {
    // NOTE: Delta could drop to zero here.  This means that the delta entry is
    // useless and could be removed.  Supporting erases is more complex than
    // leaving an entry with Delta=0, so we just leave an entry with Delta=0 in
    // the tree.
    ਮੁੱਲਾਂ[i].Delta += Delta;
    ਵਾਪਸ ਝੂਠ;
  }

  // Otherwise, we found an insertion point, and we know that the value at the
  // specified index is > FileIndex.  Handle the leaf case first.
  ਜੇ (is_ਪੱਤੀ()) {
    ਜੇ (!is_ਭਰਿਆ()) {
      // For an insertion into a non-full leaf node, just insert the value in
      // its sorted position.  This requires moving later values over.
      ਜੇ (i != e)
        memmove(&ਮੁੱਲਾਂ[i + 1], &ਮੁੱਲਾਂ[i], ਆਕਾਰ(ਮੁੱਲਾਂ[0]) * (e - i));
      ਮੁੱਲਾਂ[i] = ਸਰੋਤ_Delta::get(File_ਸੂਚਕਾਂਕ, Delta);
      ++ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ;
      ਵਾਪਸ ਝੂਠ;
    }

    // Otherwise, if this is leaf is full, split the node at its median, insert
    // the value into one of the children, and return the result.
    ਪ੍ਰਤਿਜ੍ਞਾ(ਪਾਓ_Res && "No result location specified");
    Do_ਵੰਡੋ(*ਪਾਓ_Res);

    ਜੇ (ਪਾਓ_Res->ਵੰਡੋ_2.FileLoc > File_ਸੂਚਕਾਂਕ)
      ਪਾਓ_Res->LHS->DoInsertion(File_ਸੂਚਕਾਂਕ, Delta, ਖਾਲੀ_ਸੂਚਕ /*can't fail*/);
    ਨਹੀਂਤਾਂ
      ਪਾਓ_Res->RHS->DoInsertion(File_ਸੂਚਕਾਂਕ, Delta, ਖਾਲੀ_ਸੂਚਕ /*can't fail*/);
    ਵਾਪਸ ਸੱਚ;
  }

  // Otherwise, this is an interior node.  Send the request down the tree.
  ਸਵੈਚਾਲਿਤ *ਵਿੱਚ_2 = cast<Delta_ਰੁੱਖ_Interior_ਗੰਢ>(ਇਹ);
  ਜੇ (!ਵਿੱਚ_2->Children[i]->DoInsertion(File_ਸੂਚਕਾਂਕ, Delta, ਪਾਓ_Res))
    ਵਾਪਸ ਝੂਠ; // If there was space in the child, just return.

  // Okay, this split the subtree, producing a new value and two children to
  // insert here.  If this node is non-full, we can just insert it directly.
  ਜੇ (!is_ਭਰਿਆ()) {
    // Now that we have two nodes and a new element, insert the perclated value
    // into ourself by moving all the later values/children down, then inserting
    // the new one.
    ਜੇ (i != e)
      memmove(&ਵਿੱਚ_2->Children[i + 2], &ਵਿੱਚ_2->Children[i + 1],
              (e - i) * ਆਕਾਰ(ਵਿੱਚ_2->Children[0]));
    ਵਿੱਚ_2->Children[i] = ਪਾਓ_Res->LHS;
    ਵਿੱਚ_2->Children[i + 1] = ਪਾਓ_Res->RHS;

    ਜੇ (e != i)
      memmove(&ਮੁੱਲਾਂ[i + 1], &ਮੁੱਲਾਂ[i], (e - i) * ਆਕਾਰ(ਮੁੱਲਾਂ[0]));
    ਮੁੱਲਾਂ[i] = ਪਾਓ_Res->ਵੰਡੋ_2;
    ++ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ;
    ਵਾਪਸ ਝੂਠ;
  }

  // Finally, if this interior node was full and a node is percolated up, split
  // ourself and return that up the chain.  Start by saving all our info to
  // avoid having the split clobber it.
  ਵਿੱਚ_2->Children[i] = ਪਾਓ_Res->LHS;
  Delta_ਰੁੱਖ_ਗੰਢ *SubRHS = ਪਾਓ_Res->RHS;
  ਸਰੋਤ_Delta Sub_ਵੰਡੋ = ਪਾਓ_Res->ਵੰਡੋ_2;

  // Do the split.
  Do_ਵੰਡੋ(*ਪਾਓ_Res);

  // Figure out where to insert SubRHS/NewSplit.
  Delta_ਰੁੱਖ_Interior_ਗੰਢ *ਪਾਓ_Side;
  ਜੇ (Sub_ਵੰਡੋ.FileLoc < ਪਾਓ_Res->ਵੰਡੋ_2.FileLoc)
    ਪਾਓ_Side = cast<Delta_ਰੁੱਖ_Interior_ਗੰਢ>(ਪਾਓ_Res->LHS);
  ਨਹੀਂਤਾਂ
    ਪਾਓ_Side = cast<Delta_ਰੁੱਖ_Interior_ਗੰਢ>(ਪਾਓ_Res->RHS);

  // We now have a non-empty interior node 'InsertSide' to insert
  // SubRHS/SubSplit into.  Find out where to insert SubSplit.

  // Find the insertion point, the first delta whose index is >SubSplit.FileLoc.
  i = 0;
  e = ਪਾਓ_Side->get_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ();
  ਜਦੋਂ (i != e && Sub_ਵੰਡੋ.FileLoc > ਪਾਓ_Side->get_ਮੁੱਲ(i).FileLoc)
    ++i;

  // Now we know that i is the place to insert the split value into.  Insert it
  // and the child right after it.
  ਜੇ (i != e)
    memmove(&ਪਾਓ_Side->Children[i + 2], &ਪਾਓ_Side->Children[i + 1],
            (e - i) * ਆਕਾਰ(ਵਿੱਚ_2->Children[0]));
  ਪਾਓ_Side->Children[i + 1] = SubRHS;

  ਜੇ (e != i)
    memmove(&ਪਾਓ_Side->ਮੁੱਲਾਂ[i + 1], &ਪਾਓ_Side->ਮੁੱਲਾਂ[i],
            (e - i) * ਆਕਾਰ(ਮੁੱਲਾਂ[0]));
  ਪਾਓ_Side->ਮੁੱਲਾਂ[i] = Sub_ਵੰਡੋ;
  ++ਪਾਓ_Side->ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ;
  ਪਾਓ_Side->ਭਰਿਆ_Delta += Sub_ਵੰਡੋ.Delta + SubRHS->get_ਭਰਿਆ_Delta();
  ਵਾਪਸ ਸੱਚ;
}

/// DoSplit - Split the currently full node (which has 2*WidthFactor-1 values)
/// into two subtrees each with "WidthFactor-1" values and a pivot value.
/// Return the pieces in InsertRes.
ਖਾਲੀ Delta_ਰੁੱਖ_ਗੰਢ::Do_ਵੰਡੋ(ਪਾਓ_ਨਤੀਜਾ &ਪਾਓ_Res) {
  ਪ੍ਰਤਿਜ੍ਞਾ(is_ਭਰਿਆ() && "Why split a non-full node?");

  // Since this node is full, it contains 2*WidthFactor-1 values.  We move
  // the first 'WidthFactor-1' values to the LHS child (which we leave in this
  // node), propagate one value up, and move the last 'WidthFactor-1' values
  // into the RHS child.

  // Create the new child node.
  Delta_ਰੁੱਖ_ਗੰਢ *New_ਗੰਢ;
  ਜੇ (ਸਵੈਚਾਲਿਤ *ਵਿੱਚ_2 = dyn_cast<Delta_ਰੁੱਖ_Interior_ਗੰਢ>(ਇਹ)) {
    // If this is an interior node, also move over 'WidthFactor' children
    // into the new node.
    Delta_ਰੁੱਖ_Interior_ਗੰਢ *New = ਨਵਾਂ Delta_ਰੁੱਖ_Interior_ਗੰਢ();
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&New->Children[0], &ਵਿੱਚ_2->Children[ਚੌੜਾਈ_ਗੁਣਕ],
           ਚੌੜਾਈ_ਗੁਣਕ * ਆਕਾਰ(ਵਿੱਚ_2->Children[0]));
    New_ਗੰਢ = New;
  } ਨਹੀਂਤਾਂ {
    // Just create the new leaf node.
    New_ਗੰਢ = ਨਵਾਂ Delta_ਰੁੱਖ_ਗੰਢ();
  }

  // Move over the last 'WidthFactor-1' values from here to NewNode.
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&New_ਗੰਢ->ਮੁੱਲਾਂ[0], &ਮੁੱਲਾਂ[ਚੌੜਾਈ_ਗੁਣਕ],
         (ਚੌੜਾਈ_ਗੁਣਕ - 1) * ਆਕਾਰ(ਮੁੱਲਾਂ[0]));

  // Decrease the number of values in the two nodes.
  New_ਗੰਢ->ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ = ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ = ਚੌੜਾਈ_ਗੁਣਕ - 1;

  // Recompute the two nodes' full delta.
  New_ਗੰਢ->Recompute_ਭਰਿਆ_Delta_Locally();
  Recompute_ਭਰਿਆ_Delta_Locally();

  ਪਾਓ_Res.LHS = ਇਹ;
  ਪਾਓ_Res.RHS = New_ਗੰਢ;
  ਪਾਓ_Res.ਵੰਡੋ_2 = ਮੁੱਲਾਂ[ਚੌੜਾਈ_ਗੁਣਕ - 1];
}

//===----------------------------------------------------------------------===//
//                        DeltaTree Implementation
//===----------------------------------------------------------------------===//

// #define VERIFY_TREE

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ VERIFY_ਰੁੱਖ
/// VerifyTree - Walk the btree performing assertions on various properties to
/// verify consistency.  This is useful for debugging new changes to the tree.
ਸਥਿਰ ਖਾਲੀ Verify_ਰੁੱਖ(ਅਚਲ Delta_ਰੁੱਖ_ਗੰਢ *N) {
  ਅਚਲ ਸਵੈਚਾਲਿਤ *ਵਿੱਚ_2 = dyn_cast<Delta_ਰੁੱਖ_Interior_ਗੰਢ>(N);
  ਜੇ (ਵਿੱਚ_2 == 0) {
    // Verify leaves, just ensure that FullDelta matches up and the elements
    // are in proper order.
    ਪੂਰਨ ਭਰਿਆ_Delta = 0;
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = N->get_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ(); i != e; ++i) {
      ਜੇ (i)
        ਪ੍ਰਤਿਜ੍ਞਾ(N->get_ਮੁੱਲ(i - 1).FileLoc < N->get_ਮੁੱਲ(i).FileLoc);
      ਭਰਿਆ_Delta += N->get_ਮੁੱਲ(i).Delta;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ(ਭਰਿਆ_Delta == N->get_ਭਰਿਆ_Delta());
    ਵਾਪਸ;
  }

  // Verify interior nodes: Ensure that FullDelta matches up and the
  // elements are in proper order and the children are in proper order.
  ਪੂਰਨ ਭਰਿਆ_Delta = 0;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = ਵਿੱਚ_2->get_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ(); i != e; ++i) {
    ਅਚਲ ਸਰੋਤ_Delta &IVal = N->get_ਮੁੱਲ(i);
    ਅਚਲ Delta_ਰੁੱਖ_ਗੰਢ *IChild = ਵਿੱਚ_2->get_ਬੱਚਾ(i);
    ਜੇ (i)
      ਪ੍ਰਤਿਜ੍ਞਾ(ਵਿੱਚ_2->get_ਮੁੱਲ(i - 1).FileLoc < IVal.FileLoc);
    ਭਰਿਆ_Delta += IVal.Delta;
    ਭਰਿਆ_Delta += IChild->get_ਭਰਿਆ_Delta();

    // The largest value in child #i should be smaller than FileLoc.
    ਪ੍ਰਤਿਜ੍ਞਾ(IChild->get_ਮੁੱਲ(IChild->get_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ() - 1).FileLoc <
           IVal.FileLoc);

    // The smallest value in child #i+1 should be larger than FileLoc.
    ਪ੍ਰਤਿਜ੍ਞਾ(ਵਿੱਚ_2->get_ਬੱਚਾ(i + 1)->get_ਮੁੱਲ(0).FileLoc > IVal.FileLoc);
    Verify_ਰੁੱਖ(IChild);
  }

  ਭਰਿਆ_Delta += ਵਿੱਚ_2->get_ਬੱਚਾ(ਵਿੱਚ_2->get_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ())->get_ਭਰਿਆ_Delta();

  ਪ੍ਰਤਿਜ੍ਞਾ(ਭਰਿਆ_Delta == N->get_ਭਰਿਆ_Delta());
}
#ਜੇ_ਅੰਤ // VERIFY_TREE

ਸਥਿਰ Delta_ਰੁੱਖ_ਗੰਢ *get_ਜੜ੍ਹ(ਖਾਲੀ *ਜੜ੍ਹ_2) { ਵਾਪਸ (Delta_ਰੁੱਖ_ਗੰਢ *)ਜੜ੍ਹ_2; }

Delta_ਰੁੱਖ::Delta_ਰੁੱਖ() { ਜੜ੍ਹ_2 = ਨਵਾਂ Delta_ਰੁੱਖ_ਗੰਢ(); }

Delta_ਰੁੱਖ::Delta_ਰੁੱਖ(ਅਚਲ Delta_ਰੁੱਖ &RHS) {
  // Currently we only support copying when the RHS is empty.
  ਪ੍ਰਤਿਜ੍ਞਾ(get_ਜੜ੍ਹ(RHS.ਜੜ੍ਹ_2)->get_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ() == 0 &&
         "Can only copy empty tree");
  ਜੜ੍ਹ_2 = ਨਵਾਂ Delta_ਰੁੱਖ_ਗੰਢ();
}

Delta_ਰੁੱਖ::~Delta_ਰੁੱਖ() { get_ਜੜ੍ਹ(ਜੜ੍ਹ_2)->Destroy(); }

/// getDeltaAt - Return the accumulated delta at the specified file offset.
/// This includes all insertions or delections that occurred *before* the
/// specified file index.
ਪੂਰਨ Delta_ਰੁੱਖ::getDeltaAt(ਬਿਨਾਨਿਸ਼ਾਨੀ File_ਸੂਚਕਾਂਕ) ਅਚਲ {
  ਅਚਲ Delta_ਰੁੱਖ_ਗੰਢ *ਗੰਢ = get_ਜੜ੍ਹ(ਜੜ੍ਹ_2);

  ਪੂਰਨ ਨਤੀਜਾ = 0;

  // Walk down the tree.
  ਜਦੋਂ (ਸੱਚ) {
    // For all nodes, include any local deltas before the specified file
    // index by summing them up directly.  Keep track of how many were
    // included.
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Vals_Greater = 0;
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ e = ਗੰਢ->get_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ(); ਅੰਕ_Vals_Greater != e;
         ++ਅੰਕ_Vals_Greater) {
      ਅਚਲ ਸਰੋਤ_Delta &Val = ਗੰਢ->get_ਮੁੱਲ(ਅੰਕ_Vals_Greater);

      ਜੇ (Val.FileLoc >= File_ਸੂਚਕਾਂਕ)
        ਰੋਕੋ;
      ਨਤੀਜਾ += Val.Delta;
    }

    // If we have an interior node, include information about children and
    // recurse.  Otherwise, if we have a leaf, we're done.
    ਅਚਲ ਸਵੈਚਾਲਿਤ *ਵਿੱਚ_2 = dyn_cast<Delta_ਰੁੱਖ_Interior_ਗੰਢ>(ਗੰਢ);
    ਜੇ (!ਵਿੱਚ_2)
      ਵਾਪਸ ਨਤੀਜਾ;

    // Include any children to the left of the values we skipped, all of
    // their deltas should be included as well.
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i != ਅੰਕ_Vals_Greater; ++i)
      ਨਤੀਜਾ += ਵਿੱਚ_2->get_ਬੱਚਾ(i)->get_ਭਰਿਆ_Delta();

    // If we found exactly the value we were looking for, break off the
    // search early.  There is no need to search the RHS of the value for
    // partial results.
    ਜੇ (ਅੰਕ_Vals_Greater != ਗੰਢ->get_ਅੰਕ_ਮੁੱਲਾਂ_ਵਰਤਿਆ() &&
        ਗੰਢ->get_ਮੁੱਲ(ਅੰਕ_Vals_Greater).FileLoc == File_ਸੂਚਕਾਂਕ)
      ਵਾਪਸ ਨਤੀਜਾ + ਵਿੱਚ_2->get_ਬੱਚਾ(ਅੰਕ_Vals_Greater)->get_ਭਰਿਆ_Delta();

    // Otherwise, traverse down the tree.  The selected subtree may be
    // partially included in the range.
    ਗੰਢ = ਵਿੱਚ_2->get_ਬੱਚਾ(ਅੰਕ_Vals_Greater);
  }
  // NOT REACHED.
}

/// AddDelta - When a change is made that shifts around the text buffer,
/// this method is used to record that info.  It inserts a delta of 'Delta'
/// into the current DeltaTree at offset FileIndex.
ਖਾਲੀ Delta_ਰੁੱਖ::ਜੋੜੋ_Delta(ਬਿਨਾਨਿਸ਼ਾਨੀ File_ਸੂਚਕਾਂਕ, ਪੂਰਨ Delta) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Delta && "Adding a noop?");
  Delta_ਰੁੱਖ_ਗੰਢ *My_ਜੜ੍ਹ = get_ਜੜ੍ਹ(ਜੜ੍ਹ_2);

  Delta_ਰੁੱਖ_ਗੰਢ::ਪਾਓ_ਨਤੀਜਾ ਪਾਓ_Res;
  ਜੇ (My_ਜੜ੍ਹ->DoInsertion(File_ਸੂਚਕਾਂਕ, Delta, &ਪਾਓ_Res)) {
    ਜੜ੍ਹ_2 = ਨਵਾਂ Delta_ਰੁੱਖ_Interior_ਗੰਢ(ਪਾਓ_Res);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ VERIFY_ਰੁੱਖ
    My_ਜੜ੍ਹ = ਜੜ੍ਹ_2;
#ਜੇ_ਅੰਤ
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ VERIFY_ਰੁੱਖ
  Verify_ਰੁੱਖ(My_ਜੜ੍ਹ);
#ਜੇ_ਅੰਤ
}
