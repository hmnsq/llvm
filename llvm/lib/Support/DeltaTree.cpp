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

#ضمن "llvm/ADT/DeltaTree.h"
#ضمن "llvm/Support/Casting.h"
#ضمن <cassert>
#ضمن <cstring>

استعمال نطاق llvm;

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

نطاق {

/// SourceDelta - As code in the original input buffer is added and deleted,
/// SourceDelta records are used to keep track of how the input SourceLocation
/// object is mapped into the output buffer.
بنية ذريعو_Delta {
  بنانشان FileLoc;
  صحيح Delta;

  ساكن ذريعو_Delta get(بنانشان Loc, صحيح D) {
    ذريعو_Delta Delta;
    Delta.FileLoc = Loc;
    Delta.Delta = D;
    أرجع Delta;
  }
};

/// DeltaTreeNode - The common part of all nodes.
///
صنف Delta_وڻ_ڳنڍ {
عام:
  بنية وجهو_نتيجو {
    Delta_وڻ_ڳنڍ *LHS, *RHS;
    ذريعو_Delta ورهايو_2;
  };

خاص:
  دوست صنف Delta_وڻ_Interior_ڳنڍ;

  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// WidthFactor-1 K/V pairs (except the root) and may have at most
  /// 2*WidthFactor-1 K/V pairs.
  تعداد { ويڪر_ضرب_ڪندڙ = 8 };

  /// Values - This tracks the SourceDelta's currently in this node.
  ذريعو_Delta قدرون[2 * ويڪر_ضرب_ڪندڙ - 1];

  /// NumValuesUsed - This tracks the number of values this node currently
  /// holds.
  بنانشان حرف انگ_قدرون_استعمال_ٿيل = 0;

  /// IsLeaf - This is true if this is a leaf of the btree.  If false, this is
  /// an interior node, and is actually an instance of DeltaTreeInteriorNode.
  منطقي Is_پن;

  /// FullDelta - This is the full delta of all the values in this node and
  /// all children nodes.
  صحيح ڀريل_Delta = 0;

عام:
  Delta_وڻ_ڳنڍ(منطقي is_پن = سچ) : Is_پن(is_پن) {}

  منطقي is_پن() ثابت { أرجع Is_پن; }
  صحيح get_ڀريل_Delta() ثابت { أرجع ڀريل_Delta; }
  منطقي is_ڀريل() ثابت { أرجع انگ_قدرون_استعمال_ٿيل == 2 * ويڪر_ضرب_ڪندڙ - 1; }

  بنانشان get_انگ_قدرون_استعمال_ٿيل() ثابت { أرجع انگ_قدرون_استعمال_ٿيل; }

  ثابت ذريعو_Delta &get_قدر(بنانشان i) ثابت {
    أكد(i < انگ_قدرون_استعمال_ٿيل && "Invalid value #");
    أرجع قدرون[i];
  }

  ذريعو_Delta &get_قدر(بنانشان i) {
    أكد(i < انگ_قدرون_استعمال_ٿيل && "Invalid value #");
    أرجع قدرون[i];
  }

  /// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
  /// this node.  If insertion is easy, do it and return false.  Otherwise,
  /// split the node, populate InsertRes with info about the split, and return
  /// true.
  منطقي DoInsertion(بنانشان File_اشاريو, صحيح Delta, وجهو_نتيجو *وجهو_Res);

  فراغ Do_ورهايو(وجهو_نتيجو &وجهو_Res);

  /// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
  /// local walk over our contained deltas.
  فراغ Recompute_ڀريل_Delta_Locally();

  فراغ Destroy();
};

/// DeltaTreeInteriorNode - When isLeaf = false, a node has child pointers.
/// This class tracks them.
صنف Delta_وڻ_Interior_ڳنڍ : عام Delta_وڻ_ڳنڍ {
  دوست صنف Delta_وڻ_ڳنڍ;

  Delta_وڻ_ڳنڍ *Children[2 * ويڪر_ضرب_ڪندڙ];

  ~Delta_وڻ_Interior_ڳنڍ() {
    لكل (بنانشان i = 0, e = انگ_قدرون_استعمال_ٿيل + 1; i != e; ++i)
      Children[i]->Destroy();
  }

عام:
  Delta_وڻ_Interior_ڳنڍ() : Delta_وڻ_ڳنڍ(خطأ /*nonleaf*/) {}

  Delta_وڻ_Interior_ڳنڍ(ثابت وجهو_نتيجو &IR)
      : Delta_وڻ_ڳنڍ(خطأ /*nonleaf*/) {
    Children[0] = IR.LHS;
    Children[1] = IR.RHS;
    قدرون[0] = IR.ورهايو_2;
    ڀريل_Delta =
        IR.LHS->get_ڀريل_Delta() + IR.RHS->get_ڀريل_Delta() + IR.ورهايو_2.Delta;
    انگ_قدرون_استعمال_ٿيل = 1;
  }

  ثابت Delta_وڻ_ڳنڍ *get_ٻار(بنانشان i) ثابت {
    أكد(i < get_انگ_قدرون_استعمال_ٿيل() + 1 && "Invalid child");
    أرجع Children[i];
  }

  Delta_وڻ_ڳنڍ *get_ٻار(بنانشان i) {
    أكد(i < get_انگ_قدرون_استعمال_ٿيل() + 1 && "Invalid child");
    أرجع Children[i];
  }

  ساكن منطقي classof(ثابت Delta_وڻ_ڳنڍ *N) { أرجع !N->is_پن(); }
};

} // namespace

/// Destroy - A 'virtual' destructor.
فراغ Delta_وڻ_ڳنڍ::Destroy() {
  إذا (is_پن())
    ميٽيو هذا;
  وإلا
    ميٽيو cast<Delta_وڻ_Interior_ڳنڍ>(هذا);
}

/// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
/// local walk over our contained deltas.
فراغ Delta_وڻ_ڳنڍ::Recompute_ڀريل_Delta_Locally() {
  صحيح New_ڀريل_Delta = 0;
  لكل (بنانشان i = 0, e = get_انگ_قدرون_استعمال_ٿيل(); i != e; ++i)
    New_ڀريل_Delta += قدرون[i].Delta;
  إذا (خودڪار *اندريون = dyn_cast<Delta_وڻ_Interior_ڳنڍ>(هذا))
    لكل (بنانشان i = 0, e = get_انگ_قدرون_استعمال_ٿيل() + 1; i != e; ++i)
      New_ڀريل_Delta += اندريون->get_ٻار(i)->get_ڀريل_Delta();
  ڀريل_Delta = New_ڀريل_Delta;
}

/// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
/// this node.  If insertion is easy, do it and return false.  Otherwise,
/// split the node, populate InsertRes with info about the split, and return
/// true.
منطقي Delta_وڻ_ڳنڍ::DoInsertion(بنانشان File_اشاريو, صحيح Delta,
                                وجهو_نتيجو *وجهو_Res) {
  // Maintain full delta for this node.
  ڀريل_Delta += Delta;

  // Find the insertion point, the first delta whose index is >= FileIndex.
  بنانشان i = 0, e = get_انگ_قدرون_استعمال_ٿيل();
  طالما (i != e && File_اشاريو > get_قدر(i).FileLoc)
    ++i;

  // If we found an a record for exactly this file index, just merge this
  // value into the pre-existing record and finish early.
  إذا (i != e && get_قدر(i).FileLoc == File_اشاريو) {
    // NOTE: Delta could drop to zero here.  This means that the delta entry is
    // useless and could be removed.  Supporting erases is more complex than
    // leaving an entry with Delta=0, so we just leave an entry with Delta=0 in
    // the tree.
    قدرون[i].Delta += Delta;
    أرجع خطأ;
  }

  // Otherwise, we found an insertion point, and we know that the value at the
  // specified index is > FileIndex.  Handle the leaf case first.
  إذا (is_پن()) {
    إذا (!is_ڀريل()) {
      // For an insertion into a non-full leaf node, just insert the value in
      // its sorted position.  This requires moving later values over.
      إذا (i != e)
        memmove(&قدرون[i + 1], &قدرون[i], حجم(قدرون[0]) * (e - i));
      قدرون[i] = ذريعو_Delta::get(File_اشاريو, Delta);
      ++انگ_قدرون_استعمال_ٿيل;
      أرجع خطأ;
    }

    // Otherwise, if this is leaf is full, split the node at its median, insert
    // the value into one of the children, and return the result.
    أكد(وجهو_Res && "No result location specified");
    Do_ورهايو(*وجهو_Res);

    إذا (وجهو_Res->ورهايو_2.FileLoc > File_اشاريو)
      وجهو_Res->LHS->DoInsertion(File_اشاريو, Delta, خالي_اشارو /*can't fail*/);
    وإلا
      وجهو_Res->RHS->DoInsertion(File_اشاريو, Delta, خالي_اشارو /*can't fail*/);
    أرجع سچ;
  }

  // Otherwise, this is an interior node.  Send the request down the tree.
  خودڪار *اندريون = cast<Delta_وڻ_Interior_ڳنڍ>(هذا);
  إذا (!اندريون->Children[i]->DoInsertion(File_اشاريو, Delta, وجهو_Res))
    أرجع خطأ; // If there was space in the child, just return.

  // Okay, this split the subtree, producing a new value and two children to
  // insert here.  If this node is non-full, we can just insert it directly.
  إذا (!is_ڀريل()) {
    // Now that we have two nodes and a new element, insert the perclated value
    // into ourself by moving all the later values/children down, then inserting
    // the new one.
    إذا (i != e)
      memmove(&اندريون->Children[i + 2], &اندريون->Children[i + 1],
              (e - i) * حجم(اندريون->Children[0]));
    اندريون->Children[i] = وجهو_Res->LHS;
    اندريون->Children[i + 1] = وجهو_Res->RHS;

    إذا (e != i)
      memmove(&قدرون[i + 1], &قدرون[i], (e - i) * حجم(قدرون[0]));
    قدرون[i] = وجهو_Res->ورهايو_2;
    ++انگ_قدرون_استعمال_ٿيل;
    أرجع خطأ;
  }

  // Finally, if this interior node was full and a node is percolated up, split
  // ourself and return that up the chain.  Start by saving all our info to
  // avoid having the split clobber it.
  اندريون->Children[i] = وجهو_Res->LHS;
  Delta_وڻ_ڳنڍ *SubRHS = وجهو_Res->RHS;
  ذريعو_Delta Sub_ورهايو = وجهو_Res->ورهايو_2;

  // Do the split.
  Do_ورهايو(*وجهو_Res);

  // Figure out where to insert SubRHS/NewSplit.
  Delta_وڻ_Interior_ڳنڍ *وجهو_Side;
  إذا (Sub_ورهايو.FileLoc < وجهو_Res->ورهايو_2.FileLoc)
    وجهو_Side = cast<Delta_وڻ_Interior_ڳنڍ>(وجهو_Res->LHS);
  وإلا
    وجهو_Side = cast<Delta_وڻ_Interior_ڳنڍ>(وجهو_Res->RHS);

  // We now have a non-empty interior node 'InsertSide' to insert
  // SubRHS/SubSplit into.  Find out where to insert SubSplit.

  // Find the insertion point, the first delta whose index is >SubSplit.FileLoc.
  i = 0;
  e = وجهو_Side->get_انگ_قدرون_استعمال_ٿيل();
  طالما (i != e && Sub_ورهايو.FileLoc > وجهو_Side->get_قدر(i).FileLoc)
    ++i;

  // Now we know that i is the place to insert the split value into.  Insert it
  // and the child right after it.
  إذا (i != e)
    memmove(&وجهو_Side->Children[i + 2], &وجهو_Side->Children[i + 1],
            (e - i) * حجم(اندريون->Children[0]));
  وجهو_Side->Children[i + 1] = SubRHS;

  إذا (e != i)
    memmove(&وجهو_Side->قدرون[i + 1], &وجهو_Side->قدرون[i],
            (e - i) * حجم(قدرون[0]));
  وجهو_Side->قدرون[i] = Sub_ورهايو;
  ++وجهو_Side->انگ_قدرون_استعمال_ٿيل;
  وجهو_Side->ڀريل_Delta += Sub_ورهايو.Delta + SubRHS->get_ڀريل_Delta();
  أرجع سچ;
}

/// DoSplit - Split the currently full node (which has 2*WidthFactor-1 values)
/// into two subtrees each with "WidthFactor-1" values and a pivot value.
/// Return the pieces in InsertRes.
فراغ Delta_وڻ_ڳنڍ::Do_ورهايو(وجهو_نتيجو &وجهو_Res) {
  أكد(is_ڀريل() && "Why split a non-full node?");

  // Since this node is full, it contains 2*WidthFactor-1 values.  We move
  // the first 'WidthFactor-1' values to the LHS child (which we leave in this
  // node), propagate one value up, and move the last 'WidthFactor-1' values
  // into the RHS child.

  // Create the new child node.
  Delta_وڻ_ڳنڍ *New_ڳنڍ;
  إذا (خودڪار *اندريون = dyn_cast<Delta_وڻ_Interior_ڳنڍ>(هذا)) {
    // If this is an interior node, also move over 'WidthFactor' children
    // into the new node.
    Delta_وڻ_Interior_ڳنڍ *New = جديد Delta_وڻ_Interior_ڳنڍ();
    يادگيري_نقل(&New->Children[0], &اندريون->Children[ويڪر_ضرب_ڪندڙ],
           ويڪر_ضرب_ڪندڙ * حجم(اندريون->Children[0]));
    New_ڳنڍ = New;
  } وإلا {
    // Just create the new leaf node.
    New_ڳنڍ = جديد Delta_وڻ_ڳنڍ();
  }

  // Move over the last 'WidthFactor-1' values from here to NewNode.
  يادگيري_نقل(&New_ڳنڍ->قدرون[0], &قدرون[ويڪر_ضرب_ڪندڙ],
         (ويڪر_ضرب_ڪندڙ - 1) * حجم(قدرون[0]));

  // Decrease the number of values in the two nodes.
  New_ڳنڍ->انگ_قدرون_استعمال_ٿيل = انگ_قدرون_استعمال_ٿيل = ويڪر_ضرب_ڪندڙ - 1;

  // Recompute the two nodes' full delta.
  New_ڳنڍ->Recompute_ڀريل_Delta_Locally();
  Recompute_ڀريل_Delta_Locally();

  وجهو_Res.LHS = هذا;
  وجهو_Res.RHS = New_ڳنڍ;
  وجهو_Res.ورهايو_2 = قدرون[ويڪر_ضرب_ڪندڙ - 1];
}

//===----------------------------------------------------------------------===//
//                        DeltaTree Implementation
//===----------------------------------------------------------------------===//

// #define VERIFY_TREE

#جيڪڏهن_وصف_ٿيل VERIFY_وڻ
/// VerifyTree - Walk the btree performing assertions on various properties to
/// verify consistency.  This is useful for debugging new changes to the tree.
ساكن فراغ Verify_وڻ(ثابت Delta_وڻ_ڳنڍ *N) {
  ثابت خودڪار *اندريون = dyn_cast<Delta_وڻ_Interior_ڳنڍ>(N);
  إذا (اندريون == 0) {
    // Verify leaves, just ensure that FullDelta matches up and the elements
    // are in proper order.
    صحيح ڀريل_Delta = 0;
    لكل (بنانشان i = 0, e = N->get_انگ_قدرون_استعمال_ٿيل(); i != e; ++i) {
      إذا (i)
        أكد(N->get_قدر(i - 1).FileLoc < N->get_قدر(i).FileLoc);
      ڀريل_Delta += N->get_قدر(i).Delta;
    }
    أكد(ڀريل_Delta == N->get_ڀريل_Delta());
    أرجع;
  }

  // Verify interior nodes: Ensure that FullDelta matches up and the
  // elements are in proper order and the children are in proper order.
  صحيح ڀريل_Delta = 0;
  لكل (بنانشان i = 0, e = اندريون->get_انگ_قدرون_استعمال_ٿيل(); i != e; ++i) {
    ثابت ذريعو_Delta &IVal = N->get_قدر(i);
    ثابت Delta_وڻ_ڳنڍ *IChild = اندريون->get_ٻار(i);
    إذا (i)
      أكد(اندريون->get_قدر(i - 1).FileLoc < IVal.FileLoc);
    ڀريل_Delta += IVal.Delta;
    ڀريل_Delta += IChild->get_ڀريل_Delta();

    // The largest value in child #i should be smaller than FileLoc.
    أكد(IChild->get_قدر(IChild->get_انگ_قدرون_استعمال_ٿيل() - 1).FileLoc <
           IVal.FileLoc);

    // The smallest value in child #i+1 should be larger than FileLoc.
    أكد(اندريون->get_ٻار(i + 1)->get_قدر(0).FileLoc > IVal.FileLoc);
    Verify_وڻ(IChild);
  }

  ڀريل_Delta += اندريون->get_ٻار(اندريون->get_انگ_قدرون_استعمال_ٿيل())->get_ڀريل_Delta();

  أكد(ڀريل_Delta == N->get_ڀريل_Delta());
}
#جيڪڏهن_پورو // VERIFY_TREE

ساكن Delta_وڻ_ڳنڍ *get_پاڙ(فراغ *پاڙ_2) { أرجع (Delta_وڻ_ڳنڍ *)پاڙ_2; }

Delta_وڻ::Delta_وڻ() { پاڙ_2 = جديد Delta_وڻ_ڳنڍ(); }

Delta_وڻ::Delta_وڻ(ثابت Delta_وڻ &RHS) {
  // Currently we only support copying when the RHS is empty.
  أكد(get_پاڙ(RHS.پاڙ_2)->get_انگ_قدرون_استعمال_ٿيل() == 0 &&
         "Can only copy empty tree");
  پاڙ_2 = جديد Delta_وڻ_ڳنڍ();
}

Delta_وڻ::~Delta_وڻ() { get_پاڙ(پاڙ_2)->Destroy(); }

/// getDeltaAt - Return the accumulated delta at the specified file offset.
/// This includes all insertions or delections that occurred *before* the
/// specified file index.
صحيح Delta_وڻ::getDeltaAt(بنانشان File_اشاريو) ثابت {
  ثابت Delta_وڻ_ڳنڍ *ڳنڍ = get_پاڙ(پاڙ_2);

  صحيح نتيجو = 0;

  // Walk down the tree.
  طالما (سچ) {
    // For all nodes, include any local deltas before the specified file
    // index by summing them up directly.  Keep track of how many were
    // included.
    بنانشان انگ_Vals_Greater = 0;
    لكل (بنانشان e = ڳنڍ->get_انگ_قدرون_استعمال_ٿيل(); انگ_Vals_Greater != e;
         ++انگ_Vals_Greater) {
      ثابت ذريعو_Delta &Val = ڳنڍ->get_قدر(انگ_Vals_Greater);

      إذا (Val.FileLoc >= File_اشاريو)
        اكسر;
      نتيجو += Val.Delta;
    }

    // If we have an interior node, include information about children and
    // recurse.  Otherwise, if we have a leaf, we're done.
    ثابت خودڪار *اندريون = dyn_cast<Delta_وڻ_Interior_ڳنڍ>(ڳنڍ);
    إذا (!اندريون)
      أرجع نتيجو;

    // Include any children to the left of the values we skipped, all of
    // their deltas should be included as well.
    لكل (بنانشان i = 0; i != انگ_Vals_Greater; ++i)
      نتيجو += اندريون->get_ٻار(i)->get_ڀريل_Delta();

    // If we found exactly the value we were looking for, break off the
    // search early.  There is no need to search the RHS of the value for
    // partial results.
    إذا (انگ_Vals_Greater != ڳنڍ->get_انگ_قدرون_استعمال_ٿيل() &&
        ڳنڍ->get_قدر(انگ_Vals_Greater).FileLoc == File_اشاريو)
      أرجع نتيجو + اندريون->get_ٻار(انگ_Vals_Greater)->get_ڀريل_Delta();

    // Otherwise, traverse down the tree.  The selected subtree may be
    // partially included in the range.
    ڳنڍ = اندريون->get_ٻار(انگ_Vals_Greater);
  }
  // NOT REACHED.
}

/// AddDelta - When a change is made that shifts around the text buffer,
/// this method is used to record that info.  It inserts a delta of 'Delta'
/// into the current DeltaTree at offset FileIndex.
فراغ Delta_وڻ::جوڙيو_Delta(بنانشان File_اشاريو, صحيح Delta) {
  أكد(Delta && "Adding a noop?");
  Delta_وڻ_ڳنڍ *My_پاڙ = get_پاڙ(پاڙ_2);

  Delta_وڻ_ڳنڍ::وجهو_نتيجو وجهو_Res;
  إذا (My_پاڙ->DoInsertion(File_اشاريو, Delta, &وجهو_Res)) {
    پاڙ_2 = جديد Delta_وڻ_Interior_ڳنڍ(وجهو_Res);
#جيڪڏهن_وصف_ٿيل VERIFY_وڻ
    My_پاڙ = پاڙ_2;
#جيڪڏهن_پورو
  }

#جيڪڏهن_وصف_ٿيل VERIFY_وڻ
  Verify_وڻ(My_پاڙ);
#جيڪڏهن_پورو
}
