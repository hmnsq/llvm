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

using نطاق llvm;

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
بنية SourceDelta {
  unsigned FileLoc;
  صحيح Delta;

  ساكن SourceDelta get(unsigned Loc, صحيح D) {
    SourceDelta Delta;
    Delta.FileLoc = Loc;
    Delta.Delta = D;
    أرجع Delta;
  }
};

/// DeltaTreeNode - The common part of all nodes.
///
صنف Delta_درخت_گرہ {
عام:
  بنية داخل_کرو_Result {
    Delta_درخت_گرہ *LHS, *RHS;
    SourceDelta Split;
  };

خاص:
  friend صنف Delta_درخت_Interior_گرہ;

  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// WidthFactor-1 K/V pairs (except the root) and may have at most
  /// 2*WidthFactor-1 K/V pairs.
  تعداد { WidthFactor = 8 };

  /// Values - This tracks the SourceDelta's currently in this node.
  SourceDelta اقدار[2 * WidthFactor - 1];

  /// NumValuesUsed - This tracks the number of values this node currently
  /// holds.
  unsigned حرف عدد_اقدار_Used = 0;

  /// IsLeaf - This is true if this is a leaf of the btree.  If false, this is
  /// an interior node, and is actually an instance of DeltaTreeInteriorNode.
  منطقي IsLeaf;

  /// FullDelta - This is the full delta of all the values in this node and
  /// all children nodes.
  صحيح FullDelta = 0;

عام:
  Delta_درخت_گرہ(منطقي isLeaf = true) : IsLeaf(isLeaf) {}

  منطقي isLeaf() ثابت { أرجع IsLeaf; }
  صحيح getFullDelta() ثابت { أرجع FullDelta; }
  منطقي isFull() ثابت { أرجع عدد_اقدار_Used == 2 * WidthFactor - 1; }

  unsigned get_عدد_اقدار_Used() ثابت { أرجع عدد_اقدار_Used; }

  ثابت SourceDelta &get_قدر(unsigned i) ثابت {
    أكد(i < عدد_اقدار_Used && "Invalid value #");
    أرجع اقدار[i];
  }

  SourceDelta &get_قدر(unsigned i) {
    أكد(i < عدد_اقدار_Used && "Invalid value #");
    أرجع اقدار[i];
  }

  /// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
  /// this node.  If insertion is easy, do it and return false.  Otherwise,
  /// split the node, populate InsertRes with info about the split, and return
  /// true.
  منطقي DoInsertion(unsigned FileIndex, صحيح Delta, داخل_کرو_Result *داخل_کرو_Res);

  فراغ DoSplit(داخل_کرو_Result &داخل_کرو_Res);

  /// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
  /// local walk over our contained deltas.
  فراغ RecomputeFullDeltaLocally();

  فراغ Destroy();
};

/// DeltaTreeInteriorNode - When isLeaf = false, a node has child pointers.
/// This class tracks them.
صنف Delta_درخت_Interior_گرہ : عام Delta_درخت_گرہ {
  friend صنف Delta_درخت_گرہ;

  Delta_درخت_گرہ *Children[2 * WidthFactor];

  ~Delta_درخت_Interior_گرہ() {
    لكل (unsigned i = 0, e = عدد_اقدار_Used + 1; i != e; ++i)
      Children[i]->Destroy();
  }

عام:
  Delta_درخت_Interior_گرہ() : Delta_درخت_گرہ(خطأ /*nonleaf*/) {}

  Delta_درخت_Interior_گرہ(ثابت داخل_کرو_Result &IR)
      : Delta_درخت_گرہ(خطأ /*nonleaf*/) {
    Children[0] = IR.LHS;
    Children[1] = IR.RHS;
    اقدار[0] = IR.Split;
    FullDelta =
        IR.LHS->getFullDelta() + IR.RHS->getFullDelta() + IR.Split.Delta;
    عدد_اقدار_Used = 1;
  }

  ثابت Delta_درخت_گرہ *getChild(unsigned i) ثابت {
    أكد(i < get_عدد_اقدار_Used() + 1 && "Invalid child");
    أرجع Children[i];
  }

  Delta_درخت_گرہ *getChild(unsigned i) {
    أكد(i < get_عدد_اقدار_Used() + 1 && "Invalid child");
    أرجع Children[i];
  }

  ساكن منطقي classof(ثابت Delta_درخت_گرہ *N) { أرجع !N->isLeaf(); }
};

} // namespace

/// Destroy - A 'virtual' destructor.
فراغ Delta_درخت_گرہ::Destroy() {
  إذا (isLeaf())
    delete هذا;
  وإلا
    delete cast<Delta_درخت_Interior_گرہ>(هذا);
}

/// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
/// local walk over our contained deltas.
فراغ Delta_درخت_گرہ::RecomputeFullDeltaLocally() {
  صحيح NewFullDelta = 0;
  لكل (unsigned i = 0, e = get_عدد_اقدار_Used(); i != e; ++i)
    NewFullDelta += اقدار[i].Delta;
  إذا (auto *IN = dyn_cast<Delta_درخت_Interior_گرہ>(هذا))
    لكل (unsigned i = 0, e = get_عدد_اقدار_Used() + 1; i != e; ++i)
      NewFullDelta += IN->getChild(i)->getFullDelta();
  FullDelta = NewFullDelta;
}

/// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
/// this node.  If insertion is easy, do it and return false.  Otherwise,
/// split the node, populate InsertRes with info about the split, and return
/// true.
منطقي Delta_درخت_گرہ::DoInsertion(unsigned FileIndex, صحيح Delta,
                                داخل_کرو_Result *داخل_کرو_Res) {
  // Maintain full delta for this node.
  FullDelta += Delta;

  // Find the insertion point, the first delta whose index is >= FileIndex.
  unsigned i = 0, e = get_عدد_اقدار_Used();
  طالما (i != e && FileIndex > get_قدر(i).FileLoc)
    ++i;

  // If we found an a record for exactly this file index, just merge this
  // value into the pre-existing record and finish early.
  إذا (i != e && get_قدر(i).FileLoc == FileIndex) {
    // NOTE: Delta could drop to zero here.  This means that the delta entry is
    // useless and could be removed.  Supporting erases is more complex than
    // leaving an entry with Delta=0, so we just leave an entry with Delta=0 in
    // the tree.
    اقدار[i].Delta += Delta;
    أرجع خطأ;
  }

  // Otherwise, we found an insertion point, and we know that the value at the
  // specified index is > FileIndex.  Handle the leaf case first.
  إذا (isLeaf()) {
    إذا (!isFull()) {
      // For an insertion into a non-full leaf node, just insert the value in
      // its sorted position.  This requires moving later values over.
      إذا (i != e)
        memmove(&اقدار[i + 1], &اقدار[i], حجم(اقدار[0]) * (e - i));
      اقدار[i] = SourceDelta::get(FileIndex, Delta);
      ++عدد_اقدار_Used;
      أرجع خطأ;
    }

    // Otherwise, if this is leaf is full, split the node at its median, insert
    // the value into one of the children, and return the result.
    أكد(داخل_کرو_Res && "No result location specified");
    DoSplit(*داخل_کرو_Res);

    إذا (داخل_کرو_Res->Split.FileLoc > FileIndex)
      داخل_کرو_Res->LHS->DoInsertion(FileIndex, Delta, nullptr /*can't fail*/);
    وإلا
      داخل_کرو_Res->RHS->DoInsertion(FileIndex, Delta, nullptr /*can't fail*/);
    أرجع true;
  }

  // Otherwise, this is an interior node.  Send the request down the tree.
  auto *IN = cast<Delta_درخت_Interior_گرہ>(هذا);
  إذا (!IN->Children[i]->DoInsertion(FileIndex, Delta, داخل_کرو_Res))
    أرجع خطأ; // If there was space in the child, just return.

  // Okay, this split the subtree, producing a new value and two children to
  // insert here.  If this node is non-full, we can just insert it directly.
  إذا (!isFull()) {
    // Now that we have two nodes and a new element, insert the perclated value
    // into ourself by moving all the later values/children down, then inserting
    // the new one.
    إذا (i != e)
      memmove(&IN->Children[i + 2], &IN->Children[i + 1],
              (e - i) * حجم(IN->Children[0]));
    IN->Children[i] = داخل_کرو_Res->LHS;
    IN->Children[i + 1] = داخل_کرو_Res->RHS;

    إذا (e != i)
      memmove(&اقدار[i + 1], &اقدار[i], (e - i) * حجم(اقدار[0]));
    اقدار[i] = داخل_کرو_Res->Split;
    ++عدد_اقدار_Used;
    أرجع خطأ;
  }

  // Finally, if this interior node was full and a node is percolated up, split
  // ourself and return that up the chain.  Start by saving all our info to
  // avoid having the split clobber it.
  IN->Children[i] = داخل_کرو_Res->LHS;
  Delta_درخت_گرہ *SubRHS = داخل_کرو_Res->RHS;
  SourceDelta SubSplit = داخل_کرو_Res->Split;

  // Do the split.
  DoSplit(*داخل_کرو_Res);

  // Figure out where to insert SubRHS/NewSplit.
  Delta_درخت_Interior_گرہ *داخل_کرو_Side;
  إذا (SubSplit.FileLoc < داخل_کرو_Res->Split.FileLoc)
    داخل_کرو_Side = cast<Delta_درخت_Interior_گرہ>(داخل_کرو_Res->LHS);
  وإلا
    داخل_کرو_Side = cast<Delta_درخت_Interior_گرہ>(داخل_کرو_Res->RHS);

  // We now have a non-empty interior node 'InsertSide' to insert
  // SubRHS/SubSplit into.  Find out where to insert SubSplit.

  // Find the insertion point, the first delta whose index is >SubSplit.FileLoc.
  i = 0;
  e = داخل_کرو_Side->get_عدد_اقدار_Used();
  طالما (i != e && SubSplit.FileLoc > داخل_کرو_Side->get_قدر(i).FileLoc)
    ++i;

  // Now we know that i is the place to insert the split value into.  Insert it
  // and the child right after it.
  إذا (i != e)
    memmove(&داخل_کرو_Side->Children[i + 2], &داخل_کرو_Side->Children[i + 1],
            (e - i) * حجم(IN->Children[0]));
  داخل_کرو_Side->Children[i + 1] = SubRHS;

  إذا (e != i)
    memmove(&داخل_کرو_Side->اقدار[i + 1], &داخل_کرو_Side->اقدار[i],
            (e - i) * حجم(اقدار[0]));
  داخل_کرو_Side->اقدار[i] = SubSplit;
  ++داخل_کرو_Side->عدد_اقدار_Used;
  داخل_کرو_Side->FullDelta += SubSplit.Delta + SubRHS->getFullDelta();
  أرجع true;
}

/// DoSplit - Split the currently full node (which has 2*WidthFactor-1 values)
/// into two subtrees each with "WidthFactor-1" values and a pivot value.
/// Return the pieces in InsertRes.
فراغ Delta_درخت_گرہ::DoSplit(داخل_کرو_Result &داخل_کرو_Res) {
  أكد(isFull() && "Why split a non-full node?");

  // Since this node is full, it contains 2*WidthFactor-1 values.  We move
  // the first 'WidthFactor-1' values to the LHS child (which we leave in this
  // node), propagate one value up, and move the last 'WidthFactor-1' values
  // into the RHS child.

  // Create the new child node.
  Delta_درخت_گرہ *New_گرہ;
  إذا (auto *IN = dyn_cast<Delta_درخت_Interior_گرہ>(هذا)) {
    // If this is an interior node, also move over 'WidthFactor' children
    // into the new node.
    Delta_درخت_Interior_گرہ *New = جديد Delta_درخت_Interior_گرہ();
    memcpy(&New->Children[0], &IN->Children[WidthFactor],
           WidthFactor * حجم(IN->Children[0]));
    New_گرہ = New;
  } وإلا {
    // Just create the new leaf node.
    New_گرہ = جديد Delta_درخت_گرہ();
  }

  // Move over the last 'WidthFactor-1' values from here to NewNode.
  memcpy(&New_گرہ->اقدار[0], &اقدار[WidthFactor],
         (WidthFactor - 1) * حجم(اقدار[0]));

  // Decrease the number of values in the two nodes.
  New_گرہ->عدد_اقدار_Used = عدد_اقدار_Used = WidthFactor - 1;

  // Recompute the two nodes' full delta.
  New_گرہ->RecomputeFullDeltaLocally();
  RecomputeFullDeltaLocally();

  داخل_کرو_Res.LHS = هذا;
  داخل_کرو_Res.RHS = New_گرہ;
  داخل_کرو_Res.Split = اقدار[WidthFactor - 1];
}

//===----------------------------------------------------------------------===//
//                        DeltaTree Implementation
//===----------------------------------------------------------------------===//

// #define VERIFY_TREE

#ifdef VERIFY_درخت
/// VerifyTree - Walk the btree performing assertions on various properties to
/// verify consistency.  This is useful for debugging new changes to the tree.
ساكن فراغ Verify_درخت(ثابت Delta_درخت_گرہ *N) {
  ثابت auto *IN = dyn_cast<Delta_درخت_Interior_گرہ>(N);
  إذا (IN == 0) {
    // Verify leaves, just ensure that FullDelta matches up and the elements
    // are in proper order.
    صحيح FullDelta = 0;
    لكل (unsigned i = 0, e = N->get_عدد_اقدار_Used(); i != e; ++i) {
      إذا (i)
        أكد(N->get_قدر(i - 1).FileLoc < N->get_قدر(i).FileLoc);
      FullDelta += N->get_قدر(i).Delta;
    }
    أكد(FullDelta == N->getFullDelta());
    أرجع;
  }

  // Verify interior nodes: Ensure that FullDelta matches up and the
  // elements are in proper order and the children are in proper order.
  صحيح FullDelta = 0;
  لكل (unsigned i = 0, e = IN->get_عدد_اقدار_Used(); i != e; ++i) {
    ثابت SourceDelta &IVal = N->get_قدر(i);
    ثابت Delta_درخت_گرہ *IChild = IN->getChild(i);
    إذا (i)
      أكد(IN->get_قدر(i - 1).FileLoc < IVal.FileLoc);
    FullDelta += IVal.Delta;
    FullDelta += IChild->getFullDelta();

    // The largest value in child #i should be smaller than FileLoc.
    أكد(IChild->get_قدر(IChild->get_عدد_اقدار_Used() - 1).FileLoc <
           IVal.FileLoc);

    // The smallest value in child #i+1 should be larger than FileLoc.
    أكد(IN->getChild(i + 1)->get_قدر(0).FileLoc > IVal.FileLoc);
    Verify_درخت(IChild);
  }

  FullDelta += IN->getChild(IN->get_عدد_اقدار_Used())->getFullDelta();

  أكد(FullDelta == N->getFullDelta());
}
#endif // VERIFY_TREE

ساكن Delta_درخت_گرہ *get_جڑ(فراغ *جڑ_2) { أرجع (Delta_درخت_گرہ *)جڑ_2; }

Delta_درخت::Delta_درخت() { جڑ_2 = جديد Delta_درخت_گرہ(); }

Delta_درخت::Delta_درخت(ثابت Delta_درخت &RHS) {
  // Currently we only support copying when the RHS is empty.
  أكد(get_جڑ(RHS.جڑ_2)->get_عدد_اقدار_Used() == 0 &&
         "Can only copy empty tree");
  جڑ_2 = جديد Delta_درخت_گرہ();
}

Delta_درخت::~Delta_درخت() { get_جڑ(جڑ_2)->Destroy(); }

/// getDeltaAt - Return the accumulated delta at the specified file offset.
/// This includes all insertions or delections that occurred *before* the
/// specified file index.
صحيح Delta_درخت::getDeltaAt(unsigned FileIndex) ثابت {
  ثابت Delta_درخت_گرہ *گرہ = get_جڑ(جڑ_2);

  صحيح Result = 0;

  // Walk down the tree.
  طالما (true) {
    // For all nodes, include any local deltas before the specified file
    // index by summing them up directly.  Keep track of how many were
    // included.
    unsigned عدد_Vals_Greater = 0;
    لكل (unsigned e = گرہ->get_عدد_اقدار_Used(); عدد_Vals_Greater != e;
         ++عدد_Vals_Greater) {
      ثابت SourceDelta &Val = گرہ->get_قدر(عدد_Vals_Greater);

      إذا (Val.FileLoc >= FileIndex)
        اكسر;
      Result += Val.Delta;
    }

    // If we have an interior node, include information about children and
    // recurse.  Otherwise, if we have a leaf, we're done.
    ثابت auto *IN = dyn_cast<Delta_درخت_Interior_گرہ>(گرہ);
    إذا (!IN)
      أرجع Result;

    // Include any children to the left of the values we skipped, all of
    // their deltas should be included as well.
    لكل (unsigned i = 0; i != عدد_Vals_Greater; ++i)
      Result += IN->getChild(i)->getFullDelta();

    // If we found exactly the value we were looking for, break off the
    // search early.  There is no need to search the RHS of the value for
    // partial results.
    إذا (عدد_Vals_Greater != گرہ->get_عدد_اقدار_Used() &&
        گرہ->get_قدر(عدد_Vals_Greater).FileLoc == FileIndex)
      أرجع Result + IN->getChild(عدد_Vals_Greater)->getFullDelta();

    // Otherwise, traverse down the tree.  The selected subtree may be
    // partially included in the range.
    گرہ = IN->getChild(عدد_Vals_Greater);
  }
  // NOT REACHED.
}

/// AddDelta - When a change is made that shifts around the text buffer,
/// this method is used to record that info.  It inserts a delta of 'Delta'
/// into the current DeltaTree at offset FileIndex.
فراغ Delta_درخت::AddDelta(unsigned FileIndex, صحيح Delta) {
  أكد(Delta && "Adding a noop?");
  Delta_درخت_گرہ *My_جڑ = get_جڑ(جڑ_2);

  Delta_درخت_گرہ::داخل_کرو_Result داخل_کرو_Res;
  إذا (My_جڑ->DoInsertion(FileIndex, Delta, &داخل_کرو_Res)) {
    جڑ_2 = جديد Delta_درخت_Interior_گرہ(داخل_کرو_Res);
#ifdef VERIFY_درخت
    My_جڑ = جڑ_2;
#endif
  }

#ifdef VERIFY_درخت
  Verify_درخت(My_جڑ);
#endif
}
