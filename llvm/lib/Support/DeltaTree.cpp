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

ورتوں نطاق llvm;

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
بنية سروت_Delta {
  بنانشانی FileLoc;
  صحيح Delta;

  ساكن سروت_Delta get(بنانشانی Loc, صحيح D) {
    سروت_Delta Delta;
    Delta.FileLoc = Loc;
    Delta.Delta = D;
    أرجع Delta;
  }
};

/// DeltaTreeNode - The common part of all nodes.
///
صنف Delta_درخت_گرہ {
عام:
  بنية داخل_کرو_نتیجہ {
    Delta_درخت_گرہ *LHS, *RHS;
    سروت_Delta ونڈو_2;
  };

خاص:
  یار صنف Delta_درخت_Interior_گرہ;

  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// WidthFactor-1 K/V pairs (except the root) and may have at most
  /// 2*WidthFactor-1 K/V pairs.
  تعداد { چوڑائی_ضارب = 8 };

  /// Values - This tracks the SourceDelta's currently in this node.
  سروت_Delta اقدار[2 * چوڑائی_ضارب - 1];

  /// NumValuesUsed - This tracks the number of values this node currently
  /// holds.
  بنانشانی حرف عدد_اقدار_ورتیا = 0;

  /// IsLeaf - This is true if this is a leaf of the btree.  If false, this is
  /// an interior node, and is actually an instance of DeltaTreeInteriorNode.
  منطقي Is_پتی;

  /// FullDelta - This is the full delta of all the values in this node and
  /// all children nodes.
  صحيح بھریا_Delta = 0;

عام:
  Delta_درخت_گرہ(منطقي is_پتی = سچ) : Is_پتی(is_پتی) {}

  منطقي is_پتی() ثابت { أرجع Is_پتی; }
  صحيح get_بھریا_Delta() ثابت { أرجع بھریا_Delta; }
  منطقي is_بھریا() ثابت { أرجع عدد_اقدار_ورتیا == 2 * چوڑائی_ضارب - 1; }

  بنانشانی get_عدد_اقدار_ورتیا() ثابت { أرجع عدد_اقدار_ورتیا; }

  ثابت سروت_Delta &get_قدر(بنانشانی i) ثابت {
    أكد(i < عدد_اقدار_ورتیا && "Invalid value #");
    أرجع اقدار[i];
  }

  سروت_Delta &get_قدر(بنانشانی i) {
    أكد(i < عدد_اقدار_ورتیا && "Invalid value #");
    أرجع اقدار[i];
  }

  /// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
  /// this node.  If insertion is easy, do it and return false.  Otherwise,
  /// split the node, populate InsertRes with info about the split, and return
  /// true.
  منطقي DoInsertion(بنانشانی File_اشاریہ, صحيح Delta, داخل_کرو_نتیجہ *داخل_کرو_Res);

  فراغ Do_ونڈو(داخل_کرو_نتیجہ &داخل_کرو_Res);

  /// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
  /// local walk over our contained deltas.
  فراغ Recompute_بھریا_Delta_Locally();

  فراغ Destroy();
};

/// DeltaTreeInteriorNode - When isLeaf = false, a node has child pointers.
/// This class tracks them.
صنف Delta_درخت_Interior_گرہ : عام Delta_درخت_گرہ {
  یار صنف Delta_درخت_گرہ;

  Delta_درخت_گرہ *Children[2 * چوڑائی_ضارب];

  ~Delta_درخت_Interior_گرہ() {
    لكل (بنانشانی i = 0, e = عدد_اقدار_ورتیا + 1; i != e; ++i)
      Children[i]->Destroy();
  }

عام:
  Delta_درخت_Interior_گرہ() : Delta_درخت_گرہ(خطأ /*nonleaf*/) {}

  Delta_درخت_Interior_گرہ(ثابت داخل_کرو_نتیجہ &IR)
      : Delta_درخت_گرہ(خطأ /*nonleaf*/) {
    Children[0] = IR.LHS;
    Children[1] = IR.RHS;
    اقدار[0] = IR.ونڈو_2;
    بھریا_Delta =
        IR.LHS->get_بھریا_Delta() + IR.RHS->get_بھریا_Delta() + IR.ونڈو_2.Delta;
    عدد_اقدار_ورتیا = 1;
  }

  ثابت Delta_درخت_گرہ *get_بچہ(بنانشانی i) ثابت {
    أكد(i < get_عدد_اقدار_ورتیا() + 1 && "Invalid child");
    أرجع Children[i];
  }

  Delta_درخت_گرہ *get_بچہ(بنانشانی i) {
    أكد(i < get_عدد_اقدار_ورتیا() + 1 && "Invalid child");
    أرجع Children[i];
  }

  ساكن منطقي classof(ثابت Delta_درخت_گرہ *N) { أرجع !N->is_پتی(); }
};

} // namespace

/// Destroy - A 'virtual' destructor.
فراغ Delta_درخت_گرہ::Destroy() {
  إذا (is_پتی())
    مٹاؤ هذا;
  وإلا
    مٹاؤ cast<Delta_درخت_Interior_گرہ>(هذا);
}

/// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
/// local walk over our contained deltas.
فراغ Delta_درخت_گرہ::Recompute_بھریا_Delta_Locally() {
  صحيح New_بھریا_Delta = 0;
  لكل (بنانشانی i = 0, e = get_عدد_اقدار_ورتیا(); i != e; ++i)
    New_بھریا_Delta += اقدار[i].Delta;
  إذا (خودکار *وچ = dyn_cast<Delta_درخت_Interior_گرہ>(هذا))
    لكل (بنانشانی i = 0, e = get_عدد_اقدار_ورتیا() + 1; i != e; ++i)
      New_بھریا_Delta += وچ->get_بچہ(i)->get_بھریا_Delta();
  بھریا_Delta = New_بھریا_Delta;
}

/// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
/// this node.  If insertion is easy, do it and return false.  Otherwise,
/// split the node, populate InsertRes with info about the split, and return
/// true.
منطقي Delta_درخت_گرہ::DoInsertion(بنانشانی File_اشاریہ, صحيح Delta,
                                داخل_کرو_نتیجہ *داخل_کرو_Res) {
  // Maintain full delta for this node.
  بھریا_Delta += Delta;

  // Find the insertion point, the first delta whose index is >= FileIndex.
  بنانشانی i = 0, e = get_عدد_اقدار_ورتیا();
  طالما (i != e && File_اشاریہ > get_قدر(i).FileLoc)
    ++i;

  // If we found an a record for exactly this file index, just merge this
  // value into the pre-existing record and finish early.
  إذا (i != e && get_قدر(i).FileLoc == File_اشاریہ) {
    // NOTE: Delta could drop to zero here.  This means that the delta entry is
    // useless and could be removed.  Supporting erases is more complex than
    // leaving an entry with Delta=0, so we just leave an entry with Delta=0 in
    // the tree.
    اقدار[i].Delta += Delta;
    أرجع خطأ;
  }

  // Otherwise, we found an insertion point, and we know that the value at the
  // specified index is > FileIndex.  Handle the leaf case first.
  إذا (is_پتی()) {
    إذا (!is_بھریا()) {
      // For an insertion into a non-full leaf node, just insert the value in
      // its sorted position.  This requires moving later values over.
      إذا (i != e)
        memmove(&اقدار[i + 1], &اقدار[i], حجم(اقدار[0]) * (e - i));
      اقدار[i] = سروت_Delta::get(File_اشاریہ, Delta);
      ++عدد_اقدار_ورتیا;
      أرجع خطأ;
    }

    // Otherwise, if this is leaf is full, split the node at its median, insert
    // the value into one of the children, and return the result.
    أكد(داخل_کرو_Res && "No result location specified");
    Do_ونڈو(*داخل_کرو_Res);

    إذا (داخل_کرو_Res->ونڈو_2.FileLoc > File_اشاریہ)
      داخل_کرو_Res->LHS->DoInsertion(File_اشاریہ, Delta, خالی_سوچک /*can't fail*/);
    وإلا
      داخل_کرو_Res->RHS->DoInsertion(File_اشاریہ, Delta, خالی_سوچک /*can't fail*/);
    أرجع سچ;
  }

  // Otherwise, this is an interior node.  Send the request down the tree.
  خودکار *وچ = cast<Delta_درخت_Interior_گرہ>(هذا);
  إذا (!وچ->Children[i]->DoInsertion(File_اشاریہ, Delta, داخل_کرو_Res))
    أرجع خطأ; // If there was space in the child, just return.

  // Okay, this split the subtree, producing a new value and two children to
  // insert here.  If this node is non-full, we can just insert it directly.
  إذا (!is_بھریا()) {
    // Now that we have two nodes and a new element, insert the perclated value
    // into ourself by moving all the later values/children down, then inserting
    // the new one.
    إذا (i != e)
      memmove(&وچ->Children[i + 2], &وچ->Children[i + 1],
              (e - i) * حجم(وچ->Children[0]));
    وچ->Children[i] = داخل_کرو_Res->LHS;
    وچ->Children[i + 1] = داخل_کرو_Res->RHS;

    إذا (e != i)
      memmove(&اقدار[i + 1], &اقدار[i], (e - i) * حجم(اقدار[0]));
    اقدار[i] = داخل_کرو_Res->ونڈو_2;
    ++عدد_اقدار_ورتیا;
    أرجع خطأ;
  }

  // Finally, if this interior node was full and a node is percolated up, split
  // ourself and return that up the chain.  Start by saving all our info to
  // avoid having the split clobber it.
  وچ->Children[i] = داخل_کرو_Res->LHS;
  Delta_درخت_گرہ *SubRHS = داخل_کرو_Res->RHS;
  سروت_Delta Sub_ونڈو = داخل_کرو_Res->ونڈو_2;

  // Do the split.
  Do_ونڈو(*داخل_کرو_Res);

  // Figure out where to insert SubRHS/NewSplit.
  Delta_درخت_Interior_گرہ *داخل_کرو_Side;
  إذا (Sub_ونڈو.FileLoc < داخل_کرو_Res->ونڈو_2.FileLoc)
    داخل_کرو_Side = cast<Delta_درخت_Interior_گرہ>(داخل_کرو_Res->LHS);
  وإلا
    داخل_کرو_Side = cast<Delta_درخت_Interior_گرہ>(داخل_کرو_Res->RHS);

  // We now have a non-empty interior node 'InsertSide' to insert
  // SubRHS/SubSplit into.  Find out where to insert SubSplit.

  // Find the insertion point, the first delta whose index is >SubSplit.FileLoc.
  i = 0;
  e = داخل_کرو_Side->get_عدد_اقدار_ورتیا();
  طالما (i != e && Sub_ونڈو.FileLoc > داخل_کرو_Side->get_قدر(i).FileLoc)
    ++i;

  // Now we know that i is the place to insert the split value into.  Insert it
  // and the child right after it.
  إذا (i != e)
    memmove(&داخل_کرو_Side->Children[i + 2], &داخل_کرو_Side->Children[i + 1],
            (e - i) * حجم(وچ->Children[0]));
  داخل_کرو_Side->Children[i + 1] = SubRHS;

  إذا (e != i)
    memmove(&داخل_کرو_Side->اقدار[i + 1], &داخل_کرو_Side->اقدار[i],
            (e - i) * حجم(اقدار[0]));
  داخل_کرو_Side->اقدار[i] = Sub_ونڈو;
  ++داخل_کرو_Side->عدد_اقدار_ورتیا;
  داخل_کرو_Side->بھریا_Delta += Sub_ونڈو.Delta + SubRHS->get_بھریا_Delta();
  أرجع سچ;
}

/// DoSplit - Split the currently full node (which has 2*WidthFactor-1 values)
/// into two subtrees each with "WidthFactor-1" values and a pivot value.
/// Return the pieces in InsertRes.
فراغ Delta_درخت_گرہ::Do_ونڈو(داخل_کرو_نتیجہ &داخل_کرو_Res) {
  أكد(is_بھریا() && "Why split a non-full node?");

  // Since this node is full, it contains 2*WidthFactor-1 values.  We move
  // the first 'WidthFactor-1' values to the LHS child (which we leave in this
  // node), propagate one value up, and move the last 'WidthFactor-1' values
  // into the RHS child.

  // Create the new child node.
  Delta_درخت_گرہ *New_گرہ;
  إذا (خودکار *وچ = dyn_cast<Delta_درخت_Interior_گرہ>(هذا)) {
    // If this is an interior node, also move over 'WidthFactor' children
    // into the new node.
    Delta_درخت_Interior_گرہ *New = جديد Delta_درخت_Interior_گرہ();
    حافظہ_نقل(&New->Children[0], &وچ->Children[چوڑائی_ضارب],
           چوڑائی_ضارب * حجم(وچ->Children[0]));
    New_گرہ = New;
  } وإلا {
    // Just create the new leaf node.
    New_گرہ = جديد Delta_درخت_گرہ();
  }

  // Move over the last 'WidthFactor-1' values from here to NewNode.
  حافظہ_نقل(&New_گرہ->اقدار[0], &اقدار[چوڑائی_ضارب],
         (چوڑائی_ضارب - 1) * حجم(اقدار[0]));

  // Decrease the number of values in the two nodes.
  New_گرہ->عدد_اقدار_ورتیا = عدد_اقدار_ورتیا = چوڑائی_ضارب - 1;

  // Recompute the two nodes' full delta.
  New_گرہ->Recompute_بھریا_Delta_Locally();
  Recompute_بھریا_Delta_Locally();

  داخل_کرو_Res.LHS = هذا;
  داخل_کرو_Res.RHS = New_گرہ;
  داخل_کرو_Res.ونڈو_2 = اقدار[چوڑائی_ضارب - 1];
}

//===----------------------------------------------------------------------===//
//                        DeltaTree Implementation
//===----------------------------------------------------------------------===//

// #define VERIFY_TREE

#جے_تعریف_شدہ VERIFY_درخت
/// VerifyTree - Walk the btree performing assertions on various properties to
/// verify consistency.  This is useful for debugging new changes to the tree.
ساكن فراغ Verify_درخت(ثابت Delta_درخت_گرہ *N) {
  ثابت خودکار *وچ = dyn_cast<Delta_درخت_Interior_گرہ>(N);
  إذا (وچ == 0) {
    // Verify leaves, just ensure that FullDelta matches up and the elements
    // are in proper order.
    صحيح بھریا_Delta = 0;
    لكل (بنانشانی i = 0, e = N->get_عدد_اقدار_ورتیا(); i != e; ++i) {
      إذا (i)
        أكد(N->get_قدر(i - 1).FileLoc < N->get_قدر(i).FileLoc);
      بھریا_Delta += N->get_قدر(i).Delta;
    }
    أكد(بھریا_Delta == N->get_بھریا_Delta());
    أرجع;
  }

  // Verify interior nodes: Ensure that FullDelta matches up and the
  // elements are in proper order and the children are in proper order.
  صحيح بھریا_Delta = 0;
  لكل (بنانشانی i = 0, e = وچ->get_عدد_اقدار_ورتیا(); i != e; ++i) {
    ثابت سروت_Delta &IVal = N->get_قدر(i);
    ثابت Delta_درخت_گرہ *IChild = وچ->get_بچہ(i);
    إذا (i)
      أكد(وچ->get_قدر(i - 1).FileLoc < IVal.FileLoc);
    بھریا_Delta += IVal.Delta;
    بھریا_Delta += IChild->get_بھریا_Delta();

    // The largest value in child #i should be smaller than FileLoc.
    أكد(IChild->get_قدر(IChild->get_عدد_اقدار_ورتیا() - 1).FileLoc <
           IVal.FileLoc);

    // The smallest value in child #i+1 should be larger than FileLoc.
    أكد(وچ->get_بچہ(i + 1)->get_قدر(0).FileLoc > IVal.FileLoc);
    Verify_درخت(IChild);
  }

  بھریا_Delta += وچ->get_بچہ(وچ->get_عدد_اقدار_ورتیا())->get_بھریا_Delta();

  أكد(بھریا_Delta == N->get_بھریا_Delta());
}
#جے_انت // VERIFY_TREE

ساكن Delta_درخت_گرہ *get_جڑ(فراغ *جڑ_2) { أرجع (Delta_درخت_گرہ *)جڑ_2; }

Delta_درخت::Delta_درخت() { جڑ_2 = جديد Delta_درخت_گرہ(); }

Delta_درخت::Delta_درخت(ثابت Delta_درخت &RHS) {
  // Currently we only support copying when the RHS is empty.
  أكد(get_جڑ(RHS.جڑ_2)->get_عدد_اقدار_ورتیا() == 0 &&
         "Can only copy empty tree");
  جڑ_2 = جديد Delta_درخت_گرہ();
}

Delta_درخت::~Delta_درخت() { get_جڑ(جڑ_2)->Destroy(); }

/// getDeltaAt - Return the accumulated delta at the specified file offset.
/// This includes all insertions or delections that occurred *before* the
/// specified file index.
صحيح Delta_درخت::getDeltaAt(بنانشانی File_اشاریہ) ثابت {
  ثابت Delta_درخت_گرہ *گرہ = get_جڑ(جڑ_2);

  صحيح نتیجہ = 0;

  // Walk down the tree.
  طالما (سچ) {
    // For all nodes, include any local deltas before the specified file
    // index by summing them up directly.  Keep track of how many were
    // included.
    بنانشانی عدد_Vals_Greater = 0;
    لكل (بنانشانی e = گرہ->get_عدد_اقدار_ورتیا(); عدد_Vals_Greater != e;
         ++عدد_Vals_Greater) {
      ثابت سروت_Delta &Val = گرہ->get_قدر(عدد_Vals_Greater);

      إذا (Val.FileLoc >= File_اشاریہ)
        اكسر;
      نتیجہ += Val.Delta;
    }

    // If we have an interior node, include information about children and
    // recurse.  Otherwise, if we have a leaf, we're done.
    ثابت خودکار *وچ = dyn_cast<Delta_درخت_Interior_گرہ>(گرہ);
    إذا (!وچ)
      أرجع نتیجہ;

    // Include any children to the left of the values we skipped, all of
    // their deltas should be included as well.
    لكل (بنانشانی i = 0; i != عدد_Vals_Greater; ++i)
      نتیجہ += وچ->get_بچہ(i)->get_بھریا_Delta();

    // If we found exactly the value we were looking for, break off the
    // search early.  There is no need to search the RHS of the value for
    // partial results.
    إذا (عدد_Vals_Greater != گرہ->get_عدد_اقدار_ورتیا() &&
        گرہ->get_قدر(عدد_Vals_Greater).FileLoc == File_اشاریہ)
      أرجع نتیجہ + وچ->get_بچہ(عدد_Vals_Greater)->get_بھریا_Delta();

    // Otherwise, traverse down the tree.  The selected subtree may be
    // partially included in the range.
    گرہ = وچ->get_بچہ(عدد_Vals_Greater);
  }
  // NOT REACHED.
}

/// AddDelta - When a change is made that shifts around the text buffer,
/// this method is used to record that info.  It inserts a delta of 'Delta'
/// into the current DeltaTree at offset FileIndex.
فراغ Delta_درخت::جوڑو_Delta(بنانشانی File_اشاریہ, صحيح Delta) {
  أكد(Delta && "Adding a noop?");
  Delta_درخت_گرہ *My_جڑ = get_جڑ(جڑ_2);

  Delta_درخت_گرہ::داخل_کرو_نتیجہ داخل_کرو_Res;
  إذا (My_جڑ->DoInsertion(File_اشاریہ, Delta, &داخل_کرو_Res)) {
    جڑ_2 = جديد Delta_درخت_Interior_گرہ(داخل_کرو_Res);
#جے_تعریف_شدہ VERIFY_درخت
    My_جڑ = جڑ_2;
#جے_انت
  }

#جے_تعریف_شدہ VERIFY_درخت
  Verify_درخت(My_جڑ);
#جے_انت
}
