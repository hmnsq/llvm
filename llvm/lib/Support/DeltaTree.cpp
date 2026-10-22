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
    إرجاع Delta;
  }
};

/// DeltaTreeNode - The common part of all nodes.
///
صنف Delta_شجرة_عقدة {
عام:
  بنية أدرج_Result {
    Delta_شجرة_عقدة *LHS, *RHS;
    SourceDelta Split;
  };

خاص:
  friend صنف Delta_شجرة_Interior_عقدة;

  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// WidthFactor-1 K/V pairs (except the root) and may have at most
  /// 2*WidthFactor-1 K/V pairs.
  تعداد { WidthFactor = 8 };

  /// Values - This tracks the SourceDelta's currently in this node.
  SourceDelta قيم[2 * WidthFactor - 1];

  /// NumValuesUsed - This tracks the number of values this node currently
  /// holds.
  unsigned حرف رقم_قيم_Used = 0;

  /// IsLeaf - This is true if this is a leaf of the btree.  If false, this is
  /// an interior node, and is actually an instance of DeltaTreeInteriorNode.
  منطقي IsLeaf;

  /// FullDelta - This is the full delta of all the values in this node and
  /// all children nodes.
  صحيح FullDelta = 0;

عام:
  Delta_شجرة_عقدة(منطقي isLeaf = true) : IsLeaf(isLeaf) {}

  منطقي isLeaf() ثابت { إرجاع IsLeaf; }
  صحيح getFullDelta() ثابت { إرجاع FullDelta; }
  منطقي isFull() ثابت { إرجاع رقم_قيم_Used == 2 * WidthFactor - 1; }

  unsigned get_رقم_قيم_Used() ثابت { إرجاع رقم_قيم_Used; }

  ثابت SourceDelta &get_قيمة(unsigned i) ثابت {
    أكد(i < رقم_قيم_Used && "Invalid value #");
    إرجاع قيم[i];
  }

  SourceDelta &get_قيمة(unsigned i) {
    أكد(i < رقم_قيم_Used && "Invalid value #");
    إرجاع قيم[i];
  }

  /// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
  /// this node.  If insertion is easy, do it and return false.  Otherwise,
  /// split the node, populate InsertRes with info about the split, and return
  /// true.
  منطقي DoInsertion(unsigned FileIndex, صحيح Delta, أدرج_Result *أدرج_Res);

  فراغ DoSplit(أدرج_Result &أدرج_Res);

  /// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
  /// local walk over our contained deltas.
  فراغ RecomputeFullDeltaLocally();

  فراغ Destroy();
};

/// DeltaTreeInteriorNode - When isLeaf = false, a node has child pointers.
/// This class tracks them.
صنف Delta_شجرة_Interior_عقدة : عام Delta_شجرة_عقدة {
  friend صنف Delta_شجرة_عقدة;

  Delta_شجرة_عقدة *Children[2 * WidthFactor];

  ~Delta_شجرة_Interior_عقدة() {
    لكل (unsigned i = 0, e = رقم_قيم_Used + 1; i != e; ++i)
      Children[i]->Destroy();
  }

عام:
  Delta_شجرة_Interior_عقدة() : Delta_شجرة_عقدة(خطأ /*nonleaf*/) {}

  Delta_شجرة_Interior_عقدة(ثابت أدرج_Result &IR)
      : Delta_شجرة_عقدة(خطأ /*nonleaf*/) {
    Children[0] = IR.LHS;
    Children[1] = IR.RHS;
    قيم[0] = IR.Split;
    FullDelta =
        IR.LHS->getFullDelta() + IR.RHS->getFullDelta() + IR.Split.Delta;
    رقم_قيم_Used = 1;
  }

  ثابت Delta_شجرة_عقدة *getChild(unsigned i) ثابت {
    أكد(i < get_رقم_قيم_Used() + 1 && "Invalid child");
    إرجاع Children[i];
  }

  Delta_شجرة_عقدة *getChild(unsigned i) {
    أكد(i < get_رقم_قيم_Used() + 1 && "Invalid child");
    إرجاع Children[i];
  }

  ساكن منطقي classof(ثابت Delta_شجرة_عقدة *N) { إرجاع !N->isLeaf(); }
};

} // namespace

/// Destroy - A 'virtual' destructor.
فراغ Delta_شجرة_عقدة::Destroy() {
  إذا (isLeaf())
    delete هذا;
  وإلا
    delete cast<Delta_شجرة_Interior_عقدة>(هذا);
}

/// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
/// local walk over our contained deltas.
فراغ Delta_شجرة_عقدة::RecomputeFullDeltaLocally() {
  صحيح NewFullDelta = 0;
  لكل (unsigned i = 0, e = get_رقم_قيم_Used(); i != e; ++i)
    NewFullDelta += قيم[i].Delta;
  إذا (auto *IN = dyn_cast<Delta_شجرة_Interior_عقدة>(هذا))
    لكل (unsigned i = 0, e = get_رقم_قيم_Used() + 1; i != e; ++i)
      NewFullDelta += IN->getChild(i)->getFullDelta();
  FullDelta = NewFullDelta;
}

/// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
/// this node.  If insertion is easy, do it and return false.  Otherwise,
/// split the node, populate InsertRes with info about the split, and return
/// true.
منطقي Delta_شجرة_عقدة::DoInsertion(unsigned FileIndex, صحيح Delta,
                                أدرج_Result *أدرج_Res) {
  // Maintain full delta for this node.
  FullDelta += Delta;

  // Find the insertion point, the first delta whose index is >= FileIndex.
  unsigned i = 0, e = get_رقم_قيم_Used();
  بينما (i != e && FileIndex > get_قيمة(i).FileLoc)
    ++i;

  // If we found an a record for exactly this file index, just merge this
  // value into the pre-existing record and finish early.
  إذا (i != e && get_قيمة(i).FileLoc == FileIndex) {
    // NOTE: Delta could drop to zero here.  This means that the delta entry is
    // useless and could be removed.  Supporting erases is more complex than
    // leaving an entry with Delta=0, so we just leave an entry with Delta=0 in
    // the tree.
    قيم[i].Delta += Delta;
    إرجاع خطأ;
  }

  // Otherwise, we found an insertion point, and we know that the value at the
  // specified index is > FileIndex.  Handle the leaf case first.
  إذا (isLeaf()) {
    إذا (!isFull()) {
      // For an insertion into a non-full leaf node, just insert the value in
      // its sorted position.  This requires moving later values over.
      إذا (i != e)
        memmove(&قيم[i + 1], &قيم[i], حجم(قيم[0]) * (e - i));
      قيم[i] = SourceDelta::get(FileIndex, Delta);
      ++رقم_قيم_Used;
      إرجاع خطأ;
    }

    // Otherwise, if this is leaf is full, split the node at its median, insert
    // the value into one of the children, and return the result.
    أكد(أدرج_Res && "No result location specified");
    DoSplit(*أدرج_Res);

    إذا (أدرج_Res->Split.FileLoc > FileIndex)
      أدرج_Res->LHS->DoInsertion(FileIndex, Delta, nullptr /*can't fail*/);
    وإلا
      أدرج_Res->RHS->DoInsertion(FileIndex, Delta, nullptr /*can't fail*/);
    إرجاع true;
  }

  // Otherwise, this is an interior node.  Send the request down the tree.
  auto *IN = cast<Delta_شجرة_Interior_عقدة>(هذا);
  إذا (!IN->Children[i]->DoInsertion(FileIndex, Delta, أدرج_Res))
    إرجاع خطأ; // If there was space in the child, just return.

  // Okay, this split the subtree, producing a new value and two children to
  // insert here.  If this node is non-full, we can just insert it directly.
  إذا (!isFull()) {
    // Now that we have two nodes and a new element, insert the perclated value
    // into ourself by moving all the later values/children down, then inserting
    // the new one.
    إذا (i != e)
      memmove(&IN->Children[i + 2], &IN->Children[i + 1],
              (e - i) * حجم(IN->Children[0]));
    IN->Children[i] = أدرج_Res->LHS;
    IN->Children[i + 1] = أدرج_Res->RHS;

    إذا (e != i)
      memmove(&قيم[i + 1], &قيم[i], (e - i) * حجم(قيم[0]));
    قيم[i] = أدرج_Res->Split;
    ++رقم_قيم_Used;
    إرجاع خطأ;
  }

  // Finally, if this interior node was full and a node is percolated up, split
  // ourself and return that up the chain.  Start by saving all our info to
  // avoid having the split clobber it.
  IN->Children[i] = أدرج_Res->LHS;
  Delta_شجرة_عقدة *SubRHS = أدرج_Res->RHS;
  SourceDelta SubSplit = أدرج_Res->Split;

  // Do the split.
  DoSplit(*أدرج_Res);

  // Figure out where to insert SubRHS/NewSplit.
  Delta_شجرة_Interior_عقدة *أدرج_Side;
  إذا (SubSplit.FileLoc < أدرج_Res->Split.FileLoc)
    أدرج_Side = cast<Delta_شجرة_Interior_عقدة>(أدرج_Res->LHS);
  وإلا
    أدرج_Side = cast<Delta_شجرة_Interior_عقدة>(أدرج_Res->RHS);

  // We now have a non-empty interior node 'InsertSide' to insert
  // SubRHS/SubSplit into.  Find out where to insert SubSplit.

  // Find the insertion point, the first delta whose index is >SubSplit.FileLoc.
  i = 0;
  e = أدرج_Side->get_رقم_قيم_Used();
  بينما (i != e && SubSplit.FileLoc > أدرج_Side->get_قيمة(i).FileLoc)
    ++i;

  // Now we know that i is the place to insert the split value into.  Insert it
  // and the child right after it.
  إذا (i != e)
    memmove(&أدرج_Side->Children[i + 2], &أدرج_Side->Children[i + 1],
            (e - i) * حجم(IN->Children[0]));
  أدرج_Side->Children[i + 1] = SubRHS;

  إذا (e != i)
    memmove(&أدرج_Side->قيم[i + 1], &أدرج_Side->قيم[i],
            (e - i) * حجم(قيم[0]));
  أدرج_Side->قيم[i] = SubSplit;
  ++أدرج_Side->رقم_قيم_Used;
  أدرج_Side->FullDelta += SubSplit.Delta + SubRHS->getFullDelta();
  إرجاع true;
}

/// DoSplit - Split the currently full node (which has 2*WidthFactor-1 values)
/// into two subtrees each with "WidthFactor-1" values and a pivot value.
/// Return the pieces in InsertRes.
فراغ Delta_شجرة_عقدة::DoSplit(أدرج_Result &أدرج_Res) {
  أكد(isFull() && "Why split a non-full node?");

  // Since this node is full, it contains 2*WidthFactor-1 values.  We move
  // the first 'WidthFactor-1' values to the LHS child (which we leave in this
  // node), propagate one value up, and move the last 'WidthFactor-1' values
  // into the RHS child.

  // Create the new child node.
  Delta_شجرة_عقدة *New_عقدة;
  إذا (auto *IN = dyn_cast<Delta_شجرة_Interior_عقدة>(هذا)) {
    // If this is an interior node, also move over 'WidthFactor' children
    // into the new node.
    Delta_شجرة_Interior_عقدة *New = جديد Delta_شجرة_Interior_عقدة();
    memcpy(&New->Children[0], &IN->Children[WidthFactor],
           WidthFactor * حجم(IN->Children[0]));
    New_عقدة = New;
  } وإلا {
    // Just create the new leaf node.
    New_عقدة = جديد Delta_شجرة_عقدة();
  }

  // Move over the last 'WidthFactor-1' values from here to NewNode.
  memcpy(&New_عقدة->قيم[0], &قيم[WidthFactor],
         (WidthFactor - 1) * حجم(قيم[0]));

  // Decrease the number of values in the two nodes.
  New_عقدة->رقم_قيم_Used = رقم_قيم_Used = WidthFactor - 1;

  // Recompute the two nodes' full delta.
  New_عقدة->RecomputeFullDeltaLocally();
  RecomputeFullDeltaLocally();

  أدرج_Res.LHS = هذا;
  أدرج_Res.RHS = New_عقدة;
  أدرج_Res.Split = قيم[WidthFactor - 1];
}

//===----------------------------------------------------------------------===//
//                        DeltaTree Implementation
//===----------------------------------------------------------------------===//

// #define VERIFY_TREE

#ifdef VERIFY_شجرة
/// VerifyTree - Walk the btree performing assertions on various properties to
/// verify consistency.  This is useful for debugging new changes to the tree.
ساكن فراغ Verify_شجرة(ثابت Delta_شجرة_عقدة *N) {
  ثابت auto *IN = dyn_cast<Delta_شجرة_Interior_عقدة>(N);
  إذا (IN == 0) {
    // Verify leaves, just ensure that FullDelta matches up and the elements
    // are in proper order.
    صحيح FullDelta = 0;
    لكل (unsigned i = 0, e = N->get_رقم_قيم_Used(); i != e; ++i) {
      إذا (i)
        أكد(N->get_قيمة(i - 1).FileLoc < N->get_قيمة(i).FileLoc);
      FullDelta += N->get_قيمة(i).Delta;
    }
    أكد(FullDelta == N->getFullDelta());
    إرجاع;
  }

  // Verify interior nodes: Ensure that FullDelta matches up and the
  // elements are in proper order and the children are in proper order.
  صحيح FullDelta = 0;
  لكل (unsigned i = 0, e = IN->get_رقم_قيم_Used(); i != e; ++i) {
    ثابت SourceDelta &IVal = N->get_قيمة(i);
    ثابت Delta_شجرة_عقدة *IChild = IN->getChild(i);
    إذا (i)
      أكد(IN->get_قيمة(i - 1).FileLoc < IVal.FileLoc);
    FullDelta += IVal.Delta;
    FullDelta += IChild->getFullDelta();

    // The largest value in child #i should be smaller than FileLoc.
    أكد(IChild->get_قيمة(IChild->get_رقم_قيم_Used() - 1).FileLoc <
           IVal.FileLoc);

    // The smallest value in child #i+1 should be larger than FileLoc.
    أكد(IN->getChild(i + 1)->get_قيمة(0).FileLoc > IVal.FileLoc);
    Verify_شجرة(IChild);
  }

  FullDelta += IN->getChild(IN->get_رقم_قيم_Used())->getFullDelta();

  أكد(FullDelta == N->getFullDelta());
}
#endif // VERIFY_TREE

ساكن Delta_شجرة_عقدة *get_جذر(فراغ *جذر_2) { إرجاع (Delta_شجرة_عقدة *)جذر_2; }

Delta_شجرة::Delta_شجرة() { جذر_2 = جديد Delta_شجرة_عقدة(); }

Delta_شجرة::Delta_شجرة(ثابت Delta_شجرة &RHS) {
  // Currently we only support copying when the RHS is empty.
  أكد(get_جذر(RHS.جذر_2)->get_رقم_قيم_Used() == 0 &&
         "Can only copy empty tree");
  جذر_2 = جديد Delta_شجرة_عقدة();
}

Delta_شجرة::~Delta_شجرة() { get_جذر(جذر_2)->Destroy(); }

/// getDeltaAt - Return the accumulated delta at the specified file offset.
/// This includes all insertions or delections that occurred *before* the
/// specified file index.
صحيح Delta_شجرة::getDeltaAt(unsigned FileIndex) ثابت {
  ثابت Delta_شجرة_عقدة *عقدة = get_جذر(جذر_2);

  صحيح Result = 0;

  // Walk down the tree.
  بينما (true) {
    // For all nodes, include any local deltas before the specified file
    // index by summing them up directly.  Keep track of how many were
    // included.
    unsigned رقم_Vals_Greater = 0;
    لكل (unsigned e = عقدة->get_رقم_قيم_Used(); رقم_Vals_Greater != e;
         ++رقم_Vals_Greater) {
      ثابت SourceDelta &Val = عقدة->get_قيمة(رقم_Vals_Greater);

      إذا (Val.FileLoc >= FileIndex)
        اكسر;
      Result += Val.Delta;
    }

    // If we have an interior node, include information about children and
    // recurse.  Otherwise, if we have a leaf, we're done.
    ثابت auto *IN = dyn_cast<Delta_شجرة_Interior_عقدة>(عقدة);
    إذا (!IN)
      إرجاع Result;

    // Include any children to the left of the values we skipped, all of
    // their deltas should be included as well.
    لكل (unsigned i = 0; i != رقم_Vals_Greater; ++i)
      Result += IN->getChild(i)->getFullDelta();

    // If we found exactly the value we were looking for, break off the
    // search early.  There is no need to search the RHS of the value for
    // partial results.
    إذا (رقم_Vals_Greater != عقدة->get_رقم_قيم_Used() &&
        عقدة->get_قيمة(رقم_Vals_Greater).FileLoc == FileIndex)
      إرجاع Result + IN->getChild(رقم_Vals_Greater)->getFullDelta();

    // Otherwise, traverse down the tree.  The selected subtree may be
    // partially included in the range.
    عقدة = IN->getChild(رقم_Vals_Greater);
  }
  // NOT REACHED.
}

/// AddDelta - When a change is made that shifts around the text buffer,
/// this method is used to record that info.  It inserts a delta of 'Delta'
/// into the current DeltaTree at offset FileIndex.
فراغ Delta_شجرة::AddDelta(unsigned FileIndex, صحيح Delta) {
  أكد(Delta && "Adding a noop?");
  Delta_شجرة_عقدة *My_جذر = get_جذر(جذر_2);

  Delta_شجرة_عقدة::أدرج_Result أدرج_Res;
  إذا (My_جذر->DoInsertion(FileIndex, Delta, &أدرج_Res)) {
    جذر_2 = جديد Delta_شجرة_Interior_عقدة(أدرج_Res);
#ifdef VERIFY_شجرة
    My_جذر = جذر_2;
#endif
  }

#ifdef VERIFY_شجرة
  Verify_شجرة(My_جذر);
#endif
}
