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

#అన్తర్భావ "llvm/ADT/DeltaTree.h"
#అన్తర్భావ "llvm/Support/Casting.h"
#అన్తర్భావ <cassert>
#అన్తర్భావ <cstring>

ఉపయోగించి నామాకాశ llvm;

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

నామాకాశ {

/// SourceDelta - As code in the original input buffer is added and deleted,
/// SourceDelta records are used to keep track of how the input SourceLocation
/// object is mapped into the output buffer.
నిర్మాణం ఆధారం_Delta {
  గుర్తులేని FileLoc;
  పూర్ణాంకం Delta;

  స్థిర ఆధారం_Delta get(గుర్తులేని Loc, పూర్ణాంకం D) {
    ఆధారం_Delta Delta;
    Delta.FileLoc = Loc;
    Delta.Delta = D;
    తిరిగి Delta;
  }
};

/// DeltaTreeNode - The common part of all nodes.
///
తరగతి Delta_వృక్షం_కణుపు {
బహిరంగ:
  నిర్మాణం చొప్పించు_ఫలితం {
    Delta_వృక్షం_కణుపు *LHS, *RHS;
    ఆధారం_Delta విభజించు_2;
  };

అంతరంగ:
  మిత్ర తరగతి Delta_వృక్షం_Interior_కణుపు;

  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// WidthFactor-1 K/V pairs (except the root) and may have at most
  /// 2*WidthFactor-1 K/V pairs.
  గణన { వెడల్పు_గుణకం = 8 };

  /// Values - This tracks the SourceDelta's currently in this node.
  ఆధారం_Delta విలువలు[2 * వెడల్పు_గుణకం - 1];

  /// NumValuesUsed - This tracks the number of values this node currently
  /// holds.
  గుర్తులేని అక్షరం అంకె_విలువలు_వాడిన = 0;

  /// IsLeaf - This is true if this is a leaf of the btree.  If false, this is
  /// an interior node, and is actually an instance of DeltaTreeInteriorNode.
  తార్కిక Is_ఆకు;

  /// FullDelta - This is the full delta of all the values in this node and
  /// all children nodes.
  పూర్ణాంకం నిండిన_Delta = 0;

బహిరంగ:
  Delta_వృక్షం_కణుపు(తార్కిక is_ఆకు = నిజం) : Is_ఆకు(is_ఆకు) {}

  తార్కిక is_ఆకు() స్థిరరాశి { తిరిగి Is_ఆకు; }
  పూర్ణాంకం get_నిండిన_Delta() స్థిరరాశి { తిరిగి నిండిన_Delta; }
  తార్కిక is_నిండిన() స్థిరరాశి { తిరిగి అంకె_విలువలు_వాడిన == 2 * వెడల్పు_గుణకం - 1; }

  గుర్తులేని get_అంకె_విలువలు_వాడిన() స్థిరరాశి { తిరిగి అంకె_విలువలు_వాడిన; }

  స్థిరరాశి ఆధారం_Delta &get_విలువ(గుర్తులేని i) స్థిరరాశి {
    ప్రతిజ్ఞా(i < అంకె_విలువలు_వాడిన && "Invalid value #");
    తిరిగి విలువలు[i];
  }

  ఆధారం_Delta &get_విలువ(గుర్తులేని i) {
    ప్రతిజ్ఞా(i < అంకె_విలువలు_వాడిన && "Invalid value #");
    తిరిగి విలువలు[i];
  }

  /// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
  /// this node.  If insertion is easy, do it and return false.  Otherwise,
  /// split the node, populate InsertRes with info about the split, and return
  /// true.
  తార్కిక DoInsertion(గుర్తులేని File_సూచీ, పూర్ణాంకం Delta, చొప్పించు_ఫలితం *చొప్పించు_Res);

  శూన్య Do_విభజించు(చొప్పించు_ఫలితం &చొప్పించు_Res);

  /// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
  /// local walk over our contained deltas.
  శూన్య Recompute_నిండిన_Delta_Locally();

  శూన్య Destroy();
};

/// DeltaTreeInteriorNode - When isLeaf = false, a node has child pointers.
/// This class tracks them.
తరగతి Delta_వృక్షం_Interior_కణుపు : బహిరంగ Delta_వృక్షం_కణుపు {
  మిత్ర తరగతి Delta_వృక్షం_కణుపు;

  Delta_వృక్షం_కణుపు *Children[2 * వెడల్పు_గుణకం];

  ~Delta_వృక్షం_Interior_కణుపు() {
    కొరకు (గుర్తులేని i = 0, e = అంకె_విలువలు_వాడిన + 1; i != e; ++i)
      Children[i]->Destroy();
  }

బహిరంగ:
  Delta_వృక్షం_Interior_కణుపు() : Delta_వృక్షం_కణుపు(అబద్ధం /*nonleaf*/) {}

  Delta_వృక్షం_Interior_కణుపు(స్థిరరాశి చొప్పించు_ఫలితం &IR)
      : Delta_వృక్షం_కణుపు(అబద్ధం /*nonleaf*/) {
    Children[0] = IR.LHS;
    Children[1] = IR.RHS;
    విలువలు[0] = IR.విభజించు_2;
    నిండిన_Delta =
        IR.LHS->get_నిండిన_Delta() + IR.RHS->get_నిండిన_Delta() + IR.విభజించు_2.Delta;
    అంకె_విలువలు_వాడిన = 1;
  }

  స్థిరరాశి Delta_వృక్షం_కణుపు *get_బిడ్డ(గుర్తులేని i) స్థిరరాశి {
    ప్రతిజ్ఞా(i < get_అంకె_విలువలు_వాడిన() + 1 && "Invalid child");
    తిరిగి Children[i];
  }

  Delta_వృక్షం_కణుపు *get_బిడ్డ(గుర్తులేని i) {
    ప్రతిజ్ఞా(i < get_అంకె_విలువలు_వాడిన() + 1 && "Invalid child");
    తిరిగి Children[i];
  }

  స్థిర తార్కిక classof(స్థిరరాశి Delta_వృక్షం_కణుపు *N) { తిరిగి !N->is_ఆకు(); }
};

} // namespace

/// Destroy - A 'virtual' destructor.
శూన్య Delta_వృక్షం_కణుపు::Destroy() {
  ఒకవేళ (is_ఆకు())
    తొలగించు ఇది;
  లేకపోతే
    తొలగించు cast<Delta_వృక్షం_Interior_కణుపు>(ఇది);
}

/// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
/// local walk over our contained deltas.
శూన్య Delta_వృక్షం_కణుపు::Recompute_నిండిన_Delta_Locally() {
  పూర్ణాంకం New_నిండిన_Delta = 0;
  కొరకు (గుర్తులేని i = 0, e = get_అంకె_విలువలు_వాడిన(); i != e; ++i)
    New_నిండిన_Delta += విలువలు[i].Delta;
  ఒకవేళ (స్వయంచాలక *లోపల = dyn_cast<Delta_వృక్షం_Interior_కణుపు>(ఇది))
    కొరకు (గుర్తులేని i = 0, e = get_అంకె_విలువలు_వాడిన() + 1; i != e; ++i)
      New_నిండిన_Delta += లోపల->get_బిడ్డ(i)->get_నిండిన_Delta();
  నిండిన_Delta = New_నిండిన_Delta;
}

/// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
/// this node.  If insertion is easy, do it and return false.  Otherwise,
/// split the node, populate InsertRes with info about the split, and return
/// true.
తార్కిక Delta_వృక్షం_కణుపు::DoInsertion(గుర్తులేని File_సూచీ, పూర్ణాంకం Delta,
                                చొప్పించు_ఫలితం *చొప్పించు_Res) {
  // Maintain full delta for this node.
  నిండిన_Delta += Delta;

  // Find the insertion point, the first delta whose index is >= FileIndex.
  గుర్తులేని i = 0, e = get_అంకె_విలువలు_వాడిన();
  అయితే (i != e && File_సూచీ > get_విలువ(i).FileLoc)
    ++i;

  // If we found an a record for exactly this file index, just merge this
  // value into the pre-existing record and finish early.
  ఒకవేళ (i != e && get_విలువ(i).FileLoc == File_సూచీ) {
    // NOTE: Delta could drop to zero here.  This means that the delta entry is
    // useless and could be removed.  Supporting erases is more complex than
    // leaving an entry with Delta=0, so we just leave an entry with Delta=0 in
    // the tree.
    విలువలు[i].Delta += Delta;
    తిరిగి అబద్ధం;
  }

  // Otherwise, we found an insertion point, and we know that the value at the
  // specified index is > FileIndex.  Handle the leaf case first.
  ఒకవేళ (is_ఆకు()) {
    ఒకవేళ (!is_నిండిన()) {
      // For an insertion into a non-full leaf node, just insert the value in
      // its sorted position.  This requires moving later values over.
      ఒకవేళ (i != e)
        memmove(&విలువలు[i + 1], &విలువలు[i], పరిమాణం(విలువలు[0]) * (e - i));
      విలువలు[i] = ఆధారం_Delta::get(File_సూచీ, Delta);
      ++అంకె_విలువలు_వాడిన;
      తిరిగి అబద్ధం;
    }

    // Otherwise, if this is leaf is full, split the node at its median, insert
    // the value into one of the children, and return the result.
    ప్రతిజ్ఞా(చొప్పించు_Res && "No result location specified");
    Do_విభజించు(*చొప్పించు_Res);

    ఒకవేళ (చొప్పించు_Res->విభజించు_2.FileLoc > File_సూచీ)
      చొప్పించు_Res->LHS->DoInsertion(File_సూచీ, Delta, శూన్యసూచిక /*can't fail*/);
    లేకపోతే
      చొప్పించు_Res->RHS->DoInsertion(File_సూచీ, Delta, శూన్యసూచిక /*can't fail*/);
    తిరిగి నిజం;
  }

  // Otherwise, this is an interior node.  Send the request down the tree.
  స్వయంచాలక *లోపల = cast<Delta_వృక్షం_Interior_కణుపు>(ఇది);
  ఒకవేళ (!లోపల->Children[i]->DoInsertion(File_సూచీ, Delta, చొప్పించు_Res))
    తిరిగి అబద్ధం; // If there was space in the child, just return.

  // Okay, this split the subtree, producing a new value and two children to
  // insert here.  If this node is non-full, we can just insert it directly.
  ఒకవేళ (!is_నిండిన()) {
    // Now that we have two nodes and a new element, insert the perclated value
    // into ourself by moving all the later values/children down, then inserting
    // the new one.
    ఒకవేళ (i != e)
      memmove(&లోపల->Children[i + 2], &లోపల->Children[i + 1],
              (e - i) * పరిమాణం(లోపల->Children[0]));
    లోపల->Children[i] = చొప్పించు_Res->LHS;
    లోపల->Children[i + 1] = చొప్పించు_Res->RHS;

    ఒకవేళ (e != i)
      memmove(&విలువలు[i + 1], &విలువలు[i], (e - i) * పరిమాణం(విలువలు[0]));
    విలువలు[i] = చొప్పించు_Res->విభజించు_2;
    ++అంకె_విలువలు_వాడిన;
    తిరిగి అబద్ధం;
  }

  // Finally, if this interior node was full and a node is percolated up, split
  // ourself and return that up the chain.  Start by saving all our info to
  // avoid having the split clobber it.
  లోపల->Children[i] = చొప్పించు_Res->LHS;
  Delta_వృక్షం_కణుపు *SubRHS = చొప్పించు_Res->RHS;
  ఆధారం_Delta Sub_విభజించు = చొప్పించు_Res->విభజించు_2;

  // Do the split.
  Do_విభజించు(*చొప్పించు_Res);

  // Figure out where to insert SubRHS/NewSplit.
  Delta_వృక్షం_Interior_కణుపు *చొప్పించు_Side;
  ఒకవేళ (Sub_విభజించు.FileLoc < చొప్పించు_Res->విభజించు_2.FileLoc)
    చొప్పించు_Side = cast<Delta_వృక్షం_Interior_కణుపు>(చొప్పించు_Res->LHS);
  లేకపోతే
    చొప్పించు_Side = cast<Delta_వృక్షం_Interior_కణుపు>(చొప్పించు_Res->RHS);

  // We now have a non-empty interior node 'InsertSide' to insert
  // SubRHS/SubSplit into.  Find out where to insert SubSplit.

  // Find the insertion point, the first delta whose index is >SubSplit.FileLoc.
  i = 0;
  e = చొప్పించు_Side->get_అంకె_విలువలు_వాడిన();
  అయితే (i != e && Sub_విభజించు.FileLoc > చొప్పించు_Side->get_విలువ(i).FileLoc)
    ++i;

  // Now we know that i is the place to insert the split value into.  Insert it
  // and the child right after it.
  ఒకవేళ (i != e)
    memmove(&చొప్పించు_Side->Children[i + 2], &చొప్పించు_Side->Children[i + 1],
            (e - i) * పరిమాణం(లోపల->Children[0]));
  చొప్పించు_Side->Children[i + 1] = SubRHS;

  ఒకవేళ (e != i)
    memmove(&చొప్పించు_Side->విలువలు[i + 1], &చొప్పించు_Side->విలువలు[i],
            (e - i) * పరిమాణం(విలువలు[0]));
  చొప్పించు_Side->విలువలు[i] = Sub_విభజించు;
  ++చొప్పించు_Side->అంకె_విలువలు_వాడిన;
  చొప్పించు_Side->నిండిన_Delta += Sub_విభజించు.Delta + SubRHS->get_నిండిన_Delta();
  తిరిగి నిజం;
}

/// DoSplit - Split the currently full node (which has 2*WidthFactor-1 values)
/// into two subtrees each with "WidthFactor-1" values and a pivot value.
/// Return the pieces in InsertRes.
శూన్య Delta_వృక్షం_కణుపు::Do_విభజించు(చొప్పించు_ఫలితం &చొప్పించు_Res) {
  ప్రతిజ్ఞా(is_నిండిన() && "Why split a non-full node?");

  // Since this node is full, it contains 2*WidthFactor-1 values.  We move
  // the first 'WidthFactor-1' values to the LHS child (which we leave in this
  // node), propagate one value up, and move the last 'WidthFactor-1' values
  // into the RHS child.

  // Create the new child node.
  Delta_వృక్షం_కణుపు *New_కణుపు;
  ఒకవేళ (స్వయంచాలక *లోపల = dyn_cast<Delta_వృక్షం_Interior_కణుపు>(ఇది)) {
    // If this is an interior node, also move over 'WidthFactor' children
    // into the new node.
    Delta_వృక్షం_Interior_కణుపు *New = కొత్త Delta_వృక్షం_Interior_కణుపు();
    జ్ఞాపకం_నకలు(&New->Children[0], &లోపల->Children[వెడల్పు_గుణకం],
           వెడల్పు_గుణకం * పరిమాణం(లోపల->Children[0]));
    New_కణుపు = New;
  } లేకపోతే {
    // Just create the new leaf node.
    New_కణుపు = కొత్త Delta_వృక్షం_కణుపు();
  }

  // Move over the last 'WidthFactor-1' values from here to NewNode.
  జ్ఞాపకం_నకలు(&New_కణుపు->విలువలు[0], &విలువలు[వెడల్పు_గుణకం],
         (వెడల్పు_గుణకం - 1) * పరిమాణం(విలువలు[0]));

  // Decrease the number of values in the two nodes.
  New_కణుపు->అంకె_విలువలు_వాడిన = అంకె_విలువలు_వాడిన = వెడల్పు_గుణకం - 1;

  // Recompute the two nodes' full delta.
  New_కణుపు->Recompute_నిండిన_Delta_Locally();
  Recompute_నిండిన_Delta_Locally();

  చొప్పించు_Res.LHS = ఇది;
  చొప్పించు_Res.RHS = New_కణుపు;
  చొప్పించు_Res.విభజించు_2 = విలువలు[వెడల్పు_గుణకం - 1];
}

//===----------------------------------------------------------------------===//
//                        DeltaTree Implementation
//===----------------------------------------------------------------------===//

// #define VERIFY_TREE

#నిర్వచితమైతే VERIFY_వృక్షం
/// VerifyTree - Walk the btree performing assertions on various properties to
/// verify consistency.  This is useful for debugging new changes to the tree.
స్థిర శూన్య Verify_వృక్షం(స్థిరరాశి Delta_వృక్షం_కణుపు *N) {
  స్థిరరాశి స్వయంచాలక *లోపల = dyn_cast<Delta_వృక్షం_Interior_కణుపు>(N);
  ఒకవేళ (లోపల == 0) {
    // Verify leaves, just ensure that FullDelta matches up and the elements
    // are in proper order.
    పూర్ణాంకం నిండిన_Delta = 0;
    కొరకు (గుర్తులేని i = 0, e = N->get_అంకె_విలువలు_వాడిన(); i != e; ++i) {
      ఒకవేళ (i)
        ప్రతిజ్ఞా(N->get_విలువ(i - 1).FileLoc < N->get_విలువ(i).FileLoc);
      నిండిన_Delta += N->get_విలువ(i).Delta;
    }
    ప్రతిజ్ఞా(నిండిన_Delta == N->get_నిండిన_Delta());
    తిరిగి;
  }

  // Verify interior nodes: Ensure that FullDelta matches up and the
  // elements are in proper order and the children are in proper order.
  పూర్ణాంకం నిండిన_Delta = 0;
  కొరకు (గుర్తులేని i = 0, e = లోపల->get_అంకె_విలువలు_వాడిన(); i != e; ++i) {
    స్థిరరాశి ఆధారం_Delta &IVal = N->get_విలువ(i);
    స్థిరరాశి Delta_వృక్షం_కణుపు *IChild = లోపల->get_బిడ్డ(i);
    ఒకవేళ (i)
      ప్రతిజ్ఞా(లోపల->get_విలువ(i - 1).FileLoc < IVal.FileLoc);
    నిండిన_Delta += IVal.Delta;
    నిండిన_Delta += IChild->get_నిండిన_Delta();

    // The largest value in child #i should be smaller than FileLoc.
    ప్రతిజ్ఞా(IChild->get_విలువ(IChild->get_అంకె_విలువలు_వాడిన() - 1).FileLoc <
           IVal.FileLoc);

    // The smallest value in child #i+1 should be larger than FileLoc.
    ప్రతిజ్ఞా(లోపల->get_బిడ్డ(i + 1)->get_విలువ(0).FileLoc > IVal.FileLoc);
    Verify_వృక్షం(IChild);
  }

  నిండిన_Delta += లోపల->get_బిడ్డ(లోపల->get_అంకె_విలువలు_వాడిన())->get_నిండిన_Delta();

  ప్రతిజ్ఞా(నిండిన_Delta == N->get_నిండిన_Delta());
}
#అయితే_ముగింపు // VERIFY_TREE

స్థిర Delta_వృక్షం_కణుపు *get_మూలం(శూన్య *మూలం_2) { తిరిగి (Delta_వృక్షం_కణుపు *)మూలం_2; }

Delta_వృక్షం::Delta_వృక్షం() { మూలం_2 = కొత్త Delta_వృక్షం_కణుపు(); }

Delta_వృక్షం::Delta_వృక్షం(స్థిరరాశి Delta_వృక్షం &RHS) {
  // Currently we only support copying when the RHS is empty.
  ప్రతిజ్ఞా(get_మూలం(RHS.మూలం_2)->get_అంకె_విలువలు_వాడిన() == 0 &&
         "Can only copy empty tree");
  మూలం_2 = కొత్త Delta_వృక్షం_కణుపు();
}

Delta_వృక్షం::~Delta_వృక్షం() { get_మూలం(మూలం_2)->Destroy(); }

/// getDeltaAt - Return the accumulated delta at the specified file offset.
/// This includes all insertions or delections that occurred *before* the
/// specified file index.
పూర్ణాంకం Delta_వృక్షం::getDeltaAt(గుర్తులేని File_సూచీ) స్థిరరాశి {
  స్థిరరాశి Delta_వృక్షం_కణుపు *కణుపు = get_మూలం(మూలం_2);

  పూర్ణాంకం ఫలితం = 0;

  // Walk down the tree.
  అయితే (నిజం) {
    // For all nodes, include any local deltas before the specified file
    // index by summing them up directly.  Keep track of how many were
    // included.
    గుర్తులేని అంకె_Vals_Greater = 0;
    కొరకు (గుర్తులేని e = కణుపు->get_అంకె_విలువలు_వాడిన(); అంకె_Vals_Greater != e;
         ++అంకె_Vals_Greater) {
      స్థిరరాశి ఆధారం_Delta &Val = కణుపు->get_విలువ(అంకె_Vals_Greater);

      ఒకవేళ (Val.FileLoc >= File_సూచీ)
        ఆపు;
      ఫలితం += Val.Delta;
    }

    // If we have an interior node, include information about children and
    // recurse.  Otherwise, if we have a leaf, we're done.
    స్థిరరాశి స్వయంచాలక *లోపల = dyn_cast<Delta_వృక్షం_Interior_కణుపు>(కణుపు);
    ఒకవేళ (!లోపల)
      తిరిగి ఫలితం;

    // Include any children to the left of the values we skipped, all of
    // their deltas should be included as well.
    కొరకు (గుర్తులేని i = 0; i != అంకె_Vals_Greater; ++i)
      ఫలితం += లోపల->get_బిడ్డ(i)->get_నిండిన_Delta();

    // If we found exactly the value we were looking for, break off the
    // search early.  There is no need to search the RHS of the value for
    // partial results.
    ఒకవేళ (అంకె_Vals_Greater != కణుపు->get_అంకె_విలువలు_వాడిన() &&
        కణుపు->get_విలువ(అంకె_Vals_Greater).FileLoc == File_సూచీ)
      తిరిగి ఫలితం + లోపల->get_బిడ్డ(అంకె_Vals_Greater)->get_నిండిన_Delta();

    // Otherwise, traverse down the tree.  The selected subtree may be
    // partially included in the range.
    కణుపు = లోపల->get_బిడ్డ(అంకె_Vals_Greater);
  }
  // NOT REACHED.
}

/// AddDelta - When a change is made that shifts around the text buffer,
/// this method is used to record that info.  It inserts a delta of 'Delta'
/// into the current DeltaTree at offset FileIndex.
శూన్య Delta_వృక్షం::కూడు_Delta(గుర్తులేని File_సూచీ, పూర్ణాంకం Delta) {
  ప్రతిజ్ఞా(Delta && "Adding a noop?");
  Delta_వృక్షం_కణుపు *My_మూలం = get_మూలం(మూలం_2);

  Delta_వృక్షం_కణుపు::చొప్పించు_ఫలితం చొప్పించు_Res;
  ఒకవేళ (My_మూలం->DoInsertion(File_సూచీ, Delta, &చొప్పించు_Res)) {
    మూలం_2 = కొత్త Delta_వృక్షం_Interior_కణుపు(చొప్పించు_Res);
#నిర్వచితమైతే VERIFY_వృక్షం
    My_మూలం = మూలం_2;
#అయితే_ముగింపు
  }

#నిర్వచితమైతే VERIFY_వృక్షం
  Verify_వృక్షం(My_మూలం);
#అయితే_ముగింపు
}
