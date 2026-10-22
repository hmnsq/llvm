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

#അന്തര്ഭാവ "llvm/ADT/DeltaTree.h"
#അന്തര്ഭാവ "llvm/Support/Casting.h"
#അന്തര്ഭാവ <cassert>
#അന്തര്ഭാവ <cstring>

using നാമാകാശ llvm;

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

നാമാകാശ {

/// SourceDelta - As code in the original input buffer is added and deleted,
/// SourceDelta records are used to keep track of how the input SourceLocation
/// object is mapped into the output buffer.
ഘടന SourceDelta {
  unsigned FileLoc;
  പൂർണ്ണസംഖ്യ Delta;

  സ്ഥിതം SourceDelta get(unsigned Loc, പൂർണ്ണസംഖ്യ D) {
    SourceDelta Delta;
    Delta.FileLoc = Loc;
    Delta.Delta = D;
    തിരികെ Delta;
  }
};

/// DeltaTreeNode - The common part of all nodes.
///
വർഗ്ഗം Delta_വൃക്ഷം_കണ്ണി {
പൊതു:
  ഘടന തിരുകുക_Result {
    Delta_വൃക്ഷം_കണ്ണി *LHS, *RHS;
    SourceDelta Split;
  };

സ്വകാര്യ:
  friend വർഗ്ഗം Delta_വൃക്ഷം_Interior_കണ്ണി;

  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// WidthFactor-1 K/V pairs (except the root) and may have at most
  /// 2*WidthFactor-1 K/V pairs.
  എണ്ണൽ { WidthFactor = 8 };

  /// Values - This tracks the SourceDelta's currently in this node.
  SourceDelta മൂല്യങ്ങൾ[2 * WidthFactor - 1];

  /// NumValuesUsed - This tracks the number of values this node currently
  /// holds.
  unsigned അക്ഷരം അക്കം_മൂല്യങ്ങൾ_Used = 0;

  /// IsLeaf - This is true if this is a leaf of the btree.  If false, this is
  /// an interior node, and is actually an instance of DeltaTreeInteriorNode.
  തർക്കം IsLeaf;

  /// FullDelta - This is the full delta of all the values in this node and
  /// all children nodes.
  പൂർണ്ണസംഖ്യ FullDelta = 0;

പൊതു:
  Delta_വൃക്ഷം_കണ്ണി(തർക്കം isLeaf = സത്യം) : IsLeaf(isLeaf) {}

  തർക്കം isLeaf() സ്ഥിരം { തിരികെ IsLeaf; }
  പൂർണ്ണസംഖ്യ getFullDelta() സ്ഥിരം { തിരികെ FullDelta; }
  തർക്കം isFull() സ്ഥിരം { തിരികെ അക്കം_മൂല്യങ്ങൾ_Used == 2 * WidthFactor - 1; }

  unsigned get_അക്കം_മൂല്യങ്ങൾ_Used() സ്ഥിരം { തിരികെ അക്കം_മൂല്യങ്ങൾ_Used; }

  സ്ഥിരം SourceDelta &get_മൂല്യം(unsigned i) സ്ഥിരം {
    പ്രതിജ്ഞാ(i < അക്കം_മൂല്യങ്ങൾ_Used && "Invalid value #");
    തിരികെ മൂല്യങ്ങൾ[i];
  }

  SourceDelta &get_മൂല്യം(unsigned i) {
    പ്രതിജ്ഞാ(i < അക്കം_മൂല്യങ്ങൾ_Used && "Invalid value #");
    തിരികെ മൂല്യങ്ങൾ[i];
  }

  /// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
  /// this node.  If insertion is easy, do it and return false.  Otherwise,
  /// split the node, populate InsertRes with info about the split, and return
  /// true.
  തർക്കം DoInsertion(unsigned FileIndex, പൂർണ്ണസംഖ്യ Delta, തിരുകുക_Result *തിരുകുക_Res);

  ശൂന്യം DoSplit(തിരുകുക_Result &തിരുകുക_Res);

  /// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
  /// local walk over our contained deltas.
  ശൂന്യം RecomputeFullDeltaLocally();

  ശൂന്യം Destroy();
};

/// DeltaTreeInteriorNode - When isLeaf = false, a node has child pointers.
/// This class tracks them.
വർഗ്ഗം Delta_വൃക്ഷം_Interior_കണ്ണി : പൊതു Delta_വൃക്ഷം_കണ്ണി {
  friend വർഗ്ഗം Delta_വൃക്ഷം_കണ്ണി;

  Delta_വൃക്ഷം_കണ്ണി *Children[2 * WidthFactor];

  ~Delta_വൃക്ഷം_Interior_കണ്ണി() {
    വേണ്ടി (unsigned i = 0, e = അക്കം_മൂല്യങ്ങൾ_Used + 1; i != e; ++i)
      Children[i]->Destroy();
  }

പൊതു:
  Delta_വൃക്ഷം_Interior_കണ്ണി() : Delta_വൃക്ഷം_കണ്ണി(അസത്യം /*nonleaf*/) {}

  Delta_വൃക്ഷം_Interior_കണ്ണി(സ്ഥിരം തിരുകുക_Result &IR)
      : Delta_വൃക്ഷം_കണ്ണി(അസത്യം /*nonleaf*/) {
    Children[0] = IR.LHS;
    Children[1] = IR.RHS;
    മൂല്യങ്ങൾ[0] = IR.Split;
    FullDelta =
        IR.LHS->getFullDelta() + IR.RHS->getFullDelta() + IR.Split.Delta;
    അക്കം_മൂല്യങ്ങൾ_Used = 1;
  }

  സ്ഥിരം Delta_വൃക്ഷം_കണ്ണി *getChild(unsigned i) സ്ഥിരം {
    പ്രതിജ്ഞാ(i < get_അക്കം_മൂല്യങ്ങൾ_Used() + 1 && "Invalid child");
    തിരികെ Children[i];
  }

  Delta_വൃക്ഷം_കണ്ണി *getChild(unsigned i) {
    പ്രതിജ്ഞാ(i < get_അക്കം_മൂല്യങ്ങൾ_Used() + 1 && "Invalid child");
    തിരികെ Children[i];
  }

  സ്ഥിതം തർക്കം classof(സ്ഥിരം Delta_വൃക്ഷം_കണ്ണി *N) { തിരികെ !N->isLeaf(); }
};

} // namespace

/// Destroy - A 'virtual' destructor.
ശൂന്യം Delta_വൃക്ഷം_കണ്ണി::Destroy() {
  എങ്കിൽ (isLeaf())
    delete ഇത്;
  അല്ലെങ്കിൽ
    delete cast<Delta_വൃക്ഷം_Interior_കണ്ണി>(ഇത്);
}

/// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
/// local walk over our contained deltas.
ശൂന്യം Delta_വൃക്ഷം_കണ്ണി::RecomputeFullDeltaLocally() {
  പൂർണ്ണസംഖ്യ NewFullDelta = 0;
  വേണ്ടി (unsigned i = 0, e = get_അക്കം_മൂല്യങ്ങൾ_Used(); i != e; ++i)
    NewFullDelta += മൂല്യങ്ങൾ[i].Delta;
  എങ്കിൽ (auto *IN = dyn_cast<Delta_വൃക്ഷം_Interior_കണ്ണി>(ഇത്))
    വേണ്ടി (unsigned i = 0, e = get_അക്കം_മൂല്യങ്ങൾ_Used() + 1; i != e; ++i)
      NewFullDelta += IN->getChild(i)->getFullDelta();
  FullDelta = NewFullDelta;
}

/// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
/// this node.  If insertion is easy, do it and return false.  Otherwise,
/// split the node, populate InsertRes with info about the split, and return
/// true.
തർക്കം Delta_വൃക്ഷം_കണ്ണി::DoInsertion(unsigned FileIndex, പൂർണ്ണസംഖ്യ Delta,
                                തിരുകുക_Result *തിരുകുക_Res) {
  // Maintain full delta for this node.
  FullDelta += Delta;

  // Find the insertion point, the first delta whose index is >= FileIndex.
  unsigned i = 0, e = get_അക്കം_മൂല്യങ്ങൾ_Used();
  ആകുമ്പോൾ (i != e && FileIndex > get_മൂല്യം(i).FileLoc)
    ++i;

  // If we found an a record for exactly this file index, just merge this
  // value into the pre-existing record and finish early.
  എങ്കിൽ (i != e && get_മൂല്യം(i).FileLoc == FileIndex) {
    // NOTE: Delta could drop to zero here.  This means that the delta entry is
    // useless and could be removed.  Supporting erases is more complex than
    // leaving an entry with Delta=0, so we just leave an entry with Delta=0 in
    // the tree.
    മൂല്യങ്ങൾ[i].Delta += Delta;
    തിരികെ അസത്യം;
  }

  // Otherwise, we found an insertion point, and we know that the value at the
  // specified index is > FileIndex.  Handle the leaf case first.
  എങ്കിൽ (isLeaf()) {
    എങ്കിൽ (!isFull()) {
      // For an insertion into a non-full leaf node, just insert the value in
      // its sorted position.  This requires moving later values over.
      എങ്കിൽ (i != e)
        memmove(&മൂല്യങ്ങൾ[i + 1], &മൂല്യങ്ങൾ[i], വലുപ്പം(മൂല്യങ്ങൾ[0]) * (e - i));
      മൂല്യങ്ങൾ[i] = SourceDelta::get(FileIndex, Delta);
      ++അക്കം_മൂല്യങ്ങൾ_Used;
      തിരികെ അസത്യം;
    }

    // Otherwise, if this is leaf is full, split the node at its median, insert
    // the value into one of the children, and return the result.
    പ്രതിജ്ഞാ(തിരുകുക_Res && "No result location specified");
    DoSplit(*തിരുകുക_Res);

    എങ്കിൽ (തിരുകുക_Res->Split.FileLoc > FileIndex)
      തിരുകുക_Res->LHS->DoInsertion(FileIndex, Delta, nullptr /*can't fail*/);
    അല്ലെങ്കിൽ
      തിരുകുക_Res->RHS->DoInsertion(FileIndex, Delta, nullptr /*can't fail*/);
    തിരികെ സത്യം;
  }

  // Otherwise, this is an interior node.  Send the request down the tree.
  auto *IN = cast<Delta_വൃക്ഷം_Interior_കണ്ണി>(ഇത്);
  എങ്കിൽ (!IN->Children[i]->DoInsertion(FileIndex, Delta, തിരുകുക_Res))
    തിരികെ അസത്യം; // If there was space in the child, just return.

  // Okay, this split the subtree, producing a new value and two children to
  // insert here.  If this node is non-full, we can just insert it directly.
  എങ്കിൽ (!isFull()) {
    // Now that we have two nodes and a new element, insert the perclated value
    // into ourself by moving all the later values/children down, then inserting
    // the new one.
    എങ്കിൽ (i != e)
      memmove(&IN->Children[i + 2], &IN->Children[i + 1],
              (e - i) * വലുപ്പം(IN->Children[0]));
    IN->Children[i] = തിരുകുക_Res->LHS;
    IN->Children[i + 1] = തിരുകുക_Res->RHS;

    എങ്കിൽ (e != i)
      memmove(&മൂല്യങ്ങൾ[i + 1], &മൂല്യങ്ങൾ[i], (e - i) * വലുപ്പം(മൂല്യങ്ങൾ[0]));
    മൂല്യങ്ങൾ[i] = തിരുകുക_Res->Split;
    ++അക്കം_മൂല്യങ്ങൾ_Used;
    തിരികെ അസത്യം;
  }

  // Finally, if this interior node was full and a node is percolated up, split
  // ourself and return that up the chain.  Start by saving all our info to
  // avoid having the split clobber it.
  IN->Children[i] = തിരുകുക_Res->LHS;
  Delta_വൃക്ഷം_കണ്ണി *SubRHS = തിരുകുക_Res->RHS;
  SourceDelta SubSplit = തിരുകുക_Res->Split;

  // Do the split.
  DoSplit(*തിരുകുക_Res);

  // Figure out where to insert SubRHS/NewSplit.
  Delta_വൃക്ഷം_Interior_കണ്ണി *തിരുകുക_Side;
  എങ്കിൽ (SubSplit.FileLoc < തിരുകുക_Res->Split.FileLoc)
    തിരുകുക_Side = cast<Delta_വൃക്ഷം_Interior_കണ്ണി>(തിരുകുക_Res->LHS);
  അല്ലെങ്കിൽ
    തിരുകുക_Side = cast<Delta_വൃക്ഷം_Interior_കണ്ണി>(തിരുകുക_Res->RHS);

  // We now have a non-empty interior node 'InsertSide' to insert
  // SubRHS/SubSplit into.  Find out where to insert SubSplit.

  // Find the insertion point, the first delta whose index is >SubSplit.FileLoc.
  i = 0;
  e = തിരുകുക_Side->get_അക്കം_മൂല്യങ്ങൾ_Used();
  ആകുമ്പോൾ (i != e && SubSplit.FileLoc > തിരുകുക_Side->get_മൂല്യം(i).FileLoc)
    ++i;

  // Now we know that i is the place to insert the split value into.  Insert it
  // and the child right after it.
  എങ്കിൽ (i != e)
    memmove(&തിരുകുക_Side->Children[i + 2], &തിരുകുക_Side->Children[i + 1],
            (e - i) * വലുപ്പം(IN->Children[0]));
  തിരുകുക_Side->Children[i + 1] = SubRHS;

  എങ്കിൽ (e != i)
    memmove(&തിരുകുക_Side->മൂല്യങ്ങൾ[i + 1], &തിരുകുക_Side->മൂല്യങ്ങൾ[i],
            (e - i) * വലുപ്പം(മൂല്യങ്ങൾ[0]));
  തിരുകുക_Side->മൂല്യങ്ങൾ[i] = SubSplit;
  ++തിരുകുക_Side->അക്കം_മൂല്യങ്ങൾ_Used;
  തിരുകുക_Side->FullDelta += SubSplit.Delta + SubRHS->getFullDelta();
  തിരികെ സത്യം;
}

/// DoSplit - Split the currently full node (which has 2*WidthFactor-1 values)
/// into two subtrees each with "WidthFactor-1" values and a pivot value.
/// Return the pieces in InsertRes.
ശൂന്യം Delta_വൃക്ഷം_കണ്ണി::DoSplit(തിരുകുക_Result &തിരുകുക_Res) {
  പ്രതിജ്ഞാ(isFull() && "Why split a non-full node?");

  // Since this node is full, it contains 2*WidthFactor-1 values.  We move
  // the first 'WidthFactor-1' values to the LHS child (which we leave in this
  // node), propagate one value up, and move the last 'WidthFactor-1' values
  // into the RHS child.

  // Create the new child node.
  Delta_വൃക്ഷം_കണ്ണി *New_കണ്ണി;
  എങ്കിൽ (auto *IN = dyn_cast<Delta_വൃക്ഷം_Interior_കണ്ണി>(ഇത്)) {
    // If this is an interior node, also move over 'WidthFactor' children
    // into the new node.
    Delta_വൃക്ഷം_Interior_കണ്ണി *New = പുതിയ Delta_വൃക്ഷം_Interior_കണ്ണി();
    memcpy(&New->Children[0], &IN->Children[WidthFactor],
           WidthFactor * വലുപ്പം(IN->Children[0]));
    New_കണ്ണി = New;
  } അല്ലെങ്കിൽ {
    // Just create the new leaf node.
    New_കണ്ണി = പുതിയ Delta_വൃക്ഷം_കണ്ണി();
  }

  // Move over the last 'WidthFactor-1' values from here to NewNode.
  memcpy(&New_കണ്ണി->മൂല്യങ്ങൾ[0], &മൂല്യങ്ങൾ[WidthFactor],
         (WidthFactor - 1) * വലുപ്പം(മൂല്യങ്ങൾ[0]));

  // Decrease the number of values in the two nodes.
  New_കണ്ണി->അക്കം_മൂല്യങ്ങൾ_Used = അക്കം_മൂല്യങ്ങൾ_Used = WidthFactor - 1;

  // Recompute the two nodes' full delta.
  New_കണ്ണി->RecomputeFullDeltaLocally();
  RecomputeFullDeltaLocally();

  തിരുകുക_Res.LHS = ഇത്;
  തിരുകുക_Res.RHS = New_കണ്ണി;
  തിരുകുക_Res.Split = മൂല്യങ്ങൾ[WidthFactor - 1];
}

//===----------------------------------------------------------------------===//
//                        DeltaTree Implementation
//===----------------------------------------------------------------------===//

// #define VERIFY_TREE

#ifdef VERIFY_വൃക്ഷം
/// VerifyTree - Walk the btree performing assertions on various properties to
/// verify consistency.  This is useful for debugging new changes to the tree.
സ്ഥിതം ശൂന്യം Verify_വൃക്ഷം(സ്ഥിരം Delta_വൃക്ഷം_കണ്ണി *N) {
  സ്ഥിരം auto *IN = dyn_cast<Delta_വൃക്ഷം_Interior_കണ്ണി>(N);
  എങ്കിൽ (IN == 0) {
    // Verify leaves, just ensure that FullDelta matches up and the elements
    // are in proper order.
    പൂർണ്ണസംഖ്യ FullDelta = 0;
    വേണ്ടി (unsigned i = 0, e = N->get_അക്കം_മൂല്യങ്ങൾ_Used(); i != e; ++i) {
      എങ്കിൽ (i)
        പ്രതിജ്ഞാ(N->get_മൂല്യം(i - 1).FileLoc < N->get_മൂല്യം(i).FileLoc);
      FullDelta += N->get_മൂല്യം(i).Delta;
    }
    പ്രതിജ്ഞാ(FullDelta == N->getFullDelta());
    തിരികെ;
  }

  // Verify interior nodes: Ensure that FullDelta matches up and the
  // elements are in proper order and the children are in proper order.
  പൂർണ്ണസംഖ്യ FullDelta = 0;
  വേണ്ടി (unsigned i = 0, e = IN->get_അക്കം_മൂല്യങ്ങൾ_Used(); i != e; ++i) {
    സ്ഥിരം SourceDelta &IVal = N->get_മൂല്യം(i);
    സ്ഥിരം Delta_വൃക്ഷം_കണ്ണി *IChild = IN->getChild(i);
    എങ്കിൽ (i)
      പ്രതിജ്ഞാ(IN->get_മൂല്യം(i - 1).FileLoc < IVal.FileLoc);
    FullDelta += IVal.Delta;
    FullDelta += IChild->getFullDelta();

    // The largest value in child #i should be smaller than FileLoc.
    പ്രതിജ്ഞാ(IChild->get_മൂല്യം(IChild->get_അക്കം_മൂല്യങ്ങൾ_Used() - 1).FileLoc <
           IVal.FileLoc);

    // The smallest value in child #i+1 should be larger than FileLoc.
    പ്രതിജ്ഞാ(IN->getChild(i + 1)->get_മൂല്യം(0).FileLoc > IVal.FileLoc);
    Verify_വൃക്ഷം(IChild);
  }

  FullDelta += IN->getChild(IN->get_അക്കം_മൂല്യങ്ങൾ_Used())->getFullDelta();

  പ്രതിജ്ഞാ(FullDelta == N->getFullDelta());
}
#endif // VERIFY_TREE

സ്ഥിതം Delta_വൃക്ഷം_കണ്ണി *get_വേര്(ശൂന്യം *വേര്_2) { തിരികെ (Delta_വൃക്ഷം_കണ്ണി *)വേര്_2; }

Delta_വൃക്ഷം::Delta_വൃക്ഷം() { വേര്_2 = പുതിയ Delta_വൃക്ഷം_കണ്ണി(); }

Delta_വൃക്ഷം::Delta_വൃക്ഷം(സ്ഥിരം Delta_വൃക്ഷം &RHS) {
  // Currently we only support copying when the RHS is empty.
  പ്രതിജ്ഞാ(get_വേര്(RHS.വേര്_2)->get_അക്കം_മൂല്യങ്ങൾ_Used() == 0 &&
         "Can only copy empty tree");
  വേര്_2 = പുതിയ Delta_വൃക്ഷം_കണ്ണി();
}

Delta_വൃക്ഷം::~Delta_വൃക്ഷം() { get_വേര്(വേര്_2)->Destroy(); }

/// getDeltaAt - Return the accumulated delta at the specified file offset.
/// This includes all insertions or delections that occurred *before* the
/// specified file index.
പൂർണ്ണസംഖ്യ Delta_വൃക്ഷം::getDeltaAt(unsigned FileIndex) സ്ഥിരം {
  സ്ഥിരം Delta_വൃക്ഷം_കണ്ണി *കണ്ണി = get_വേര്(വേര്_2);

  പൂർണ്ണസംഖ്യ Result = 0;

  // Walk down the tree.
  ആകുമ്പോൾ (സത്യം) {
    // For all nodes, include any local deltas before the specified file
    // index by summing them up directly.  Keep track of how many were
    // included.
    unsigned അക്കം_Vals_Greater = 0;
    വേണ്ടി (unsigned e = കണ്ണി->get_അക്കം_മൂല്യങ്ങൾ_Used(); അക്കം_Vals_Greater != e;
         ++അക്കം_Vals_Greater) {
      സ്ഥിരം SourceDelta &Val = കണ്ണി->get_മൂല്യം(അക്കം_Vals_Greater);

      എങ്കിൽ (Val.FileLoc >= FileIndex)
        നിർത്തുക;
      Result += Val.Delta;
    }

    // If we have an interior node, include information about children and
    // recurse.  Otherwise, if we have a leaf, we're done.
    സ്ഥിരം auto *IN = dyn_cast<Delta_വൃക്ഷം_Interior_കണ്ണി>(കണ്ണി);
    എങ്കിൽ (!IN)
      തിരികെ Result;

    // Include any children to the left of the values we skipped, all of
    // their deltas should be included as well.
    വേണ്ടി (unsigned i = 0; i != അക്കം_Vals_Greater; ++i)
      Result += IN->getChild(i)->getFullDelta();

    // If we found exactly the value we were looking for, break off the
    // search early.  There is no need to search the RHS of the value for
    // partial results.
    എങ്കിൽ (അക്കം_Vals_Greater != കണ്ണി->get_അക്കം_മൂല്യങ്ങൾ_Used() &&
        കണ്ണി->get_മൂല്യം(അക്കം_Vals_Greater).FileLoc == FileIndex)
      തിരികെ Result + IN->getChild(അക്കം_Vals_Greater)->getFullDelta();

    // Otherwise, traverse down the tree.  The selected subtree may be
    // partially included in the range.
    കണ്ണി = IN->getChild(അക്കം_Vals_Greater);
  }
  // NOT REACHED.
}

/// AddDelta - When a change is made that shifts around the text buffer,
/// this method is used to record that info.  It inserts a delta of 'Delta'
/// into the current DeltaTree at offset FileIndex.
ശൂന്യം Delta_വൃക്ഷം::AddDelta(unsigned FileIndex, പൂർണ്ണസംഖ്യ Delta) {
  പ്രതിജ്ഞാ(Delta && "Adding a noop?");
  Delta_വൃക്ഷം_കണ്ണി *My_വേര് = get_വേര്(വേര്_2);

  Delta_വൃക്ഷം_കണ്ണി::തിരുകുക_Result തിരുകുക_Res;
  എങ്കിൽ (My_വേര്->DoInsertion(FileIndex, Delta, &തിരുകുക_Res)) {
    വേര്_2 = പുതിയ Delta_വൃക്ഷം_Interior_കണ്ണി(തിരുകുക_Res);
#ifdef VERIFY_വൃക്ഷം
    My_വേര് = വേര്_2;
#endif
  }

#ifdef VERIFY_വൃക്ഷം
  Verify_വൃക്ഷം(My_വേര്);
#endif
}
