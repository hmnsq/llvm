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

استفاده نطاق llvm;

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
بنية منبع_Delta {
  بیعلامت FileLoc;
  صحيح Delta;

  ساكن منبع_Delta get(بیعلامت Loc, صحيح D) {
    منبع_Delta Delta;
    Delta.FileLoc = Loc;
    Delta.Delta = D;
    أرجع Delta;
  }
};

/// DeltaTreeNode - The common part of all nodes.
///
صنف Delta_درخت_گره {
عام:
  بنية درج_نتیجه {
    Delta_درخت_گره *LHS, *RHS;
    منبع_Delta تقسیم_2;
  };

خاص:
  دوست صنف Delta_درخت_Interior_گره;

  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// WidthFactor-1 K/V pairs (except the root) and may have at most
  /// 2*WidthFactor-1 K/V pairs.
  تعداد { عرض_ضریب = 8 };

  /// Values - This tracks the SourceDelta's currently in this node.
  منبع_Delta مقادیر[2 * عرض_ضریب - 1];

  /// NumValuesUsed - This tracks the number of values this node currently
  /// holds.
  بیعلامت حرف رقم_مقادیر_استفاده_شده = 0;

  /// IsLeaf - This is true if this is a leaf of the btree.  If false, this is
  /// an interior node, and is actually an instance of DeltaTreeInteriorNode.
  منطقي Is_برگ;

  /// FullDelta - This is the full delta of all the values in this node and
  /// all children nodes.
  صحيح پر_Delta = 0;

عام:
  Delta_درخت_گره(منطقي is_برگ = درست) : Is_برگ(is_برگ) {}

  منطقي is_برگ() ثابت { أرجع Is_برگ; }
  صحيح get_پر_Delta() ثابت { أرجع پر_Delta; }
  منطقي is_پر() ثابت { أرجع رقم_مقادیر_استفاده_شده == 2 * عرض_ضریب - 1; }

  بیعلامت get_رقم_مقادیر_استفاده_شده() ثابت { أرجع رقم_مقادیر_استفاده_شده; }

  ثابت منبع_Delta &get_مقدار(بیعلامت i) ثابت {
    أكد(i < رقم_مقادیر_استفاده_شده && "Invalid value #");
    أرجع مقادیر[i];
  }

  منبع_Delta &get_مقدار(بیعلامت i) {
    أكد(i < رقم_مقادیر_استفاده_شده && "Invalid value #");
    أرجع مقادیر[i];
  }

  /// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
  /// this node.  If insertion is easy, do it and return false.  Otherwise,
  /// split the node, populate InsertRes with info about the split, and return
  /// true.
  منطقي DoInsertion(بیعلامت File_شاخص, صحيح Delta, درج_نتیجه *درج_Res);

  فراغ Do_تقسیم(درج_نتیجه &درج_Res);

  /// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
  /// local walk over our contained deltas.
  فراغ Recompute_پر_Delta_Locally();

  فراغ Destroy();
};

/// DeltaTreeInteriorNode - When isLeaf = false, a node has child pointers.
/// This class tracks them.
صنف Delta_درخت_Interior_گره : عام Delta_درخت_گره {
  دوست صنف Delta_درخت_گره;

  Delta_درخت_گره *Children[2 * عرض_ضریب];

  ~Delta_درخت_Interior_گره() {
    لكل (بیعلامت i = 0, e = رقم_مقادیر_استفاده_شده + 1; i != e; ++i)
      Children[i]->Destroy();
  }

عام:
  Delta_درخت_Interior_گره() : Delta_درخت_گره(خطأ /*nonleaf*/) {}

  Delta_درخت_Interior_گره(ثابت درج_نتیجه &IR)
      : Delta_درخت_گره(خطأ /*nonleaf*/) {
    Children[0] = IR.LHS;
    Children[1] = IR.RHS;
    مقادیر[0] = IR.تقسیم_2;
    پر_Delta =
        IR.LHS->get_پر_Delta() + IR.RHS->get_پر_Delta() + IR.تقسیم_2.Delta;
    رقم_مقادیر_استفاده_شده = 1;
  }

  ثابت Delta_درخت_گره *get_فرزند(بیعلامت i) ثابت {
    أكد(i < get_رقم_مقادیر_استفاده_شده() + 1 && "Invalid child");
    أرجع Children[i];
  }

  Delta_درخت_گره *get_فرزند(بیعلامت i) {
    أكد(i < get_رقم_مقادیر_استفاده_شده() + 1 && "Invalid child");
    أرجع Children[i];
  }

  ساكن منطقي classof(ثابت Delta_درخت_گره *N) { أرجع !N->is_برگ(); }
};

} // namespace

/// Destroy - A 'virtual' destructor.
فراغ Delta_درخت_گره::Destroy() {
  إذا (is_برگ())
    حذف هذا;
  وإلا
    حذف cast<Delta_درخت_Interior_گره>(هذا);
}

/// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
/// local walk over our contained deltas.
فراغ Delta_درخت_گره::Recompute_پر_Delta_Locally() {
  صحيح New_پر_Delta = 0;
  لكل (بیعلامت i = 0, e = get_رقم_مقادیر_استفاده_شده(); i != e; ++i)
    New_پر_Delta += مقادیر[i].Delta;
  إذا (خودکار *درون = dyn_cast<Delta_درخت_Interior_گره>(هذا))
    لكل (بیعلامت i = 0, e = get_رقم_مقادیر_استفاده_شده() + 1; i != e; ++i)
      New_پر_Delta += درون->get_فرزند(i)->get_پر_Delta();
  پر_Delta = New_پر_Delta;
}

/// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
/// this node.  If insertion is easy, do it and return false.  Otherwise,
/// split the node, populate InsertRes with info about the split, and return
/// true.
منطقي Delta_درخت_گره::DoInsertion(بیعلامت File_شاخص, صحيح Delta,
                                درج_نتیجه *درج_Res) {
  // Maintain full delta for this node.
  پر_Delta += Delta;

  // Find the insertion point, the first delta whose index is >= FileIndex.
  بیعلامت i = 0, e = get_رقم_مقادیر_استفاده_شده();
  طالما (i != e && File_شاخص > get_مقدار(i).FileLoc)
    ++i;

  // If we found an a record for exactly this file index, just merge this
  // value into the pre-existing record and finish early.
  إذا (i != e && get_مقدار(i).FileLoc == File_شاخص) {
    // NOTE: Delta could drop to zero here.  This means that the delta entry is
    // useless and could be removed.  Supporting erases is more complex than
    // leaving an entry with Delta=0, so we just leave an entry with Delta=0 in
    // the tree.
    مقادیر[i].Delta += Delta;
    أرجع خطأ;
  }

  // Otherwise, we found an insertion point, and we know that the value at the
  // specified index is > FileIndex.  Handle the leaf case first.
  إذا (is_برگ()) {
    إذا (!is_پر()) {
      // For an insertion into a non-full leaf node, just insert the value in
      // its sorted position.  This requires moving later values over.
      إذا (i != e)
        memmove(&مقادیر[i + 1], &مقادیر[i], حجم(مقادیر[0]) * (e - i));
      مقادیر[i] = منبع_Delta::get(File_شاخص, Delta);
      ++رقم_مقادیر_استفاده_شده;
      أرجع خطأ;
    }

    // Otherwise, if this is leaf is full, split the node at its median, insert
    // the value into one of the children, and return the result.
    أكد(درج_Res && "No result location specified");
    Do_تقسیم(*درج_Res);

    إذا (درج_Res->تقسیم_2.FileLoc > File_شاخص)
      درج_Res->LHS->DoInsertion(File_شاخص, Delta, اشاره_تهی /*can't fail*/);
    وإلا
      درج_Res->RHS->DoInsertion(File_شاخص, Delta, اشاره_تهی /*can't fail*/);
    أرجع درست;
  }

  // Otherwise, this is an interior node.  Send the request down the tree.
  خودکار *درون = cast<Delta_درخت_Interior_گره>(هذا);
  إذا (!درون->Children[i]->DoInsertion(File_شاخص, Delta, درج_Res))
    أرجع خطأ; // If there was space in the child, just return.

  // Okay, this split the subtree, producing a new value and two children to
  // insert here.  If this node is non-full, we can just insert it directly.
  إذا (!is_پر()) {
    // Now that we have two nodes and a new element, insert the perclated value
    // into ourself by moving all the later values/children down, then inserting
    // the new one.
    إذا (i != e)
      memmove(&درون->Children[i + 2], &درون->Children[i + 1],
              (e - i) * حجم(درون->Children[0]));
    درون->Children[i] = درج_Res->LHS;
    درون->Children[i + 1] = درج_Res->RHS;

    إذا (e != i)
      memmove(&مقادیر[i + 1], &مقادیر[i], (e - i) * حجم(مقادیر[0]));
    مقادیر[i] = درج_Res->تقسیم_2;
    ++رقم_مقادیر_استفاده_شده;
    أرجع خطأ;
  }

  // Finally, if this interior node was full and a node is percolated up, split
  // ourself and return that up the chain.  Start by saving all our info to
  // avoid having the split clobber it.
  درون->Children[i] = درج_Res->LHS;
  Delta_درخت_گره *SubRHS = درج_Res->RHS;
  منبع_Delta Sub_تقسیم = درج_Res->تقسیم_2;

  // Do the split.
  Do_تقسیم(*درج_Res);

  // Figure out where to insert SubRHS/NewSplit.
  Delta_درخت_Interior_گره *درج_Side;
  إذا (Sub_تقسیم.FileLoc < درج_Res->تقسیم_2.FileLoc)
    درج_Side = cast<Delta_درخت_Interior_گره>(درج_Res->LHS);
  وإلا
    درج_Side = cast<Delta_درخت_Interior_گره>(درج_Res->RHS);

  // We now have a non-empty interior node 'InsertSide' to insert
  // SubRHS/SubSplit into.  Find out where to insert SubSplit.

  // Find the insertion point, the first delta whose index is >SubSplit.FileLoc.
  i = 0;
  e = درج_Side->get_رقم_مقادیر_استفاده_شده();
  طالما (i != e && Sub_تقسیم.FileLoc > درج_Side->get_مقدار(i).FileLoc)
    ++i;

  // Now we know that i is the place to insert the split value into.  Insert it
  // and the child right after it.
  إذا (i != e)
    memmove(&درج_Side->Children[i + 2], &درج_Side->Children[i + 1],
            (e - i) * حجم(درون->Children[0]));
  درج_Side->Children[i + 1] = SubRHS;

  إذا (e != i)
    memmove(&درج_Side->مقادیر[i + 1], &درج_Side->مقادیر[i],
            (e - i) * حجم(مقادیر[0]));
  درج_Side->مقادیر[i] = Sub_تقسیم;
  ++درج_Side->رقم_مقادیر_استفاده_شده;
  درج_Side->پر_Delta += Sub_تقسیم.Delta + SubRHS->get_پر_Delta();
  أرجع درست;
}

/// DoSplit - Split the currently full node (which has 2*WidthFactor-1 values)
/// into two subtrees each with "WidthFactor-1" values and a pivot value.
/// Return the pieces in InsertRes.
فراغ Delta_درخت_گره::Do_تقسیم(درج_نتیجه &درج_Res) {
  أكد(is_پر() && "Why split a non-full node?");

  // Since this node is full, it contains 2*WidthFactor-1 values.  We move
  // the first 'WidthFactor-1' values to the LHS child (which we leave in this
  // node), propagate one value up, and move the last 'WidthFactor-1' values
  // into the RHS child.

  // Create the new child node.
  Delta_درخت_گره *New_گره;
  إذا (خودکار *درون = dyn_cast<Delta_درخت_Interior_گره>(هذا)) {
    // If this is an interior node, also move over 'WidthFactor' children
    // into the new node.
    Delta_درخت_Interior_گره *New = جديد Delta_درخت_Interior_گره();
    کپی_حافظه(&New->Children[0], &درون->Children[عرض_ضریب],
           عرض_ضریب * حجم(درون->Children[0]));
    New_گره = New;
  } وإلا {
    // Just create the new leaf node.
    New_گره = جديد Delta_درخت_گره();
  }

  // Move over the last 'WidthFactor-1' values from here to NewNode.
  کپی_حافظه(&New_گره->مقادیر[0], &مقادیر[عرض_ضریب],
         (عرض_ضریب - 1) * حجم(مقادیر[0]));

  // Decrease the number of values in the two nodes.
  New_گره->رقم_مقادیر_استفاده_شده = رقم_مقادیر_استفاده_شده = عرض_ضریب - 1;

  // Recompute the two nodes' full delta.
  New_گره->Recompute_پر_Delta_Locally();
  Recompute_پر_Delta_Locally();

  درج_Res.LHS = هذا;
  درج_Res.RHS = New_گره;
  درج_Res.تقسیم_2 = مقادیر[عرض_ضریب - 1];
}

//===----------------------------------------------------------------------===//
//                        DeltaTree Implementation
//===----------------------------------------------------------------------===//

// #define VERIFY_TREE

#اگر_تعریف_شده VERIFY_درخت
/// VerifyTree - Walk the btree performing assertions on various properties to
/// verify consistency.  This is useful for debugging new changes to the tree.
ساكن فراغ Verify_درخت(ثابت Delta_درخت_گره *N) {
  ثابت خودکار *درون = dyn_cast<Delta_درخت_Interior_گره>(N);
  إذا (درون == 0) {
    // Verify leaves, just ensure that FullDelta matches up and the elements
    // are in proper order.
    صحيح پر_Delta = 0;
    لكل (بیعلامت i = 0, e = N->get_رقم_مقادیر_استفاده_شده(); i != e; ++i) {
      إذا (i)
        أكد(N->get_مقدار(i - 1).FileLoc < N->get_مقدار(i).FileLoc);
      پر_Delta += N->get_مقدار(i).Delta;
    }
    أكد(پر_Delta == N->get_پر_Delta());
    أرجع;
  }

  // Verify interior nodes: Ensure that FullDelta matches up and the
  // elements are in proper order and the children are in proper order.
  صحيح پر_Delta = 0;
  لكل (بیعلامت i = 0, e = درون->get_رقم_مقادیر_استفاده_شده(); i != e; ++i) {
    ثابت منبع_Delta &IVal = N->get_مقدار(i);
    ثابت Delta_درخت_گره *IChild = درون->get_فرزند(i);
    إذا (i)
      أكد(درون->get_مقدار(i - 1).FileLoc < IVal.FileLoc);
    پر_Delta += IVal.Delta;
    پر_Delta += IChild->get_پر_Delta();

    // The largest value in child #i should be smaller than FileLoc.
    أكد(IChild->get_مقدار(IChild->get_رقم_مقادیر_استفاده_شده() - 1).FileLoc <
           IVal.FileLoc);

    // The smallest value in child #i+1 should be larger than FileLoc.
    أكد(درون->get_فرزند(i + 1)->get_مقدار(0).FileLoc > IVal.FileLoc);
    Verify_درخت(IChild);
  }

  پر_Delta += درون->get_فرزند(درون->get_رقم_مقادیر_استفاده_شده())->get_پر_Delta();

  أكد(پر_Delta == N->get_پر_Delta());
}
#پایان_اگر // VERIFY_TREE

ساكن Delta_درخت_گره *get_ریشه(فراغ *ریشه_2) { أرجع (Delta_درخت_گره *)ریشه_2; }

Delta_درخت::Delta_درخت() { ریشه_2 = جديد Delta_درخت_گره(); }

Delta_درخت::Delta_درخت(ثابت Delta_درخت &RHS) {
  // Currently we only support copying when the RHS is empty.
  أكد(get_ریشه(RHS.ریشه_2)->get_رقم_مقادیر_استفاده_شده() == 0 &&
         "Can only copy empty tree");
  ریشه_2 = جديد Delta_درخت_گره();
}

Delta_درخت::~Delta_درخت() { get_ریشه(ریشه_2)->Destroy(); }

/// getDeltaAt - Return the accumulated delta at the specified file offset.
/// This includes all insertions or delections that occurred *before* the
/// specified file index.
صحيح Delta_درخت::getDeltaAt(بیعلامت File_شاخص) ثابت {
  ثابت Delta_درخت_گره *گره = get_ریشه(ریشه_2);

  صحيح نتیجه = 0;

  // Walk down the tree.
  طالما (درست) {
    // For all nodes, include any local deltas before the specified file
    // index by summing them up directly.  Keep track of how many were
    // included.
    بیعلامت رقم_Vals_Greater = 0;
    لكل (بیعلامت e = گره->get_رقم_مقادیر_استفاده_شده(); رقم_Vals_Greater != e;
         ++رقم_Vals_Greater) {
      ثابت منبع_Delta &Val = گره->get_مقدار(رقم_Vals_Greater);

      إذا (Val.FileLoc >= File_شاخص)
        اكسر;
      نتیجه += Val.Delta;
    }

    // If we have an interior node, include information about children and
    // recurse.  Otherwise, if we have a leaf, we're done.
    ثابت خودکار *درون = dyn_cast<Delta_درخت_Interior_گره>(گره);
    إذا (!درون)
      أرجع نتیجه;

    // Include any children to the left of the values we skipped, all of
    // their deltas should be included as well.
    لكل (بیعلامت i = 0; i != رقم_Vals_Greater; ++i)
      نتیجه += درون->get_فرزند(i)->get_پر_Delta();

    // If we found exactly the value we were looking for, break off the
    // search early.  There is no need to search the RHS of the value for
    // partial results.
    إذا (رقم_Vals_Greater != گره->get_رقم_مقادیر_استفاده_شده() &&
        گره->get_مقدار(رقم_Vals_Greater).FileLoc == File_شاخص)
      أرجع نتیجه + درون->get_فرزند(رقم_Vals_Greater)->get_پر_Delta();

    // Otherwise, traverse down the tree.  The selected subtree may be
    // partially included in the range.
    گره = درون->get_فرزند(رقم_Vals_Greater);
  }
  // NOT REACHED.
}

/// AddDelta - When a change is made that shifts around the text buffer,
/// this method is used to record that info.  It inserts a delta of 'Delta'
/// into the current DeltaTree at offset FileIndex.
فراغ Delta_درخت::جمع_کن_Delta(بیعلامت File_شاخص, صحيح Delta) {
  أكد(Delta && "Adding a noop?");
  Delta_درخت_گره *My_ریشه = get_ریشه(ریشه_2);

  Delta_درخت_گره::درج_نتیجه درج_Res;
  إذا (My_ریشه->DoInsertion(File_شاخص, Delta, &درج_Res)) {
    ریشه_2 = جديد Delta_درخت_Interior_گره(درج_Res);
#اگر_تعریف_شده VERIFY_درخت
    My_ریشه = ریشه_2;
#پایان_اگر
  }

#اگر_تعریف_شده VERIFY_درخت
  Verify_درخت(My_ریشه);
#پایان_اگر
}
