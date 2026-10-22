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

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/DeltaTree.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Casting.h"
#ଅନ୍ତର୍ଭାଵ <cassert>
#ଅନ୍ତର୍ଭାଵ <cstring>

using ନାମାକାଶ llvm;

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

ନାମାକାଶ {

/// SourceDelta - As code in the original input buffer is added and deleted,
/// SourceDelta records are used to keep track of how the input SourceLocation
/// object is mapped into the output buffer.
ସଂରଚନା SourceDelta {
  unsigned FileLoc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Delta;

  ସ୍ଥିର SourceDelta get(unsigned Loc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା D) {
    SourceDelta Delta;
    Delta.FileLoc = Loc;
    Delta.Delta = D;
    ଫେରନ୍ତୁ Delta;
  }
};

/// DeltaTreeNode - The common part of all nodes.
///
ଶ୍ରେଣୀ Delta_ବୃକ୍ଷ_ଗଣ୍ଠି {
ସର୍ବସାଧାରଣ:
  ସଂରଚନା ଭର୍ତ୍ତି_Result {
    Delta_ବୃକ୍ଷ_ଗଣ୍ଠି *LHS, *RHS;
    SourceDelta Split;
  };

ବ୍ୟକ୍ତିଗତ:
  friend ଶ୍ରେଣୀ Delta_ବୃକ୍ଷ_Interior_ଗଣ୍ଠି;

  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// WidthFactor-1 K/V pairs (except the root) and may have at most
  /// 2*WidthFactor-1 K/V pairs.
  ଗଣନା { WidthFactor = 8 };

  /// Values - This tracks the SourceDelta's currently in this node.
  SourceDelta ମୂଲ୍ୟଗୁଡ଼ିକ[2 * WidthFactor - 1];

  /// NumValuesUsed - This tracks the number of values this node currently
  /// holds.
  unsigned ଅକ୍ଷର ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used = 0;

  /// IsLeaf - This is true if this is a leaf of the btree.  If false, this is
  /// an interior node, and is actually an instance of DeltaTreeInteriorNode.
  ତାର୍କିକ IsLeaf;

  /// FullDelta - This is the full delta of all the values in this node and
  /// all children nodes.
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା FullDelta = 0;

ସର୍ବସାଧାରଣ:
  Delta_ବୃକ୍ଷ_ଗଣ୍ଠି(ତାର୍କିକ isLeaf = ସତ୍ୟ) : IsLeaf(isLeaf) {}

  ତାର୍କିକ isLeaf() ଅଚଳ { ଫେରନ୍ତୁ IsLeaf; }
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା getFullDelta() ଅଚଳ { ଫେରନ୍ତୁ FullDelta; }
  ତାର୍କିକ isFull() ଅଚଳ { ଫେରନ୍ତୁ ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used == 2 * WidthFactor - 1; }

  unsigned get_ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used() ଅଚଳ { ଫେରନ୍ତୁ ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used; }

  ଅଚଳ SourceDelta &get_ମୂଲ୍ୟ(unsigned i) ଅଚଳ {
    ପ୍ରତିଜ୍ଞା(i < ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used && "Invalid value #");
    ଫେରନ୍ତୁ ମୂଲ୍ୟଗୁଡ଼ିକ[i];
  }

  SourceDelta &get_ମୂଲ୍ୟ(unsigned i) {
    ପ୍ରତିଜ୍ଞା(i < ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used && "Invalid value #");
    ଫେରନ୍ତୁ ମୂଲ୍ୟଗୁଡ଼ିକ[i];
  }

  /// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
  /// this node.  If insertion is easy, do it and return false.  Otherwise,
  /// split the node, populate InsertRes with info about the split, and return
  /// true.
  ତାର୍କିକ DoInsertion(unsigned FileIndex, ପୂର୍ଣ୍ଣସଂଖ୍ୟା Delta, ଭର୍ତ୍ତି_Result *ଭର୍ତ୍ତି_Res);

  ଶୂନ୍ୟ DoSplit(ଭର୍ତ୍ତି_Result &ଭର୍ତ୍ତି_Res);

  /// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
  /// local walk over our contained deltas.
  ଶୂନ୍ୟ RecomputeFullDeltaLocally();

  ଶୂନ୍ୟ Destroy();
};

/// DeltaTreeInteriorNode - When isLeaf = false, a node has child pointers.
/// This class tracks them.
ଶ୍ରେଣୀ Delta_ବୃକ୍ଷ_Interior_ଗଣ୍ଠି : ସର୍ବସାଧାରଣ Delta_ବୃକ୍ଷ_ଗଣ୍ଠି {
  friend ଶ୍ରେଣୀ Delta_ବୃକ୍ଷ_ଗଣ୍ଠି;

  Delta_ବୃକ୍ଷ_ଗଣ୍ଠି *Children[2 * WidthFactor];

  ~Delta_ବୃକ୍ଷ_Interior_ଗଣ୍ଠି() {
    ପାଇଁ (unsigned i = 0, e = ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used + 1; i != e; ++i)
      Children[i]->Destroy();
  }

ସର୍ବସାଧାରଣ:
  Delta_ବୃକ୍ଷ_Interior_ଗଣ୍ଠି() : Delta_ବୃକ୍ଷ_ଗଣ୍ଠି(ମିଥ୍ୟା /*nonleaf*/) {}

  Delta_ବୃକ୍ଷ_Interior_ଗଣ୍ଠି(ଅଚଳ ଭର୍ତ୍ତି_Result &IR)
      : Delta_ବୃକ୍ଷ_ଗଣ୍ଠି(ମିଥ୍ୟା /*nonleaf*/) {
    Children[0] = IR.LHS;
    Children[1] = IR.RHS;
    ମୂଲ୍ୟଗୁଡ଼ିକ[0] = IR.Split;
    FullDelta =
        IR.LHS->getFullDelta() + IR.RHS->getFullDelta() + IR.Split.Delta;
    ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used = 1;
  }

  ଅଚଳ Delta_ବୃକ୍ଷ_ଗଣ୍ଠି *getChild(unsigned i) ଅଚଳ {
    ପ୍ରତିଜ୍ଞା(i < get_ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used() + 1 && "Invalid child");
    ଫେରନ୍ତୁ Children[i];
  }

  Delta_ବୃକ୍ଷ_ଗଣ୍ଠି *getChild(unsigned i) {
    ପ୍ରତିଜ୍ଞା(i < get_ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used() + 1 && "Invalid child");
    ଫେରନ୍ତୁ Children[i];
  }

  ସ୍ଥିର ତାର୍କିକ classof(ଅଚଳ Delta_ବୃକ୍ଷ_ଗଣ୍ଠି *N) { ଫେରନ୍ତୁ !N->isLeaf(); }
};

} // namespace

/// Destroy - A 'virtual' destructor.
ଶୂନ୍ୟ Delta_ବୃକ୍ଷ_ଗଣ୍ଠି::Destroy() {
  ଯଦି (isLeaf())
    delete ଏହି;
  ଅନ୍ୟଥା
    delete cast<Delta_ବୃକ୍ଷ_Interior_ଗଣ୍ଠି>(ଏହି);
}

/// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
/// local walk over our contained deltas.
ଶୂନ୍ୟ Delta_ବୃକ୍ଷ_ଗଣ୍ଠି::RecomputeFullDeltaLocally() {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା NewFullDelta = 0;
  ପାଇଁ (unsigned i = 0, e = get_ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used(); i != e; ++i)
    NewFullDelta += ମୂଲ୍ୟଗୁଡ଼ିକ[i].Delta;
  ଯଦି (auto *IN = dyn_cast<Delta_ବୃକ୍ଷ_Interior_ଗଣ୍ଠି>(ଏହି))
    ପାଇଁ (unsigned i = 0, e = get_ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used() + 1; i != e; ++i)
      NewFullDelta += IN->getChild(i)->getFullDelta();
  FullDelta = NewFullDelta;
}

/// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
/// this node.  If insertion is easy, do it and return false.  Otherwise,
/// split the node, populate InsertRes with info about the split, and return
/// true.
ତାର୍କିକ Delta_ବୃକ୍ଷ_ଗଣ୍ଠି::DoInsertion(unsigned FileIndex, ପୂର୍ଣ୍ଣସଂଖ୍ୟା Delta,
                                ଭର୍ତ୍ତି_Result *ଭର୍ତ୍ତି_Res) {
  // Maintain full delta for this node.
  FullDelta += Delta;

  // Find the insertion point, the first delta whose index is >= FileIndex.
  unsigned i = 0, e = get_ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used();
  ଯେତେବେଳେ (i != e && FileIndex > get_ମୂଲ୍ୟ(i).FileLoc)
    ++i;

  // If we found an a record for exactly this file index, just merge this
  // value into the pre-existing record and finish early.
  ଯଦି (i != e && get_ମୂଲ୍ୟ(i).FileLoc == FileIndex) {
    // NOTE: Delta could drop to zero here.  This means that the delta entry is
    // useless and could be removed.  Supporting erases is more complex than
    // leaving an entry with Delta=0, so we just leave an entry with Delta=0 in
    // the tree.
    ମୂଲ୍ୟଗୁଡ଼ିକ[i].Delta += Delta;
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  // Otherwise, we found an insertion point, and we know that the value at the
  // specified index is > FileIndex.  Handle the leaf case first.
  ଯଦି (isLeaf()) {
    ଯଦି (!isFull()) {
      // For an insertion into a non-full leaf node, just insert the value in
      // its sorted position.  This requires moving later values over.
      ଯଦି (i != e)
        memmove(&ମୂଲ୍ୟଗୁଡ଼ିକ[i + 1], &ମୂଲ୍ୟଗୁଡ଼ିକ[i], ଆକାର(ମୂଲ୍ୟଗୁଡ଼ିକ[0]) * (e - i));
      ମୂଲ୍ୟଗୁଡ଼ିକ[i] = SourceDelta::get(FileIndex, Delta);
      ++ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used;
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }

    // Otherwise, if this is leaf is full, split the node at its median, insert
    // the value into one of the children, and return the result.
    ପ୍ରତିଜ୍ଞା(ଭର୍ତ୍ତି_Res && "No result location specified");
    DoSplit(*ଭର୍ତ୍ତି_Res);

    ଯଦି (ଭର୍ତ୍ତି_Res->Split.FileLoc > FileIndex)
      ଭର୍ତ୍ତି_Res->LHS->DoInsertion(FileIndex, Delta, nullptr /*can't fail*/);
    ଅନ୍ୟଥା
      ଭର୍ତ୍ତି_Res->RHS->DoInsertion(FileIndex, Delta, nullptr /*can't fail*/);
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }

  // Otherwise, this is an interior node.  Send the request down the tree.
  auto *IN = cast<Delta_ବୃକ୍ଷ_Interior_ଗଣ୍ଠି>(ଏହି);
  ଯଦି (!IN->Children[i]->DoInsertion(FileIndex, Delta, ଭର୍ତ୍ତି_Res))
    ଫେରନ୍ତୁ ମିଥ୍ୟା; // If there was space in the child, just return.

  // Okay, this split the subtree, producing a new value and two children to
  // insert here.  If this node is non-full, we can just insert it directly.
  ଯଦି (!isFull()) {
    // Now that we have two nodes and a new element, insert the perclated value
    // into ourself by moving all the later values/children down, then inserting
    // the new one.
    ଯଦି (i != e)
      memmove(&IN->Children[i + 2], &IN->Children[i + 1],
              (e - i) * ଆକାର(IN->Children[0]));
    IN->Children[i] = ଭର୍ତ୍ତି_Res->LHS;
    IN->Children[i + 1] = ଭର୍ତ୍ତି_Res->RHS;

    ଯଦି (e != i)
      memmove(&ମୂଲ୍ୟଗୁଡ଼ିକ[i + 1], &ମୂଲ୍ୟଗୁଡ଼ିକ[i], (e - i) * ଆକାର(ମୂଲ୍ୟଗୁଡ଼ିକ[0]));
    ମୂଲ୍ୟଗୁଡ଼ିକ[i] = ଭର୍ତ୍ତି_Res->Split;
    ++ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used;
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  // Finally, if this interior node was full and a node is percolated up, split
  // ourself and return that up the chain.  Start by saving all our info to
  // avoid having the split clobber it.
  IN->Children[i] = ଭର୍ତ୍ତି_Res->LHS;
  Delta_ବୃକ୍ଷ_ଗଣ୍ଠି *SubRHS = ଭର୍ତ୍ତି_Res->RHS;
  SourceDelta SubSplit = ଭର୍ତ୍ତି_Res->Split;

  // Do the split.
  DoSplit(*ଭର୍ତ୍ତି_Res);

  // Figure out where to insert SubRHS/NewSplit.
  Delta_ବୃକ୍ଷ_Interior_ଗଣ୍ଠି *ଭର୍ତ୍ତି_Side;
  ଯଦି (SubSplit.FileLoc < ଭର୍ତ୍ତି_Res->Split.FileLoc)
    ଭର୍ତ୍ତି_Side = cast<Delta_ବୃକ୍ଷ_Interior_ଗଣ୍ଠି>(ଭର୍ତ୍ତି_Res->LHS);
  ଅନ୍ୟଥା
    ଭର୍ତ୍ତି_Side = cast<Delta_ବୃକ୍ଷ_Interior_ଗଣ୍ଠି>(ଭର୍ତ୍ତି_Res->RHS);

  // We now have a non-empty interior node 'InsertSide' to insert
  // SubRHS/SubSplit into.  Find out where to insert SubSplit.

  // Find the insertion point, the first delta whose index is >SubSplit.FileLoc.
  i = 0;
  e = ଭର୍ତ୍ତି_Side->get_ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used();
  ଯେତେବେଳେ (i != e && SubSplit.FileLoc > ଭର୍ତ୍ତି_Side->get_ମୂଲ୍ୟ(i).FileLoc)
    ++i;

  // Now we know that i is the place to insert the split value into.  Insert it
  // and the child right after it.
  ଯଦି (i != e)
    memmove(&ଭର୍ତ୍ତି_Side->Children[i + 2], &ଭର୍ତ୍ତି_Side->Children[i + 1],
            (e - i) * ଆକାର(IN->Children[0]));
  ଭର୍ତ୍ତି_Side->Children[i + 1] = SubRHS;

  ଯଦି (e != i)
    memmove(&ଭର୍ତ୍ତି_Side->ମୂଲ୍ୟଗୁଡ଼ିକ[i + 1], &ଭର୍ତ୍ତି_Side->ମୂଲ୍ୟଗୁଡ଼ିକ[i],
            (e - i) * ଆକାର(ମୂଲ୍ୟଗୁଡ଼ିକ[0]));
  ଭର୍ତ୍ତି_Side->ମୂଲ୍ୟଗୁଡ଼ିକ[i] = SubSplit;
  ++ଭର୍ତ୍ତି_Side->ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used;
  ଭର୍ତ୍ତି_Side->FullDelta += SubSplit.Delta + SubRHS->getFullDelta();
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

/// DoSplit - Split the currently full node (which has 2*WidthFactor-1 values)
/// into two subtrees each with "WidthFactor-1" values and a pivot value.
/// Return the pieces in InsertRes.
ଶୂନ୍ୟ Delta_ବୃକ୍ଷ_ଗଣ୍ଠି::DoSplit(ଭର୍ତ୍ତି_Result &ଭର୍ତ୍ତି_Res) {
  ପ୍ରତିଜ୍ଞା(isFull() && "Why split a non-full node?");

  // Since this node is full, it contains 2*WidthFactor-1 values.  We move
  // the first 'WidthFactor-1' values to the LHS child (which we leave in this
  // node), propagate one value up, and move the last 'WidthFactor-1' values
  // into the RHS child.

  // Create the new child node.
  Delta_ବୃକ୍ଷ_ଗଣ୍ଠି *New_ଗଣ୍ଠି;
  ଯଦି (auto *IN = dyn_cast<Delta_ବୃକ୍ଷ_Interior_ଗଣ୍ଠି>(ଏହି)) {
    // If this is an interior node, also move over 'WidthFactor' children
    // into the new node.
    Delta_ବୃକ୍ଷ_Interior_ଗଣ୍ଠି *New = ନୂତନ Delta_ବୃକ୍ଷ_Interior_ଗଣ୍ଠି();
    memcpy(&New->Children[0], &IN->Children[WidthFactor],
           WidthFactor * ଆକାର(IN->Children[0]));
    New_ଗଣ୍ଠି = New;
  } ଅନ୍ୟଥା {
    // Just create the new leaf node.
    New_ଗଣ୍ଠି = ନୂତନ Delta_ବୃକ୍ଷ_ଗଣ୍ଠି();
  }

  // Move over the last 'WidthFactor-1' values from here to NewNode.
  memcpy(&New_ଗଣ୍ଠି->ମୂଲ୍ୟଗୁଡ଼ିକ[0], &ମୂଲ୍ୟଗୁଡ଼ିକ[WidthFactor],
         (WidthFactor - 1) * ଆକାର(ମୂଲ୍ୟଗୁଡ଼ିକ[0]));

  // Decrease the number of values in the two nodes.
  New_ଗଣ୍ଠି->ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used = ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used = WidthFactor - 1;

  // Recompute the two nodes' full delta.
  New_ଗଣ୍ଠି->RecomputeFullDeltaLocally();
  RecomputeFullDeltaLocally();

  ଭର୍ତ୍ତି_Res.LHS = ଏହି;
  ଭର୍ତ୍ତି_Res.RHS = New_ଗଣ୍ଠି;
  ଭର୍ତ୍ତି_Res.Split = ମୂଲ୍ୟଗୁଡ଼ିକ[WidthFactor - 1];
}

//===----------------------------------------------------------------------===//
//                        DeltaTree Implementation
//===----------------------------------------------------------------------===//

// #define VERIFY_TREE

#ifdef VERIFY_ବୃକ୍ଷ
/// VerifyTree - Walk the btree performing assertions on various properties to
/// verify consistency.  This is useful for debugging new changes to the tree.
ସ୍ଥିର ଶୂନ୍ୟ Verify_ବୃକ୍ଷ(ଅଚଳ Delta_ବୃକ୍ଷ_ଗଣ୍ଠି *N) {
  ଅଚଳ auto *IN = dyn_cast<Delta_ବୃକ୍ଷ_Interior_ଗଣ୍ଠି>(N);
  ଯଦି (IN == 0) {
    // Verify leaves, just ensure that FullDelta matches up and the elements
    // are in proper order.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା FullDelta = 0;
    ପାଇଁ (unsigned i = 0, e = N->get_ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used(); i != e; ++i) {
      ଯଦି (i)
        ପ୍ରତିଜ୍ଞା(N->get_ମୂଲ୍ୟ(i - 1).FileLoc < N->get_ମୂଲ୍ୟ(i).FileLoc);
      FullDelta += N->get_ମୂଲ୍ୟ(i).Delta;
    }
    ପ୍ରତିଜ୍ଞା(FullDelta == N->getFullDelta());
    ଫେରନ୍ତୁ;
  }

  // Verify interior nodes: Ensure that FullDelta matches up and the
  // elements are in proper order and the children are in proper order.
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା FullDelta = 0;
  ପାଇଁ (unsigned i = 0, e = IN->get_ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used(); i != e; ++i) {
    ଅଚଳ SourceDelta &IVal = N->get_ମୂଲ୍ୟ(i);
    ଅଚଳ Delta_ବୃକ୍ଷ_ଗଣ୍ଠି *IChild = IN->getChild(i);
    ଯଦି (i)
      ପ୍ରତିଜ୍ଞା(IN->get_ମୂଲ୍ୟ(i - 1).FileLoc < IVal.FileLoc);
    FullDelta += IVal.Delta;
    FullDelta += IChild->getFullDelta();

    // The largest value in child #i should be smaller than FileLoc.
    ପ୍ରତିଜ୍ଞା(IChild->get_ମୂଲ୍ୟ(IChild->get_ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used() - 1).FileLoc <
           IVal.FileLoc);

    // The smallest value in child #i+1 should be larger than FileLoc.
    ପ୍ରତିଜ୍ଞା(IN->getChild(i + 1)->get_ମୂଲ୍ୟ(0).FileLoc > IVal.FileLoc);
    Verify_ବୃକ୍ଷ(IChild);
  }

  FullDelta += IN->getChild(IN->get_ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used())->getFullDelta();

  ପ୍ରତିଜ୍ଞା(FullDelta == N->getFullDelta());
}
#endif // VERIFY_TREE

ସ୍ଥିର Delta_ବୃକ୍ଷ_ଗଣ୍ଠି *get_ମୂଳ(ଶୂନ୍ୟ *ମୂଳ_2) { ଫେରନ୍ତୁ (Delta_ବୃକ୍ଷ_ଗଣ୍ଠି *)ମୂଳ_2; }

Delta_ବୃକ୍ଷ::Delta_ବୃକ୍ଷ() { ମୂଳ_2 = ନୂତନ Delta_ବୃକ୍ଷ_ଗଣ୍ଠି(); }

Delta_ବୃକ୍ଷ::Delta_ବୃକ୍ଷ(ଅଚଳ Delta_ବୃକ୍ଷ &RHS) {
  // Currently we only support copying when the RHS is empty.
  ପ୍ରତିଜ୍ଞା(get_ମୂଳ(RHS.ମୂଳ_2)->get_ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used() == 0 &&
         "Can only copy empty tree");
  ମୂଳ_2 = ନୂତନ Delta_ବୃକ୍ଷ_ଗଣ୍ଠି();
}

Delta_ବୃକ୍ଷ::~Delta_ବୃକ୍ଷ() { get_ମୂଳ(ମୂଳ_2)->Destroy(); }

/// getDeltaAt - Return the accumulated delta at the specified file offset.
/// This includes all insertions or delections that occurred *before* the
/// specified file index.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Delta_ବୃକ୍ଷ::getDeltaAt(unsigned FileIndex) ଅଚଳ {
  ଅଚଳ Delta_ବୃକ୍ଷ_ଗଣ୍ଠି *ଗଣ୍ଠି = get_ମୂଳ(ମୂଳ_2);

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Result = 0;

  // Walk down the tree.
  ଯେତେବେଳେ (ସତ୍ୟ) {
    // For all nodes, include any local deltas before the specified file
    // index by summing them up directly.  Keep track of how many were
    // included.
    unsigned ଅଙ୍କ_Vals_Greater = 0;
    ପାଇଁ (unsigned e = ଗଣ୍ଠି->get_ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used(); ଅଙ୍କ_Vals_Greater != e;
         ++ଅଙ୍କ_Vals_Greater) {
      ଅଚଳ SourceDelta &Val = ଗଣ୍ଠି->get_ମୂଲ୍ୟ(ଅଙ୍କ_Vals_Greater);

      ଯଦି (Val.FileLoc >= FileIndex)
        ଭାଙ୍ଗ;
      Result += Val.Delta;
    }

    // If we have an interior node, include information about children and
    // recurse.  Otherwise, if we have a leaf, we're done.
    ଅଚଳ auto *IN = dyn_cast<Delta_ବୃକ୍ଷ_Interior_ଗଣ୍ଠି>(ଗଣ୍ଠି);
    ଯଦି (!IN)
      ଫେରନ୍ତୁ Result;

    // Include any children to the left of the values we skipped, all of
    // their deltas should be included as well.
    ପାଇଁ (unsigned i = 0; i != ଅଙ୍କ_Vals_Greater; ++i)
      Result += IN->getChild(i)->getFullDelta();

    // If we found exactly the value we were looking for, break off the
    // search early.  There is no need to search the RHS of the value for
    // partial results.
    ଯଦି (ଅଙ୍କ_Vals_Greater != ଗଣ୍ଠି->get_ଅଙ୍କ_ମୂଲ୍ୟଗୁଡ଼ିକ_Used() &&
        ଗଣ୍ଠି->get_ମୂଲ୍ୟ(ଅଙ୍କ_Vals_Greater).FileLoc == FileIndex)
      ଫେରନ୍ତୁ Result + IN->getChild(ଅଙ୍କ_Vals_Greater)->getFullDelta();

    // Otherwise, traverse down the tree.  The selected subtree may be
    // partially included in the range.
    ଗଣ୍ଠି = IN->getChild(ଅଙ୍କ_Vals_Greater);
  }
  // NOT REACHED.
}

/// AddDelta - When a change is made that shifts around the text buffer,
/// this method is used to record that info.  It inserts a delta of 'Delta'
/// into the current DeltaTree at offset FileIndex.
ଶୂନ୍ୟ Delta_ବୃକ୍ଷ::AddDelta(unsigned FileIndex, ପୂର୍ଣ୍ଣସଂଖ୍ୟା Delta) {
  ପ୍ରତିଜ୍ଞା(Delta && "Adding a noop?");
  Delta_ବୃକ୍ଷ_ଗଣ୍ଠି *My_ମୂଳ = get_ମୂଳ(ମୂଳ_2);

  Delta_ବୃକ୍ଷ_ଗଣ୍ଠି::ଭର୍ତ୍ତି_Result ଭର୍ତ୍ତି_Res;
  ଯଦି (My_ମୂଳ->DoInsertion(FileIndex, Delta, &ଭର୍ତ୍ତି_Res)) {
    ମୂଳ_2 = ନୂତନ Delta_ବୃକ୍ଷ_Interior_ଗଣ୍ଠି(ଭର୍ତ୍ତି_Res);
#ifdef VERIFY_ବୃକ୍ଷ
    My_ମୂଳ = ମୂଳ_2;
#endif
  }

#ifdef VERIFY_ବୃକ୍ଷ
  Verify_ବୃକ୍ଷ(My_ମୂଳ);
#endif
}
