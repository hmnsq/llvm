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

#شامل "llvm/ADT/DeltaTree.h"
#شامل "llvm/Support/Casting.h"
#شامل <cassert>
#شامل <cstring>

استعمال نامخلا llvm;

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

نامخلا {

/// SourceDelta - As code in the original input buffer is added and deleted,
/// SourceDelta records are used to keep track of how the input SourceLocation
/// object is mapped into the output buffer.
ساخت ماخذ_Delta {
  غیرعلامتی FileLoc;
  صحیح Delta;

  ساکن ماخذ_Delta get(غیرعلامتی Loc, صحیح D) {
    ماخذ_Delta Delta;
    Delta.FileLoc = Loc;
    Delta.Delta = D;
    واپس Delta;
  }
};

/// DeltaTreeNode - The common part of all nodes.
///
جماعت Delta_درخت_گرہ {
عوامی:
  ساخت داخل_کرو_نتیجہ {
    Delta_درخت_گرہ *LHS, *RHS;
    ماخذ_Delta بانٹو_2;
  };

نجی:
  دوست جماعت Delta_درخت_Interior_گرہ;

  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// WidthFactor-1 K/V pairs (except the root) and may have at most
  /// 2*WidthFactor-1 K/V pairs.
  شمار { چوڑائی_ضربی_عدد = 8 };

  /// Values - This tracks the SourceDelta's currently in this node.
  ماخذ_Delta اقدار[2 * چوڑائی_ضربی_عدد - 1];

  /// NumValuesUsed - This tracks the number of values this node currently
  /// holds.
  غیرعلامتی حرف عدد_اقدار_مستعمل = 0;

  /// IsLeaf - This is true if this is a leaf of the btree.  If false, this is
  /// an interior node, and is actually an instance of DeltaTreeInteriorNode.
  منطقی Is_برگ;

  /// FullDelta - This is the full delta of all the values in this node and
  /// all children nodes.
  صحیح بھرا_Delta = 0;

عوامی:
  Delta_درخت_گرہ(منطقی is_برگ = سچ) : Is_برگ(is_برگ) {}

  منطقی is_برگ() ثابت { واپس Is_برگ; }
  صحیح get_بھرا_Delta() ثابت { واپس بھرا_Delta; }
  منطقی is_بھرا() ثابت { واپس عدد_اقدار_مستعمل == 2 * چوڑائی_ضربی_عدد - 1; }

  غیرعلامتی get_عدد_اقدار_مستعمل() ثابت { واپس عدد_اقدار_مستعمل; }

  ثابت ماخذ_Delta &get_قدر(غیرعلامتی i) ثابت {
    أكد(i < عدد_اقدار_مستعمل && "Invalid value #");
    واپس اقدار[i];
  }

  ماخذ_Delta &get_قدر(غیرعلامتی i) {
    أكد(i < عدد_اقدار_مستعمل && "Invalid value #");
    واپس اقدار[i];
  }

  /// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
  /// this node.  If insertion is easy, do it and return false.  Otherwise,
  /// split the node, populate InsertRes with info about the split, and return
  /// true.
  منطقی DoInsertion(غیرعلامتی File_اشاریہ, صحیح Delta, داخل_کرو_نتیجہ *داخل_کرو_Res);

  خالی Do_بانٹو(داخل_کرو_نتیجہ &داخل_کرو_Res);

  /// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
  /// local walk over our contained deltas.
  خالی Recompute_بھرا_Delta_Locally();

  خالی Destroy();
};

/// DeltaTreeInteriorNode - When isLeaf = false, a node has child pointers.
/// This class tracks them.
جماعت Delta_درخت_Interior_گرہ : عوامی Delta_درخت_گرہ {
  دوست جماعت Delta_درخت_گرہ;

  Delta_درخت_گرہ *Children[2 * چوڑائی_ضربی_عدد];

  ~Delta_درخت_Interior_گرہ() {
    برائے (غیرعلامتی i = 0, e = عدد_اقدار_مستعمل + 1; i != e; ++i)
      Children[i]->Destroy();
  }

عوامی:
  Delta_درخت_Interior_گرہ() : Delta_درخت_گرہ(جھوٹ /*nonleaf*/) {}

  Delta_درخت_Interior_گرہ(ثابت داخل_کرو_نتیجہ &IR)
      : Delta_درخت_گرہ(جھوٹ /*nonleaf*/) {
    Children[0] = IR.LHS;
    Children[1] = IR.RHS;
    اقدار[0] = IR.بانٹو_2;
    بھرا_Delta =
        IR.LHS->get_بھرا_Delta() + IR.RHS->get_بھرا_Delta() + IR.بانٹو_2.Delta;
    عدد_اقدار_مستعمل = 1;
  }

  ثابت Delta_درخت_گرہ *get_بچہ(غیرعلامتی i) ثابت {
    أكد(i < get_عدد_اقدار_مستعمل() + 1 && "Invalid child");
    واپس Children[i];
  }

  Delta_درخت_گرہ *get_بچہ(غیرعلامتی i) {
    أكد(i < get_عدد_اقدار_مستعمل() + 1 && "Invalid child");
    واپس Children[i];
  }

  ساکن منطقی classof(ثابت Delta_درخت_گرہ *N) { واپس !N->is_برگ(); }
};

} // namespace

/// Destroy - A 'virtual' destructor.
خالی Delta_درخت_گرہ::Destroy() {
  اگر (is_برگ())
    حذف یہ;
  ورنہ
    حذف cast<Delta_درخت_Interior_گرہ>(یہ);
}

/// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
/// local walk over our contained deltas.
خالی Delta_درخت_گرہ::Recompute_بھرا_Delta_Locally() {
  صحیح New_بھرا_Delta = 0;
  برائے (غیرعلامتی i = 0, e = get_عدد_اقدار_مستعمل(); i != e; ++i)
    New_بھرا_Delta += اقدار[i].Delta;
  اگر (خودکار *اندر = dyn_cast<Delta_درخت_Interior_گرہ>(یہ))
    برائے (غیرعلامتی i = 0, e = get_عدد_اقدار_مستعمل() + 1; i != e; ++i)
      New_بھرا_Delta += اندر->get_بچہ(i)->get_بھرا_Delta();
  بھرا_Delta = New_بھرا_Delta;
}

/// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
/// this node.  If insertion is easy, do it and return false.  Otherwise,
/// split the node, populate InsertRes with info about the split, and return
/// true.
منطقی Delta_درخت_گرہ::DoInsertion(غیرعلامتی File_اشاریہ, صحیح Delta,
                                داخل_کرو_نتیجہ *داخل_کرو_Res) {
  // Maintain full delta for this node.
  بھرا_Delta += Delta;

  // Find the insertion point, the first delta whose index is >= FileIndex.
  غیرعلامتی i = 0, e = get_عدد_اقدار_مستعمل();
  جب (i != e && File_اشاریہ > get_قدر(i).FileLoc)
    ++i;

  // If we found an a record for exactly this file index, just merge this
  // value into the pre-existing record and finish early.
  اگر (i != e && get_قدر(i).FileLoc == File_اشاریہ) {
    // NOTE: Delta could drop to zero here.  This means that the delta entry is
    // useless and could be removed.  Supporting erases is more complex than
    // leaving an entry with Delta=0, so we just leave an entry with Delta=0 in
    // the tree.
    اقدار[i].Delta += Delta;
    واپس جھوٹ;
  }

  // Otherwise, we found an insertion point, and we know that the value at the
  // specified index is > FileIndex.  Handle the leaf case first.
  اگر (is_برگ()) {
    اگر (!is_بھرا()) {
      // For an insertion into a non-full leaf node, just insert the value in
      // its sorted position.  This requires moving later values over.
      اگر (i != e)
        memmove(&اقدار[i + 1], &اقدار[i], حجم(اقدار[0]) * (e - i));
      اقدار[i] = ماخذ_Delta::get(File_اشاریہ, Delta);
      ++عدد_اقدار_مستعمل;
      واپس جھوٹ;
    }

    // Otherwise, if this is leaf is full, split the node at its median, insert
    // the value into one of the children, and return the result.
    أكد(داخل_کرو_Res && "No result location specified");
    Do_بانٹو(*داخل_کرو_Res);

    اگر (داخل_کرو_Res->بانٹو_2.FileLoc > File_اشاریہ)
      داخل_کرو_Res->LHS->DoInsertion(File_اشاریہ, Delta, خالی_اشاریہ /*can't fail*/);
    ورنہ
      داخل_کرو_Res->RHS->DoInsertion(File_اشاریہ, Delta, خالی_اشاریہ /*can't fail*/);
    واپس سچ;
  }

  // Otherwise, this is an interior node.  Send the request down the tree.
  خودکار *اندر = cast<Delta_درخت_Interior_گرہ>(یہ);
  اگر (!اندر->Children[i]->DoInsertion(File_اشاریہ, Delta, داخل_کرو_Res))
    واپس جھوٹ; // If there was space in the child, just return.

  // Okay, this split the subtree, producing a new value and two children to
  // insert here.  If this node is non-full, we can just insert it directly.
  اگر (!is_بھرا()) {
    // Now that we have two nodes and a new element, insert the perclated value
    // into ourself by moving all the later values/children down, then inserting
    // the new one.
    اگر (i != e)
      memmove(&اندر->Children[i + 2], &اندر->Children[i + 1],
              (e - i) * حجم(اندر->Children[0]));
    اندر->Children[i] = داخل_کرو_Res->LHS;
    اندر->Children[i + 1] = داخل_کرو_Res->RHS;

    اگر (e != i)
      memmove(&اقدار[i + 1], &اقدار[i], (e - i) * حجم(اقدار[0]));
    اقدار[i] = داخل_کرو_Res->بانٹو_2;
    ++عدد_اقدار_مستعمل;
    واپس جھوٹ;
  }

  // Finally, if this interior node was full and a node is percolated up, split
  // ourself and return that up the chain.  Start by saving all our info to
  // avoid having the split clobber it.
  اندر->Children[i] = داخل_کرو_Res->LHS;
  Delta_درخت_گرہ *SubRHS = داخل_کرو_Res->RHS;
  ماخذ_Delta Sub_بانٹو = داخل_کرو_Res->بانٹو_2;

  // Do the split.
  Do_بانٹو(*داخل_کرو_Res);

  // Figure out where to insert SubRHS/NewSplit.
  Delta_درخت_Interior_گرہ *داخل_کرو_Side;
  اگر (Sub_بانٹو.FileLoc < داخل_کرو_Res->بانٹو_2.FileLoc)
    داخل_کرو_Side = cast<Delta_درخت_Interior_گرہ>(داخل_کرو_Res->LHS);
  ورنہ
    داخل_کرو_Side = cast<Delta_درخت_Interior_گرہ>(داخل_کرو_Res->RHS);

  // We now have a non-empty interior node 'InsertSide' to insert
  // SubRHS/SubSplit into.  Find out where to insert SubSplit.

  // Find the insertion point, the first delta whose index is >SubSplit.FileLoc.
  i = 0;
  e = داخل_کرو_Side->get_عدد_اقدار_مستعمل();
  جب (i != e && Sub_بانٹو.FileLoc > داخل_کرو_Side->get_قدر(i).FileLoc)
    ++i;

  // Now we know that i is the place to insert the split value into.  Insert it
  // and the child right after it.
  اگر (i != e)
    memmove(&داخل_کرو_Side->Children[i + 2], &داخل_کرو_Side->Children[i + 1],
            (e - i) * حجم(اندر->Children[0]));
  داخل_کرو_Side->Children[i + 1] = SubRHS;

  اگر (e != i)
    memmove(&داخل_کرو_Side->اقدار[i + 1], &داخل_کرو_Side->اقدار[i],
            (e - i) * حجم(اقدار[0]));
  داخل_کرو_Side->اقدار[i] = Sub_بانٹو;
  ++داخل_کرو_Side->عدد_اقدار_مستعمل;
  داخل_کرو_Side->بھرا_Delta += Sub_بانٹو.Delta + SubRHS->get_بھرا_Delta();
  واپس سچ;
}

/// DoSplit - Split the currently full node (which has 2*WidthFactor-1 values)
/// into two subtrees each with "WidthFactor-1" values and a pivot value.
/// Return the pieces in InsertRes.
خالی Delta_درخت_گرہ::Do_بانٹو(داخل_کرو_نتیجہ &داخل_کرو_Res) {
  أكد(is_بھرا() && "Why split a non-full node?");

  // Since this node is full, it contains 2*WidthFactor-1 values.  We move
  // the first 'WidthFactor-1' values to the LHS child (which we leave in this
  // node), propagate one value up, and move the last 'WidthFactor-1' values
  // into the RHS child.

  // Create the new child node.
  Delta_درخت_گرہ *New_گرہ;
  اگر (خودکار *اندر = dyn_cast<Delta_درخت_Interior_گرہ>(یہ)) {
    // If this is an interior node, also move over 'WidthFactor' children
    // into the new node.
    Delta_درخت_Interior_گرہ *New = نیا Delta_درخت_Interior_گرہ();
    حافظہ_نقل(&New->Children[0], &اندر->Children[چوڑائی_ضربی_عدد],
           چوڑائی_ضربی_عدد * حجم(اندر->Children[0]));
    New_گرہ = New;
  } ورنہ {
    // Just create the new leaf node.
    New_گرہ = نیا Delta_درخت_گرہ();
  }

  // Move over the last 'WidthFactor-1' values from here to NewNode.
  حافظہ_نقل(&New_گرہ->اقدار[0], &اقدار[چوڑائی_ضربی_عدد],
         (چوڑائی_ضربی_عدد - 1) * حجم(اقدار[0]));

  // Decrease the number of values in the two nodes.
  New_گرہ->عدد_اقدار_مستعمل = عدد_اقدار_مستعمل = چوڑائی_ضربی_عدد - 1;

  // Recompute the two nodes' full delta.
  New_گرہ->Recompute_بھرا_Delta_Locally();
  Recompute_بھرا_Delta_Locally();

  داخل_کرو_Res.LHS = یہ;
  داخل_کرو_Res.RHS = New_گرہ;
  داخل_کرو_Res.بانٹو_2 = اقدار[چوڑائی_ضربی_عدد - 1];
}

//===----------------------------------------------------------------------===//
//                        DeltaTree Implementation
//===----------------------------------------------------------------------===//

// #define VERIFY_TREE

#اگر_تعریف_شدہ VERIFY_درخت
/// VerifyTree - Walk the btree performing assertions on various properties to
/// verify consistency.  This is useful for debugging new changes to the tree.
ساکن خالی Verify_درخت(ثابت Delta_درخت_گرہ *N) {
  ثابت خودکار *اندر = dyn_cast<Delta_درخت_Interior_گرہ>(N);
  اگر (اندر == 0) {
    // Verify leaves, just ensure that FullDelta matches up and the elements
    // are in proper order.
    صحیح بھرا_Delta = 0;
    برائے (غیرعلامتی i = 0, e = N->get_عدد_اقدار_مستعمل(); i != e; ++i) {
      اگر (i)
        أكد(N->get_قدر(i - 1).FileLoc < N->get_قدر(i).FileLoc);
      بھرا_Delta += N->get_قدر(i).Delta;
    }
    أكد(بھرا_Delta == N->get_بھرا_Delta());
    واپس;
  }

  // Verify interior nodes: Ensure that FullDelta matches up and the
  // elements are in proper order and the children are in proper order.
  صحیح بھرا_Delta = 0;
  برائے (غیرعلامتی i = 0, e = اندر->get_عدد_اقدار_مستعمل(); i != e; ++i) {
    ثابت ماخذ_Delta &IVal = N->get_قدر(i);
    ثابت Delta_درخت_گرہ *IChild = اندر->get_بچہ(i);
    اگر (i)
      أكد(اندر->get_قدر(i - 1).FileLoc < IVal.FileLoc);
    بھرا_Delta += IVal.Delta;
    بھرا_Delta += IChild->get_بھرا_Delta();

    // The largest value in child #i should be smaller than FileLoc.
    أكد(IChild->get_قدر(IChild->get_عدد_اقدار_مستعمل() - 1).FileLoc <
           IVal.FileLoc);

    // The smallest value in child #i+1 should be larger than FileLoc.
    أكد(اندر->get_بچہ(i + 1)->get_قدر(0).FileLoc > IVal.FileLoc);
    Verify_درخت(IChild);
  }

  بھرا_Delta += اندر->get_بچہ(اندر->get_عدد_اقدار_مستعمل())->get_بھرا_Delta();

  أكد(بھرا_Delta == N->get_بھرا_Delta());
}
#اگر_ختم // VERIFY_TREE

ساکن Delta_درخت_گرہ *get_جڑ(خالی *جڑ_2) { واپس (Delta_درخت_گرہ *)جڑ_2; }

Delta_درخت::Delta_درخت() { جڑ_2 = نیا Delta_درخت_گرہ(); }

Delta_درخت::Delta_درخت(ثابت Delta_درخت &RHS) {
  // Currently we only support copying when the RHS is empty.
  أكد(get_جڑ(RHS.جڑ_2)->get_عدد_اقدار_مستعمل() == 0 &&
         "Can only copy empty tree");
  جڑ_2 = نیا Delta_درخت_گرہ();
}

Delta_درخت::~Delta_درخت() { get_جڑ(جڑ_2)->Destroy(); }

/// getDeltaAt - Return the accumulated delta at the specified file offset.
/// This includes all insertions or delections that occurred *before* the
/// specified file index.
صحیح Delta_درخت::getDeltaAt(غیرعلامتی File_اشاریہ) ثابت {
  ثابت Delta_درخت_گرہ *گرہ = get_جڑ(جڑ_2);

  صحیح نتیجہ = 0;

  // Walk down the tree.
  جب (سچ) {
    // For all nodes, include any local deltas before the specified file
    // index by summing them up directly.  Keep track of how many were
    // included.
    غیرعلامتی عدد_Vals_Greater = 0;
    برائے (غیرعلامتی e = گرہ->get_عدد_اقدار_مستعمل(); عدد_Vals_Greater != e;
         ++عدد_Vals_Greater) {
      ثابت ماخذ_Delta &Val = گرہ->get_قدر(عدد_Vals_Greater);

      اگر (Val.FileLoc >= File_اشاریہ)
        توڑو;
      نتیجہ += Val.Delta;
    }

    // If we have an interior node, include information about children and
    // recurse.  Otherwise, if we have a leaf, we're done.
    ثابت خودکار *اندر = dyn_cast<Delta_درخت_Interior_گرہ>(گرہ);
    اگر (!اندر)
      واپس نتیجہ;

    // Include any children to the left of the values we skipped, all of
    // their deltas should be included as well.
    برائے (غیرعلامتی i = 0; i != عدد_Vals_Greater; ++i)
      نتیجہ += اندر->get_بچہ(i)->get_بھرا_Delta();

    // If we found exactly the value we were looking for, break off the
    // search early.  There is no need to search the RHS of the value for
    // partial results.
    اگر (عدد_Vals_Greater != گرہ->get_عدد_اقدار_مستعمل() &&
        گرہ->get_قدر(عدد_Vals_Greater).FileLoc == File_اشاریہ)
      واپس نتیجہ + اندر->get_بچہ(عدد_Vals_Greater)->get_بھرا_Delta();

    // Otherwise, traverse down the tree.  The selected subtree may be
    // partially included in the range.
    گرہ = اندر->get_بچہ(عدد_Vals_Greater);
  }
  // NOT REACHED.
}

/// AddDelta - When a change is made that shifts around the text buffer,
/// this method is used to record that info.  It inserts a delta of 'Delta'
/// into the current DeltaTree at offset FileIndex.
خالی Delta_درخت::جوڑو_Delta(غیرعلامتی File_اشاریہ, صحیح Delta) {
  أكد(Delta && "Adding a noop?");
  Delta_درخت_گرہ *My_جڑ = get_جڑ(جڑ_2);

  Delta_درخت_گرہ::داخل_کرو_نتیجہ داخل_کرو_Res;
  اگر (My_جڑ->DoInsertion(File_اشاریہ, Delta, &داخل_کرو_Res)) {
    جڑ_2 = نیا Delta_درخت_Interior_گرہ(داخل_کرو_Res);
#اگر_تعریف_شدہ VERIFY_درخت
    My_جڑ = جڑ_2;
#اگر_ختم
  }

#اگر_تعریف_شدہ VERIFY_درخت
  Verify_درخت(My_جڑ);
#اگر_ختم
}
