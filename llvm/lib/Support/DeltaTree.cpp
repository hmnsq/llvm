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

#include "llvm/ADT/DeltaTree.h"
#include "llvm/Support/Casting.h"
#include <cassert>
#include <cstring>

using namespace llvm;

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

namespace {

/// SourceDelta - As code in the original input buffer is added and deleted,
/// SourceDelta records are used to keep track of how the input SourceLocation
/// object is mapped into the output buffer.
struct SourceDelta {
  unsigned FileLoc;
  int Delta;

  static SourceDelta get(unsigned Loc, int D) {
    SourceDelta Delta;
    Delta.FileLoc = Loc;
    Delta.Delta = D;
    return Delta;
  }
};

/// DeltaTreeNode - The common part of all nodes.
///
class Delta_पुनो_बुहोल {
public:
  struct इसिंगित_Result {
    Delta_पुनो_बुहोल *LHS, *RHS;
    SourceDelta Split;
  };

private:
  friend class Delta_पुनो_Interior_बुहोल;

  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// WidthFactor-1 K/V pairs (except the root) and may have at most
  /// 2*WidthFactor-1 K/V pairs.
  enum { WidthFactor = 8 };

  /// Values - This tracks the SourceDelta's currently in this node.
  SourceDelta मगा_हालागा[2 * WidthFactor - 1];

  /// NumValuesUsed - This tracks the number of values this node currently
  /// holds.
  unsigned char नुमेरो_मगा_हालागा_Used = 0;

  /// IsLeaf - This is true if this is a leaf of the btree.  If false, this is
  /// an interior node, and is actually an instance of DeltaTreeInteriorNode.
  bool IsLeaf;

  /// FullDelta - This is the full delta of all the values in this node and
  /// all children nodes.
  int FullDelta = 0;

public:
  Delta_पुनो_बुहोल(bool isLeaf = true) : IsLeaf(isLeaf) {}

  bool isLeaf() const { return IsLeaf; }
  int getFullDelta() const { return FullDelta; }
  bool isFull() const { return नुमेरो_मगा_हालागा_Used == 2 * WidthFactor - 1; }

  unsigned get_नुमेरो_मगा_हालागा_Used() const { return नुमेरो_मगा_हालागा_Used; }

  const SourceDelta &get_हालागा(unsigned i) const {
    assert(i < नुमेरो_मगा_हालागा_Used && "Invalid value #");
    return मगा_हालागा[i];
  }

  SourceDelta &get_हालागा(unsigned i) {
    assert(i < नुमेरो_मगा_हालागा_Used && "Invalid value #");
    return मगा_हालागा[i];
  }

  /// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
  /// this node.  If insertion is easy, do it and return false.  Otherwise,
  /// split the node, populate InsertRes with info about the split, and return
  /// true.
  bool DoInsertion(unsigned FileIndex, int Delta, इसिंगित_Result *इसिंगित_Res);

  void DoSplit(इसिंगित_Result &इसिंगित_Res);

  /// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
  /// local walk over our contained deltas.
  void RecomputeFullDeltaLocally();

  void Destroy();
};

/// DeltaTreeInteriorNode - When isLeaf = false, a node has child pointers.
/// This class tracks them.
class Delta_पुनो_Interior_बुहोल : public Delta_पुनो_बुहोल {
  friend class Delta_पुनो_बुहोल;

  Delta_पुनो_बुहोल *Children[2 * WidthFactor];

  ~Delta_पुनो_Interior_बुहोल() {
    for (unsigned i = 0, e = नुमेरो_मगा_हालागा_Used + 1; i != e; ++i)
      Children[i]->Destroy();
  }

public:
  Delta_पुनो_Interior_बुहोल() : Delta_पुनो_बुहोल(false /*nonleaf*/) {}

  Delta_पुनो_Interior_बुहोल(const इसिंगित_Result &IR)
      : Delta_पुनो_बुहोल(false /*nonleaf*/) {
    Children[0] = IR.LHS;
    Children[1] = IR.RHS;
    मगा_हालागा[0] = IR.Split;
    FullDelta =
        IR.LHS->getFullDelta() + IR.RHS->getFullDelta() + IR.Split.Delta;
    नुमेरो_मगा_हालागा_Used = 1;
  }

  const Delta_पुनो_बुहोल *getChild(unsigned i) const {
    assert(i < get_नुमेरो_मगा_हालागा_Used() + 1 && "Invalid child");
    return Children[i];
  }

  Delta_पुनो_बुहोल *getChild(unsigned i) {
    assert(i < get_नुमेरो_मगा_हालागा_Used() + 1 && "Invalid child");
    return Children[i];
  }

  static bool classof(const Delta_पुनो_बुहोल *N) { return !N->isLeaf(); }
};

} // namespace

/// Destroy - A 'virtual' destructor.
void Delta_पुनो_बुहोल::Destroy() {
  if (isLeaf())
    delete this;
  else
    delete cast<Delta_पुनो_Interior_बुहोल>(this);
}

/// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
/// local walk over our contained deltas.
void Delta_पुनो_बुहोल::RecomputeFullDeltaLocally() {
  int NewFullDelta = 0;
  for (unsigned i = 0, e = get_नुमेरो_मगा_हालागा_Used(); i != e; ++i)
    NewFullDelta += मगा_हालागा[i].Delta;
  if (auto *IN = dyn_cast<Delta_पुनो_Interior_बुहोल>(this))
    for (unsigned i = 0, e = get_नुमेरो_मगा_हालागा_Used() + 1; i != e; ++i)
      NewFullDelta += IN->getChild(i)->getFullDelta();
  FullDelta = NewFullDelta;
}

/// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
/// this node.  If insertion is easy, do it and return false.  Otherwise,
/// split the node, populate InsertRes with info about the split, and return
/// true.
bool Delta_पुनो_बुहोल::DoInsertion(unsigned FileIndex, int Delta,
                                इसिंगित_Result *इसिंगित_Res) {
  // Maintain full delta for this node.
  FullDelta += Delta;

  // Find the insertion point, the first delta whose index is >= FileIndex.
  unsigned i = 0, e = get_नुमेरो_मगा_हालागा_Used();
  while (i != e && FileIndex > get_हालागा(i).FileLoc)
    ++i;

  // If we found an a record for exactly this file index, just merge this
  // value into the pre-existing record and finish early.
  if (i != e && get_हालागा(i).FileLoc == FileIndex) {
    // NOTE: Delta could drop to zero here.  This means that the delta entry is
    // useless and could be removed.  Supporting erases is more complex than
    // leaving an entry with Delta=0, so we just leave an entry with Delta=0 in
    // the tree.
    मगा_हालागा[i].Delta += Delta;
    return false;
  }

  // Otherwise, we found an insertion point, and we know that the value at the
  // specified index is > FileIndex.  Handle the leaf case first.
  if (isLeaf()) {
    if (!isFull()) {
      // For an insertion into a non-full leaf node, just insert the value in
      // its sorted position.  This requires moving later values over.
      if (i != e)
        memmove(&मगा_हालागा[i + 1], &मगा_हालागा[i], sizeof(मगा_हालागा[0]) * (e - i));
      मगा_हालागा[i] = SourceDelta::get(FileIndex, Delta);
      ++नुमेरो_मगा_हालागा_Used;
      return false;
    }

    // Otherwise, if this is leaf is full, split the node at its median, insert
    // the value into one of the children, and return the result.
    assert(इसिंगित_Res && "No result location specified");
    DoSplit(*इसिंगित_Res);

    if (इसिंगित_Res->Split.FileLoc > FileIndex)
      इसिंगित_Res->LHS->DoInsertion(FileIndex, Delta, nullptr /*can't fail*/);
    else
      इसिंगित_Res->RHS->DoInsertion(FileIndex, Delta, nullptr /*can't fail*/);
    return true;
  }

  // Otherwise, this is an interior node.  Send the request down the tree.
  auto *IN = cast<Delta_पुनो_Interior_बुहोल>(this);
  if (!IN->Children[i]->DoInsertion(FileIndex, Delta, इसिंगित_Res))
    return false; // If there was space in the child, just return.

  // Okay, this split the subtree, producing a new value and two children to
  // insert here.  If this node is non-full, we can just insert it directly.
  if (!isFull()) {
    // Now that we have two nodes and a new element, insert the perclated value
    // into ourself by moving all the later values/children down, then inserting
    // the new one.
    if (i != e)
      memmove(&IN->Children[i + 2], &IN->Children[i + 1],
              (e - i) * sizeof(IN->Children[0]));
    IN->Children[i] = इसिंगित_Res->LHS;
    IN->Children[i + 1] = इसिंगित_Res->RHS;

    if (e != i)
      memmove(&मगा_हालागा[i + 1], &मगा_हालागा[i], (e - i) * sizeof(मगा_हालागा[0]));
    मगा_हालागा[i] = इसिंगित_Res->Split;
    ++नुमेरो_मगा_हालागा_Used;
    return false;
  }

  // Finally, if this interior node was full and a node is percolated up, split
  // ourself and return that up the chain.  Start by saving all our info to
  // avoid having the split clobber it.
  IN->Children[i] = इसिंगित_Res->LHS;
  Delta_पुनो_बुहोल *SubRHS = इसिंगित_Res->RHS;
  SourceDelta SubSplit = इसिंगित_Res->Split;

  // Do the split.
  DoSplit(*इसिंगित_Res);

  // Figure out where to insert SubRHS/NewSplit.
  Delta_पुनो_Interior_बुहोल *इसिंगित_Side;
  if (SubSplit.FileLoc < इसिंगित_Res->Split.FileLoc)
    इसिंगित_Side = cast<Delta_पुनो_Interior_बुहोल>(इसिंगित_Res->LHS);
  else
    इसिंगित_Side = cast<Delta_पुनो_Interior_बुहोल>(इसिंगित_Res->RHS);

  // We now have a non-empty interior node 'InsertSide' to insert
  // SubRHS/SubSplit into.  Find out where to insert SubSplit.

  // Find the insertion point, the first delta whose index is >SubSplit.FileLoc.
  i = 0;
  e = इसिंगित_Side->get_नुमेरो_मगा_हालागा_Used();
  while (i != e && SubSplit.FileLoc > इसिंगित_Side->get_हालागा(i).FileLoc)
    ++i;

  // Now we know that i is the place to insert the split value into.  Insert it
  // and the child right after it.
  if (i != e)
    memmove(&इसिंगित_Side->Children[i + 2], &इसिंगित_Side->Children[i + 1],
            (e - i) * sizeof(IN->Children[0]));
  इसिंगित_Side->Children[i + 1] = SubRHS;

  if (e != i)
    memmove(&इसिंगित_Side->मगा_हालागा[i + 1], &इसिंगित_Side->मगा_हालागा[i],
            (e - i) * sizeof(मगा_हालागा[0]));
  इसिंगित_Side->मगा_हालागा[i] = SubSplit;
  ++इसिंगित_Side->नुमेरो_मगा_हालागा_Used;
  इसिंगित_Side->FullDelta += SubSplit.Delta + SubRHS->getFullDelta();
  return true;
}

/// DoSplit - Split the currently full node (which has 2*WidthFactor-1 values)
/// into two subtrees each with "WidthFactor-1" values and a pivot value.
/// Return the pieces in InsertRes.
void Delta_पुनो_बुहोल::DoSplit(इसिंगित_Result &इसिंगित_Res) {
  assert(isFull() && "Why split a non-full node?");

  // Since this node is full, it contains 2*WidthFactor-1 values.  We move
  // the first 'WidthFactor-1' values to the LHS child (which we leave in this
  // node), propagate one value up, and move the last 'WidthFactor-1' values
  // into the RHS child.

  // Create the new child node.
  Delta_पुनो_बुहोल *New_बुहोल;
  if (auto *IN = dyn_cast<Delta_पुनो_Interior_बुहोल>(this)) {
    // If this is an interior node, also move over 'WidthFactor' children
    // into the new node.
    Delta_पुनो_Interior_बुहोल *New = new Delta_पुनो_Interior_बुहोल();
    memcpy(&New->Children[0], &IN->Children[WidthFactor],
           WidthFactor * sizeof(IN->Children[0]));
    New_बुहोल = New;
  } else {
    // Just create the new leaf node.
    New_बुहोल = new Delta_पुनो_बुहोल();
  }

  // Move over the last 'WidthFactor-1' values from here to NewNode.
  memcpy(&New_बुहोल->मगा_हालागा[0], &मगा_हालागा[WidthFactor],
         (WidthFactor - 1) * sizeof(मगा_हालागा[0]));

  // Decrease the number of values in the two nodes.
  New_बुहोल->नुमेरो_मगा_हालागा_Used = नुमेरो_मगा_हालागा_Used = WidthFactor - 1;

  // Recompute the two nodes' full delta.
  New_बुहोल->RecomputeFullDeltaLocally();
  RecomputeFullDeltaLocally();

  इसिंगित_Res.LHS = this;
  इसिंगित_Res.RHS = New_बुहोल;
  इसिंगित_Res.Split = मगा_हालागा[WidthFactor - 1];
}

//===----------------------------------------------------------------------===//
//                        DeltaTree Implementation
//===----------------------------------------------------------------------===//

// #define VERIFY_TREE

#ifdef VERIFY_पुनो
/// VerifyTree - Walk the btree performing assertions on various properties to
/// verify consistency.  This is useful for debugging new changes to the tree.
static void Verify_पुनो(const Delta_पुनो_बुहोल *N) {
  const auto *IN = dyn_cast<Delta_पुनो_Interior_बुहोल>(N);
  if (IN == 0) {
    // Verify leaves, just ensure that FullDelta matches up and the elements
    // are in proper order.
    int FullDelta = 0;
    for (unsigned i = 0, e = N->get_नुमेरो_मगा_हालागा_Used(); i != e; ++i) {
      if (i)
        assert(N->get_हालागा(i - 1).FileLoc < N->get_हालागा(i).FileLoc);
      FullDelta += N->get_हालागा(i).Delta;
    }
    assert(FullDelta == N->getFullDelta());
    return;
  }

  // Verify interior nodes: Ensure that FullDelta matches up and the
  // elements are in proper order and the children are in proper order.
  int FullDelta = 0;
  for (unsigned i = 0, e = IN->get_नुमेरो_मगा_हालागा_Used(); i != e; ++i) {
    const SourceDelta &IVal = N->get_हालागा(i);
    const Delta_पुनो_बुहोल *IChild = IN->getChild(i);
    if (i)
      assert(IN->get_हालागा(i - 1).FileLoc < IVal.FileLoc);
    FullDelta += IVal.Delta;
    FullDelta += IChild->getFullDelta();

    // The largest value in child #i should be smaller than FileLoc.
    assert(IChild->get_हालागा(IChild->get_नुमेरो_मगा_हालागा_Used() - 1).FileLoc <
           IVal.FileLoc);

    // The smallest value in child #i+1 should be larger than FileLoc.
    assert(IN->getChild(i + 1)->get_हालागा(0).FileLoc > IVal.FileLoc);
    Verify_पुनो(IChild);
  }

  FullDelta += IN->getChild(IN->get_नुमेरो_मगा_हालागा_Used())->getFullDelta();

  assert(FullDelta == N->getFullDelta());
}
#endif // VERIFY_TREE

static Delta_पुनो_बुहोल *get_उगात(void *उगात_2) { return (Delta_पुनो_बुहोल *)उगात_2; }

Delta_पुनो::Delta_पुनो() { उगात_2 = new Delta_पुनो_बुहोल(); }

Delta_पुनो::Delta_पुनो(const Delta_पुनो &RHS) {
  // Currently we only support copying when the RHS is empty.
  assert(get_उगात(RHS.उगात_2)->get_नुमेरो_मगा_हालागा_Used() == 0 &&
         "Can only copy empty tree");
  उगात_2 = new Delta_पुनो_बुहोल();
}

Delta_पुनो::~Delta_पुनो() { get_उगात(उगात_2)->Destroy(); }

/// getDeltaAt - Return the accumulated delta at the specified file offset.
/// This includes all insertions or delections that occurred *before* the
/// specified file index.
int Delta_पुनो::getDeltaAt(unsigned FileIndex) const {
  const Delta_पुनो_बुहोल *बुहोल = get_उगात(उगात_2);

  int Result = 0;

  // Walk down the tree.
  while (true) {
    // For all nodes, include any local deltas before the specified file
    // index by summing them up directly.  Keep track of how many were
    // included.
    unsigned नुमेरो_Vals_Greater = 0;
    for (unsigned e = बुहोल->get_नुमेरो_मगा_हालागा_Used(); नुमेरो_Vals_Greater != e;
         ++नुमेरो_Vals_Greater) {
      const SourceDelta &Val = बुहोल->get_हालागा(नुमेरो_Vals_Greater);

      if (Val.FileLoc >= FileIndex)
        break;
      Result += Val.Delta;
    }

    // If we have an interior node, include information about children and
    // recurse.  Otherwise, if we have a leaf, we're done.
    const auto *IN = dyn_cast<Delta_पुनो_Interior_बुहोल>(बुहोल);
    if (!IN)
      return Result;

    // Include any children to the left of the values we skipped, all of
    // their deltas should be included as well.
    for (unsigned i = 0; i != नुमेरो_Vals_Greater; ++i)
      Result += IN->getChild(i)->getFullDelta();

    // If we found exactly the value we were looking for, break off the
    // search early.  There is no need to search the RHS of the value for
    // partial results.
    if (नुमेरो_Vals_Greater != बुहोल->get_नुमेरो_मगा_हालागा_Used() &&
        बुहोल->get_हालागा(नुमेरो_Vals_Greater).FileLoc == FileIndex)
      return Result + IN->getChild(नुमेरो_Vals_Greater)->getFullDelta();

    // Otherwise, traverse down the tree.  The selected subtree may be
    // partially included in the range.
    बुहोल = IN->getChild(नुमेरो_Vals_Greater);
  }
  // NOT REACHED.
}

/// AddDelta - When a change is made that shifts around the text buffer,
/// this method is used to record that info.  It inserts a delta of 'Delta'
/// into the current DeltaTree at offset FileIndex.
void Delta_पुनो::AddDelta(unsigned FileIndex, int Delta) {
  assert(Delta && "Adding a noop?");
  Delta_पुनो_बुहोल *My_उगात = get_उगात(उगात_2);

  Delta_पुनो_बुहोल::इसिंगित_Result इसिंगित_Res;
  if (My_उगात->DoInsertion(FileIndex, Delta, &इसिंगित_Res)) {
    उगात_2 = new Delta_पुनो_Interior_बुहोल(इसिंगित_Res);
#ifdef VERIFY_पुनो
    My_उगात = उगात_2;
#endif
  }

#ifdef VERIFY_पुनो
  Verify_पुनो(My_उगात);
#endif
}
