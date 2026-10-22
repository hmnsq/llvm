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

کارول نطاق llvm;

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
بنية سرچینه_Delta {
  بینښه FileLoc;
  صحيح Delta;

  ساكن سرچینه_Delta get(بینښه Loc, صحيح D) {
    سرچینه_Delta Delta;
    Delta.FileLoc = Loc;
    Delta.Delta = D;
    أرجع Delta;
  }
};

/// DeltaTreeNode - The common part of all nodes.
///
صنف Delta_ونه_غوټه {
عام:
  بنية ورننباسه_پایله {
    Delta_ونه_غوټه *LHS, *RHS;
    سرچینه_Delta وېشل_2;
  };

خاص:
  ملګری صنف Delta_ونه_Interior_غوټه;

  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// WidthFactor-1 K/V pairs (except the root) and may have at most
  /// 2*WidthFactor-1 K/V pairs.
  تعداد { پلنوالی_ضریب = 8 };

  /// Values - This tracks the SourceDelta's currently in this node.
  سرچینه_Delta ارزښتونه[2 * پلنوالی_ضریب - 1];

  /// NumValuesUsed - This tracks the number of values this node currently
  /// holds.
  بینښه حرف عدد_ارزښتونه_کارول_شوی = 0;

  /// IsLeaf - This is true if this is a leaf of the btree.  If false, this is
  /// an interior node, and is actually an instance of DeltaTreeInteriorNode.
  منطقي Is_ونې_پاڼه;

  /// FullDelta - This is the full delta of all the values in this node and
  /// all children nodes.
  صحيح ډک_Delta = 0;

عام:
  Delta_ونه_غوټه(منطقي is_ونې_پاڼه = ریښتیا) : Is_ونې_پاڼه(is_ونې_پاڼه) {}

  منطقي is_ونې_پاڼه() ثابت { أرجع Is_ونې_پاڼه; }
  صحيح get_ډک_Delta() ثابت { أرجع ډک_Delta; }
  منطقي is_ډک() ثابت { أرجع عدد_ارزښتونه_کارول_شوی == 2 * پلنوالی_ضریب - 1; }

  بینښه get_عدد_ارزښتونه_کارول_شوی() ثابت { أرجع عدد_ارزښتونه_کارول_شوی; }

  ثابت سرچینه_Delta &get_ارزښت(بینښه i) ثابت {
    أكد(i < عدد_ارزښتونه_کارول_شوی && "Invalid value #");
    أرجع ارزښتونه[i];
  }

  سرچینه_Delta &get_ارزښت(بینښه i) {
    أكد(i < عدد_ارزښتونه_کارول_شوی && "Invalid value #");
    أرجع ارزښتونه[i];
  }

  /// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
  /// this node.  If insertion is easy, do it and return false.  Otherwise,
  /// split the node, populate InsertRes with info about the split, and return
  /// true.
  منطقي DoInsertion(بینښه File_شاخص, صحيح Delta, ورننباسه_پایله *ورننباسه_Res);

  فراغ Do_وېشل(ورننباسه_پایله &ورننباسه_Res);

  /// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
  /// local walk over our contained deltas.
  فراغ Recompute_ډک_Delta_Locally();

  فراغ Destroy();
};

/// DeltaTreeInteriorNode - When isLeaf = false, a node has child pointers.
/// This class tracks them.
صنف Delta_ونه_Interior_غوټه : عام Delta_ونه_غوټه {
  ملګری صنف Delta_ونه_غوټه;

  Delta_ونه_غوټه *Children[2 * پلنوالی_ضریب];

  ~Delta_ونه_Interior_غوټه() {
    لكل (بینښه i = 0, e = عدد_ارزښتونه_کارول_شوی + 1; i != e; ++i)
      Children[i]->Destroy();
  }

عام:
  Delta_ونه_Interior_غوټه() : Delta_ونه_غوټه(خطأ /*nonleaf*/) {}

  Delta_ونه_Interior_غوټه(ثابت ورننباسه_پایله &IR)
      : Delta_ونه_غوټه(خطأ /*nonleaf*/) {
    Children[0] = IR.LHS;
    Children[1] = IR.RHS;
    ارزښتونه[0] = IR.وېشل_2;
    ډک_Delta =
        IR.LHS->get_ډک_Delta() + IR.RHS->get_ډک_Delta() + IR.وېشل_2.Delta;
    عدد_ارزښتونه_کارول_شوی = 1;
  }

  ثابت Delta_ونه_غوټه *get_ماشوم(بینښه i) ثابت {
    أكد(i < get_عدد_ارزښتونه_کارول_شوی() + 1 && "Invalid child");
    أرجع Children[i];
  }

  Delta_ونه_غوټه *get_ماشوم(بینښه i) {
    أكد(i < get_عدد_ارزښتونه_کارول_شوی() + 1 && "Invalid child");
    أرجع Children[i];
  }

  ساكن منطقي classof(ثابت Delta_ونه_غوټه *N) { أرجع !N->is_ونې_پاڼه(); }
};

} // namespace

/// Destroy - A 'virtual' destructor.
فراغ Delta_ونه_غوټه::Destroy() {
  إذا (is_ونې_پاڼه())
    ړنګول هذا;
  وإلا
    ړنګول cast<Delta_ونه_Interior_غوټه>(هذا);
}

/// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
/// local walk over our contained deltas.
فراغ Delta_ونه_غوټه::Recompute_ډک_Delta_Locally() {
  صحيح New_ډک_Delta = 0;
  لكل (بینښه i = 0, e = get_عدد_ارزښتونه_کارول_شوی(); i != e; ++i)
    New_ډک_Delta += ارزښتونه[i].Delta;
  إذا (خپلکاری *دننه = dyn_cast<Delta_ونه_Interior_غوټه>(هذا))
    لكل (بینښه i = 0, e = get_عدد_ارزښتونه_کارول_شوی() + 1; i != e; ++i)
      New_ډک_Delta += دننه->get_ماشوم(i)->get_ډک_Delta();
  ډک_Delta = New_ډک_Delta;
}

/// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
/// this node.  If insertion is easy, do it and return false.  Otherwise,
/// split the node, populate InsertRes with info about the split, and return
/// true.
منطقي Delta_ونه_غوټه::DoInsertion(بینښه File_شاخص, صحيح Delta,
                                ورننباسه_پایله *ورننباسه_Res) {
  // Maintain full delta for this node.
  ډک_Delta += Delta;

  // Find the insertion point, the first delta whose index is >= FileIndex.
  بینښه i = 0, e = get_عدد_ارزښتونه_کارول_شوی();
  طالما (i != e && File_شاخص > get_ارزښت(i).FileLoc)
    ++i;

  // If we found an a record for exactly this file index, just merge this
  // value into the pre-existing record and finish early.
  إذا (i != e && get_ارزښت(i).FileLoc == File_شاخص) {
    // NOTE: Delta could drop to zero here.  This means that the delta entry is
    // useless and could be removed.  Supporting erases is more complex than
    // leaving an entry with Delta=0, so we just leave an entry with Delta=0 in
    // the tree.
    ارزښتونه[i].Delta += Delta;
    أرجع خطأ;
  }

  // Otherwise, we found an insertion point, and we know that the value at the
  // specified index is > FileIndex.  Handle the leaf case first.
  إذا (is_ونې_پاڼه()) {
    إذا (!is_ډک()) {
      // For an insertion into a non-full leaf node, just insert the value in
      // its sorted position.  This requires moving later values over.
      إذا (i != e)
        memmove(&ارزښتونه[i + 1], &ارزښتونه[i], حجم(ارزښتونه[0]) * (e - i));
      ارزښتونه[i] = سرچینه_Delta::get(File_شاخص, Delta);
      ++عدد_ارزښتونه_کارول_شوی;
      أرجع خطأ;
    }

    // Otherwise, if this is leaf is full, split the node at its median, insert
    // the value into one of the children, and return the result.
    أكد(ورننباسه_Res && "No result location specified");
    Do_وېشل(*ورننباسه_Res);

    إذا (ورننباسه_Res->وېشل_2.FileLoc > File_شاخص)
      ورننباسه_Res->LHS->DoInsertion(File_شاخص, Delta, تش_اشاره /*can't fail*/);
    وإلا
      ورننباسه_Res->RHS->DoInsertion(File_شاخص, Delta, تش_اشاره /*can't fail*/);
    أرجع ریښتیا;
  }

  // Otherwise, this is an interior node.  Send the request down the tree.
  خپلکاری *دننه = cast<Delta_ونه_Interior_غوټه>(هذا);
  إذا (!دننه->Children[i]->DoInsertion(File_شاخص, Delta, ورننباسه_Res))
    أرجع خطأ; // If there was space in the child, just return.

  // Okay, this split the subtree, producing a new value and two children to
  // insert here.  If this node is non-full, we can just insert it directly.
  إذا (!is_ډک()) {
    // Now that we have two nodes and a new element, insert the perclated value
    // into ourself by moving all the later values/children down, then inserting
    // the new one.
    إذا (i != e)
      memmove(&دننه->Children[i + 2], &دننه->Children[i + 1],
              (e - i) * حجم(دننه->Children[0]));
    دننه->Children[i] = ورننباسه_Res->LHS;
    دننه->Children[i + 1] = ورننباسه_Res->RHS;

    إذا (e != i)
      memmove(&ارزښتونه[i + 1], &ارزښتونه[i], (e - i) * حجم(ارزښتونه[0]));
    ارزښتونه[i] = ورننباسه_Res->وېشل_2;
    ++عدد_ارزښتونه_کارول_شوی;
    أرجع خطأ;
  }

  // Finally, if this interior node was full and a node is percolated up, split
  // ourself and return that up the chain.  Start by saving all our info to
  // avoid having the split clobber it.
  دننه->Children[i] = ورننباسه_Res->LHS;
  Delta_ونه_غوټه *SubRHS = ورننباسه_Res->RHS;
  سرچینه_Delta Sub_وېشل = ورننباسه_Res->وېشل_2;

  // Do the split.
  Do_وېشل(*ورننباسه_Res);

  // Figure out where to insert SubRHS/NewSplit.
  Delta_ونه_Interior_غوټه *ورننباسه_Side;
  إذا (Sub_وېشل.FileLoc < ورننباسه_Res->وېشل_2.FileLoc)
    ورننباسه_Side = cast<Delta_ونه_Interior_غوټه>(ورننباسه_Res->LHS);
  وإلا
    ورننباسه_Side = cast<Delta_ونه_Interior_غوټه>(ورننباسه_Res->RHS);

  // We now have a non-empty interior node 'InsertSide' to insert
  // SubRHS/SubSplit into.  Find out where to insert SubSplit.

  // Find the insertion point, the first delta whose index is >SubSplit.FileLoc.
  i = 0;
  e = ورننباسه_Side->get_عدد_ارزښتونه_کارول_شوی();
  طالما (i != e && Sub_وېشل.FileLoc > ورننباسه_Side->get_ارزښت(i).FileLoc)
    ++i;

  // Now we know that i is the place to insert the split value into.  Insert it
  // and the child right after it.
  إذا (i != e)
    memmove(&ورننباسه_Side->Children[i + 2], &ورننباسه_Side->Children[i + 1],
            (e - i) * حجم(دننه->Children[0]));
  ورننباسه_Side->Children[i + 1] = SubRHS;

  إذا (e != i)
    memmove(&ورننباسه_Side->ارزښتونه[i + 1], &ورننباسه_Side->ارزښتونه[i],
            (e - i) * حجم(ارزښتونه[0]));
  ورننباسه_Side->ارزښتونه[i] = Sub_وېشل;
  ++ورننباسه_Side->عدد_ارزښتونه_کارول_شوی;
  ورننباسه_Side->ډک_Delta += Sub_وېشل.Delta + SubRHS->get_ډک_Delta();
  أرجع ریښتیا;
}

/// DoSplit - Split the currently full node (which has 2*WidthFactor-1 values)
/// into two subtrees each with "WidthFactor-1" values and a pivot value.
/// Return the pieces in InsertRes.
فراغ Delta_ونه_غوټه::Do_وېشل(ورننباسه_پایله &ورننباسه_Res) {
  أكد(is_ډک() && "Why split a non-full node?");

  // Since this node is full, it contains 2*WidthFactor-1 values.  We move
  // the first 'WidthFactor-1' values to the LHS child (which we leave in this
  // node), propagate one value up, and move the last 'WidthFactor-1' values
  // into the RHS child.

  // Create the new child node.
  Delta_ونه_غوټه *New_غوټه;
  إذا (خپلکاری *دننه = dyn_cast<Delta_ونه_Interior_غوټه>(هذا)) {
    // If this is an interior node, also move over 'WidthFactor' children
    // into the new node.
    Delta_ونه_Interior_غوټه *New = جديد Delta_ونه_Interior_غوټه();
    حافظه_نقل(&New->Children[0], &دننه->Children[پلنوالی_ضریب],
           پلنوالی_ضریب * حجم(دننه->Children[0]));
    New_غوټه = New;
  } وإلا {
    // Just create the new leaf node.
    New_غوټه = جديد Delta_ونه_غوټه();
  }

  // Move over the last 'WidthFactor-1' values from here to NewNode.
  حافظه_نقل(&New_غوټه->ارزښتونه[0], &ارزښتونه[پلنوالی_ضریب],
         (پلنوالی_ضریب - 1) * حجم(ارزښتونه[0]));

  // Decrease the number of values in the two nodes.
  New_غوټه->عدد_ارزښتونه_کارول_شوی = عدد_ارزښتونه_کارول_شوی = پلنوالی_ضریب - 1;

  // Recompute the two nodes' full delta.
  New_غوټه->Recompute_ډک_Delta_Locally();
  Recompute_ډک_Delta_Locally();

  ورننباسه_Res.LHS = هذا;
  ورننباسه_Res.RHS = New_غوټه;
  ورننباسه_Res.وېشل_2 = ارزښتونه[پلنوالی_ضریب - 1];
}

//===----------------------------------------------------------------------===//
//                        DeltaTree Implementation
//===----------------------------------------------------------------------===//

// #define VERIFY_TREE

#که_تعریف_شوی VERIFY_ونه
/// VerifyTree - Walk the btree performing assertions on various properties to
/// verify consistency.  This is useful for debugging new changes to the tree.
ساكن فراغ Verify_ونه(ثابت Delta_ونه_غوټه *N) {
  ثابت خپلکاری *دننه = dyn_cast<Delta_ونه_Interior_غوټه>(N);
  إذا (دننه == 0) {
    // Verify leaves, just ensure that FullDelta matches up and the elements
    // are in proper order.
    صحيح ډک_Delta = 0;
    لكل (بینښه i = 0, e = N->get_عدد_ارزښتونه_کارول_شوی(); i != e; ++i) {
      إذا (i)
        أكد(N->get_ارزښت(i - 1).FileLoc < N->get_ارزښت(i).FileLoc);
      ډک_Delta += N->get_ارزښت(i).Delta;
    }
    أكد(ډک_Delta == N->get_ډک_Delta());
    أرجع;
  }

  // Verify interior nodes: Ensure that FullDelta matches up and the
  // elements are in proper order and the children are in proper order.
  صحيح ډک_Delta = 0;
  لكل (بینښه i = 0, e = دننه->get_عدد_ارزښتونه_کارول_شوی(); i != e; ++i) {
    ثابت سرچینه_Delta &IVal = N->get_ارزښت(i);
    ثابت Delta_ونه_غوټه *IChild = دننه->get_ماشوم(i);
    إذا (i)
      أكد(دننه->get_ارزښت(i - 1).FileLoc < IVal.FileLoc);
    ډک_Delta += IVal.Delta;
    ډک_Delta += IChild->get_ډک_Delta();

    // The largest value in child #i should be smaller than FileLoc.
    أكد(IChild->get_ارزښت(IChild->get_عدد_ارزښتونه_کارول_شوی() - 1).FileLoc <
           IVal.FileLoc);

    // The smallest value in child #i+1 should be larger than FileLoc.
    أكد(دننه->get_ماشوم(i + 1)->get_ارزښت(0).FileLoc > IVal.FileLoc);
    Verify_ونه(IChild);
  }

  ډک_Delta += دننه->get_ماشوم(دننه->get_عدد_ارزښتونه_کارول_شوی())->get_ډک_Delta();

  أكد(ډک_Delta == N->get_ډک_Delta());
}
#د_که_پای // VERIFY_TREE

ساكن Delta_ونه_غوټه *get_ریښه(فراغ *ریښه_2) { أرجع (Delta_ونه_غوټه *)ریښه_2; }

Delta_ونه::Delta_ونه() { ریښه_2 = جديد Delta_ونه_غوټه(); }

Delta_ونه::Delta_ونه(ثابت Delta_ونه &RHS) {
  // Currently we only support copying when the RHS is empty.
  أكد(get_ریښه(RHS.ریښه_2)->get_عدد_ارزښتونه_کارول_شوی() == 0 &&
         "Can only copy empty tree");
  ریښه_2 = جديد Delta_ونه_غوټه();
}

Delta_ونه::~Delta_ونه() { get_ریښه(ریښه_2)->Destroy(); }

/// getDeltaAt - Return the accumulated delta at the specified file offset.
/// This includes all insertions or delections that occurred *before* the
/// specified file index.
صحيح Delta_ونه::getDeltaAt(بینښه File_شاخص) ثابت {
  ثابت Delta_ونه_غوټه *غوټه = get_ریښه(ریښه_2);

  صحيح پایله = 0;

  // Walk down the tree.
  طالما (ریښتیا) {
    // For all nodes, include any local deltas before the specified file
    // index by summing them up directly.  Keep track of how many were
    // included.
    بینښه عدد_Vals_Greater = 0;
    لكل (بینښه e = غوټه->get_عدد_ارزښتونه_کارول_شوی(); عدد_Vals_Greater != e;
         ++عدد_Vals_Greater) {
      ثابت سرچینه_Delta &Val = غوټه->get_ارزښت(عدد_Vals_Greater);

      إذا (Val.FileLoc >= File_شاخص)
        اكسر;
      پایله += Val.Delta;
    }

    // If we have an interior node, include information about children and
    // recurse.  Otherwise, if we have a leaf, we're done.
    ثابت خپلکاری *دننه = dyn_cast<Delta_ونه_Interior_غوټه>(غوټه);
    إذا (!دننه)
      أرجع پایله;

    // Include any children to the left of the values we skipped, all of
    // their deltas should be included as well.
    لكل (بینښه i = 0; i != عدد_Vals_Greater; ++i)
      پایله += دننه->get_ماشوم(i)->get_ډک_Delta();

    // If we found exactly the value we were looking for, break off the
    // search early.  There is no need to search the RHS of the value for
    // partial results.
    إذا (عدد_Vals_Greater != غوټه->get_عدد_ارزښتونه_کارول_شوی() &&
        غوټه->get_ارزښت(عدد_Vals_Greater).FileLoc == File_شاخص)
      أرجع پایله + دننه->get_ماشوم(عدد_Vals_Greater)->get_ډک_Delta();

    // Otherwise, traverse down the tree.  The selected subtree may be
    // partially included in the range.
    غوټه = دننه->get_ماشوم(عدد_Vals_Greater);
  }
  // NOT REACHED.
}

/// AddDelta - When a change is made that shifts around the text buffer,
/// this method is used to record that info.  It inserts a delta of 'Delta'
/// into the current DeltaTree at offset FileIndex.
فراغ Delta_ونه::جمع_کړه_Delta(بینښه File_شاخص, صحيح Delta) {
  أكد(Delta && "Adding a noop?");
  Delta_ونه_غوټه *My_ریښه = get_ریښه(ریښه_2);

  Delta_ونه_غوټه::ورننباسه_پایله ورننباسه_Res;
  إذا (My_ریښه->DoInsertion(File_شاخص, Delta, &ورننباسه_Res)) {
    ریښه_2 = جديد Delta_ونه_Interior_غوټه(ورننباسه_Res);
#که_تعریف_شوی VERIFY_ونه
    My_ریښه = ریښه_2;
#د_که_پای
  }

#که_تعریف_شوی VERIFY_ونه
  Verify_ونه(My_ریښه);
#د_که_پای
}
