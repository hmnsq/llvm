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

#अन्तर्भाव "llvm/ADT/DeltaTree.h"
#अन्तर्भाव "llvm/Support/Casting.h"
#अन्तर्भाव <cassert>
#अन्तर्भाव <cstring>

उपयुज्य नामाकाश llvm;

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

नामाकाश {

/// SourceDelta - As code in the original input buffer is added and deleted,
/// SourceDelta records are used to keep track of how the input SourceLocation
/// object is mapped into the output buffer.
संरचना SourceDelta {
  अचिह्नित FileLoc;
  पूर्णाङ्क Delta;

  स्थिर SourceDelta get(अचिह्नित Loc, पूर्णाङ्क D) {
    SourceDelta Delta;
    Delta.FileLoc = Loc;
    Delta.Delta = D;
    प्रत्यागम Delta;
  }
};

/// DeltaTreeNode - The common part of all nodes.
///
वर्ग Delta_वृक्षः_ग्रन्थिः {
सार्वजनिक:
  संरचना निवेशय_परिणाम {
    Delta_वृक्षः_ग्रन्थिः *LHS, *RHS;
    SourceDelta विभज_2;
  };

गुप्त:
  मित्रम् वर्ग Delta_वृक्षः_Interior_ग्रन्थिः;

  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// WidthFactor-1 K/V pairs (except the root) and may have at most
  /// 2*WidthFactor-1 K/V pairs.
  गणना { विस्तारः_गुणकः = 8 };

  /// Values - This tracks the SourceDelta's currently in this node.
  SourceDelta मानानि[2 * विस्तारः_गुणकः - 1];

  /// NumValuesUsed - This tracks the number of values this node currently
  /// holds.
  अचिह्नित वर्ण अङ्कः_मानानि_प्रयुक्तम् = 0;

  /// IsLeaf - This is true if this is a leaf of the btree.  If false, this is
  /// an interior node, and is actually an instance of DeltaTreeInteriorNode.
  तार्किक Is_पर्णम्;

  /// FullDelta - This is the full delta of all the values in this node and
  /// all children nodes.
  पूर्णाङ्क पूर्णम्_Delta = 0;

सार्वजनिक:
  Delta_वृक्षः_ग्रन्थिः(तार्किक is_पर्णम् = सत्य) : Is_पर्णम्(is_पर्णम्) {}

  तार्किक is_पर्णम्() ध्रुव { प्रत्यागम Is_पर्णम्; }
  पूर्णाङ्क get_पूर्णम्_Delta() ध्रुव { प्रत्यागम पूर्णम्_Delta; }
  तार्किक is_पूर्णम्() ध्रुव { प्रत्यागम अङ्कः_मानानि_प्रयुक्तम् == 2 * विस्तारः_गुणकः - 1; }

  अचिह्नित get_अङ्कः_मानानि_प्रयुक्तम्() ध्रुव { प्रत्यागम अङ्कः_मानानि_प्रयुक्तम्; }

  ध्रुव SourceDelta &get_मानम्(अचिह्नित i) ध्रुव {
    प्रतिज्ञा(i < अङ्कः_मानानि_प्रयुक्तम् && "Invalid value #");
    प्रत्यागम मानानि[i];
  }

  SourceDelta &get_मानम्(अचिह्नित i) {
    प्रतिज्ञा(i < अङ्कः_मानानि_प्रयुक्तम् && "Invalid value #");
    प्रत्यागम मानानि[i];
  }

  /// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
  /// this node.  If insertion is easy, do it and return false.  Otherwise,
  /// split the node, populate InsertRes with info about the split, and return
  /// true.
  तार्किक DoInsertion(अचिह्नित File_सूचकांक, पूर्णाङ्क Delta, निवेशय_परिणाम *निवेशय_Res);

  शून्य Do_विभज(निवेशय_परिणाम &निवेशय_Res);

  /// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
  /// local walk over our contained deltas.
  शून्य Recompute_पूर्णम्_Delta_Locally();

  शून्य Destroy();
};

/// DeltaTreeInteriorNode - When isLeaf = false, a node has child pointers.
/// This class tracks them.
वर्ग Delta_वृक्षः_Interior_ग्रन्थिः : सार्वजनिक Delta_वृक्षः_ग्रन्थिः {
  मित्रम् वर्ग Delta_वृक्षः_ग्रन्थिः;

  Delta_वृक्षः_ग्रन्थिः *Children[2 * विस्तारः_गुणकः];

  ~Delta_वृक्षः_Interior_ग्रन्थिः() {
    प्रति (अचिह्नित i = 0, e = अङ्कः_मानानि_प्रयुक्तम् + 1; i != e; ++i)
      Children[i]->Destroy();
  }

सार्वजनिक:
  Delta_वृक्षः_Interior_ग्रन्थिः() : Delta_वृक्षः_ग्रन्थिः(असत्य /*nonleaf*/) {}

  Delta_वृक्षः_Interior_ग्रन्थिः(ध्रुव निवेशय_परिणाम &IR)
      : Delta_वृक्षः_ग्रन्थिः(असत्य /*nonleaf*/) {
    Children[0] = IR.LHS;
    Children[1] = IR.RHS;
    मानानि[0] = IR.विभज_2;
    पूर्णम्_Delta =
        IR.LHS->get_पूर्णम्_Delta() + IR.RHS->get_पूर्णम्_Delta() + IR.विभज_2.Delta;
    अङ्कः_मानानि_प्रयुक्तम् = 1;
  }

  ध्रुव Delta_वृक्षः_ग्रन्थिः *getChild(अचिह्नित i) ध्रुव {
    प्रतिज्ञा(i < get_अङ्कः_मानानि_प्रयुक्तम्() + 1 && "Invalid child");
    प्रत्यागम Children[i];
  }

  Delta_वृक्षः_ग्रन्थिः *getChild(अचिह्नित i) {
    प्रतिज्ञा(i < get_अङ्कः_मानानि_प्रयुक्तम्() + 1 && "Invalid child");
    प्रत्यागम Children[i];
  }

  स्थिर तार्किक classof(ध्रुव Delta_वृक्षः_ग्रन्थिः *N) { प्रत्यागम !N->is_पर्णम्(); }
};

} // namespace

/// Destroy - A 'virtual' destructor.
शून्य Delta_वृक्षः_ग्रन्थिः::Destroy() {
  यदि (is_पर्णम्())
    विलोपय एतत्;
  अन्यथा
    विलोपय cast<Delta_वृक्षः_Interior_ग्रन्थिः>(एतत्);
}

/// RecomputeFullDeltaLocally - Recompute the FullDelta field by doing a
/// local walk over our contained deltas.
शून्य Delta_वृक्षः_ग्रन्थिः::Recompute_पूर्णम्_Delta_Locally() {
  पूर्णाङ्क New_पूर्णम्_Delta = 0;
  प्रति (अचिह्नित i = 0, e = get_अङ्कः_मानानि_प्रयुक्तम्(); i != e; ++i)
    New_पूर्णम्_Delta += मानानि[i].Delta;
  यदि (स्वचालित *अन्तर् = dyn_cast<Delta_वृक्षः_Interior_ग्रन्थिः>(एतत्))
    प्रति (अचिह्नित i = 0, e = get_अङ्कः_मानानि_प्रयुक्तम्() + 1; i != e; ++i)
      New_पूर्णम्_Delta += अन्तर्->getChild(i)->get_पूर्णम्_Delta();
  पूर्णम्_Delta = New_पूर्णम्_Delta;
}

/// DoInsertion - Do an insertion of the specified FileIndex/Delta pair into
/// this node.  If insertion is easy, do it and return false.  Otherwise,
/// split the node, populate InsertRes with info about the split, and return
/// true.
तार्किक Delta_वृक्षः_ग्रन्थिः::DoInsertion(अचिह्नित File_सूचकांक, पूर्णाङ्क Delta,
                                निवेशय_परिणाम *निवेशय_Res) {
  // Maintain full delta for this node.
  पूर्णम्_Delta += Delta;

  // Find the insertion point, the first delta whose index is >= FileIndex.
  अचिह्नित i = 0, e = get_अङ्कः_मानानि_प्रयुक्तम्();
  यावत् (i != e && File_सूचकांक > get_मानम्(i).FileLoc)
    ++i;

  // If we found an a record for exactly this file index, just merge this
  // value into the pre-existing record and finish early.
  यदि (i != e && get_मानम्(i).FileLoc == File_सूचकांक) {
    // NOTE: Delta could drop to zero here.  This means that the delta entry is
    // useless and could be removed.  Supporting erases is more complex than
    // leaving an entry with Delta=0, so we just leave an entry with Delta=0 in
    // the tree.
    मानानि[i].Delta += Delta;
    प्रत्यागम असत्य;
  }

  // Otherwise, we found an insertion point, and we know that the value at the
  // specified index is > FileIndex.  Handle the leaf case first.
  यदि (is_पर्णम्()) {
    यदि (!is_पूर्णम्()) {
      // For an insertion into a non-full leaf node, just insert the value in
      // its sorted position.  This requires moving later values over.
      यदि (i != e)
        memmove(&मानानि[i + 1], &मानानि[i], परिमाण(मानानि[0]) * (e - i));
      मानानि[i] = SourceDelta::get(File_सूचकांक, Delta);
      ++अङ्कः_मानानि_प्रयुक्तम्;
      प्रत्यागम असत्य;
    }

    // Otherwise, if this is leaf is full, split the node at its median, insert
    // the value into one of the children, and return the result.
    प्रतिज्ञा(निवेशय_Res && "No result location specified");
    Do_विभज(*निवेशय_Res);

    यदि (निवेशय_Res->विभज_2.FileLoc > File_सूचकांक)
      निवेशय_Res->LHS->DoInsertion(File_सूचकांक, Delta, शून्यसूचकः /*can't fail*/);
    अन्यथा
      निवेशय_Res->RHS->DoInsertion(File_सूचकांक, Delta, शून्यसूचकः /*can't fail*/);
    प्रत्यागम सत्य;
  }

  // Otherwise, this is an interior node.  Send the request down the tree.
  स्वचालित *अन्तर् = cast<Delta_वृक्षः_Interior_ग्रन्थिः>(एतत्);
  यदि (!अन्तर्->Children[i]->DoInsertion(File_सूचकांक, Delta, निवेशय_Res))
    प्रत्यागम असत्य; // If there was space in the child, just return.

  // Okay, this split the subtree, producing a new value and two children to
  // insert here.  If this node is non-full, we can just insert it directly.
  यदि (!is_पूर्णम्()) {
    // Now that we have two nodes and a new element, insert the perclated value
    // into ourself by moving all the later values/children down, then inserting
    // the new one.
    यदि (i != e)
      memmove(&अन्तर्->Children[i + 2], &अन्तर्->Children[i + 1],
              (e - i) * परिमाण(अन्तर्->Children[0]));
    अन्तर्->Children[i] = निवेशय_Res->LHS;
    अन्तर्->Children[i + 1] = निवेशय_Res->RHS;

    यदि (e != i)
      memmove(&मानानि[i + 1], &मानानि[i], (e - i) * परिमाण(मानानि[0]));
    मानानि[i] = निवेशय_Res->विभज_2;
    ++अङ्कः_मानानि_प्रयुक्तम्;
    प्रत्यागम असत्य;
  }

  // Finally, if this interior node was full and a node is percolated up, split
  // ourself and return that up the chain.  Start by saving all our info to
  // avoid having the split clobber it.
  अन्तर्->Children[i] = निवेशय_Res->LHS;
  Delta_वृक्षः_ग्रन्थिः *SubRHS = निवेशय_Res->RHS;
  SourceDelta Sub_विभज = निवेशय_Res->विभज_2;

  // Do the split.
  Do_विभज(*निवेशय_Res);

  // Figure out where to insert SubRHS/NewSplit.
  Delta_वृक्षः_Interior_ग्रन्थिः *निवेशय_Side;
  यदि (Sub_विभज.FileLoc < निवेशय_Res->विभज_2.FileLoc)
    निवेशय_Side = cast<Delta_वृक्षः_Interior_ग्रन्थिः>(निवेशय_Res->LHS);
  अन्यथा
    निवेशय_Side = cast<Delta_वृक्षः_Interior_ग्रन्थिः>(निवेशय_Res->RHS);

  // We now have a non-empty interior node 'InsertSide' to insert
  // SubRHS/SubSplit into.  Find out where to insert SubSplit.

  // Find the insertion point, the first delta whose index is >SubSplit.FileLoc.
  i = 0;
  e = निवेशय_Side->get_अङ्कः_मानानि_प्रयुक्तम्();
  यावत् (i != e && Sub_विभज.FileLoc > निवेशय_Side->get_मानम्(i).FileLoc)
    ++i;

  // Now we know that i is the place to insert the split value into.  Insert it
  // and the child right after it.
  यदि (i != e)
    memmove(&निवेशय_Side->Children[i + 2], &निवेशय_Side->Children[i + 1],
            (e - i) * परिमाण(अन्तर्->Children[0]));
  निवेशय_Side->Children[i + 1] = SubRHS;

  यदि (e != i)
    memmove(&निवेशय_Side->मानानि[i + 1], &निवेशय_Side->मानानि[i],
            (e - i) * परिमाण(मानानि[0]));
  निवेशय_Side->मानानि[i] = Sub_विभज;
  ++निवेशय_Side->अङ्कः_मानानि_प्रयुक्तम्;
  निवेशय_Side->पूर्णम्_Delta += Sub_विभज.Delta + SubRHS->get_पूर्णम्_Delta();
  प्रत्यागम सत्य;
}

/// DoSplit - Split the currently full node (which has 2*WidthFactor-1 values)
/// into two subtrees each with "WidthFactor-1" values and a pivot value.
/// Return the pieces in InsertRes.
शून्य Delta_वृक्षः_ग्रन्थिः::Do_विभज(निवेशय_परिणाम &निवेशय_Res) {
  प्रतिज्ञा(is_पूर्णम्() && "Why split a non-full node?");

  // Since this node is full, it contains 2*WidthFactor-1 values.  We move
  // the first 'WidthFactor-1' values to the LHS child (which we leave in this
  // node), propagate one value up, and move the last 'WidthFactor-1' values
  // into the RHS child.

  // Create the new child node.
  Delta_वृक्षः_ग्रन्थिः *New_ग्रन्थिः;
  यदि (स्वचालित *अन्तर् = dyn_cast<Delta_वृक्षः_Interior_ग्रन्थिः>(एतत्)) {
    // If this is an interior node, also move over 'WidthFactor' children
    // into the new node.
    Delta_वृक्षः_Interior_ग्रन्थिः *New = नूतन Delta_वृक्षः_Interior_ग्रन्थिः();
    स्मृतिं_प्रतिलिख(&New->Children[0], &अन्तर्->Children[विस्तारः_गुणकः],
           विस्तारः_गुणकः * परिमाण(अन्तर्->Children[0]));
    New_ग्रन्थिः = New;
  } अन्यथा {
    // Just create the new leaf node.
    New_ग्रन्थिः = नूतन Delta_वृक्षः_ग्रन्थिः();
  }

  // Move over the last 'WidthFactor-1' values from here to NewNode.
  स्मृतिं_प्रतिलिख(&New_ग्रन्थिः->मानानि[0], &मानानि[विस्तारः_गुणकः],
         (विस्तारः_गुणकः - 1) * परिमाण(मानानि[0]));

  // Decrease the number of values in the two nodes.
  New_ग्रन्थिः->अङ्कः_मानानि_प्रयुक्तम् = अङ्कः_मानानि_प्रयुक्तम् = विस्तारः_गुणकः - 1;

  // Recompute the two nodes' full delta.
  New_ग्रन्थिः->Recompute_पूर्णम्_Delta_Locally();
  Recompute_पूर्णम्_Delta_Locally();

  निवेशय_Res.LHS = एतत्;
  निवेशय_Res.RHS = New_ग्रन्थिः;
  निवेशय_Res.विभज_2 = मानानि[विस्तारः_गुणकः - 1];
}

//===----------------------------------------------------------------------===//
//                        DeltaTree Implementation
//===----------------------------------------------------------------------===//

// #define VERIFY_TREE

#यदि_परिभाषितम् VERIFY_वृक्षः
/// VerifyTree - Walk the btree performing assertions on various properties to
/// verify consistency.  This is useful for debugging new changes to the tree.
स्थिर शून्य Verify_वृक्षः(ध्रुव Delta_वृक्षः_ग्रन्थिः *N) {
  ध्रुव स्वचालित *अन्तर् = dyn_cast<Delta_वृक्षः_Interior_ग्रन्थिः>(N);
  यदि (अन्तर् == 0) {
    // Verify leaves, just ensure that FullDelta matches up and the elements
    // are in proper order.
    पूर्णाङ्क पूर्णम्_Delta = 0;
    प्रति (अचिह्नित i = 0, e = N->get_अङ्कः_मानानि_प्रयुक्तम्(); i != e; ++i) {
      यदि (i)
        प्रतिज्ञा(N->get_मानम्(i - 1).FileLoc < N->get_मानम्(i).FileLoc);
      पूर्णम्_Delta += N->get_मानम्(i).Delta;
    }
    प्रतिज्ञा(पूर्णम्_Delta == N->get_पूर्णम्_Delta());
    प्रत्यागम;
  }

  // Verify interior nodes: Ensure that FullDelta matches up and the
  // elements are in proper order and the children are in proper order.
  पूर्णाङ्क पूर्णम्_Delta = 0;
  प्रति (अचिह्नित i = 0, e = अन्तर्->get_अङ्कः_मानानि_प्रयुक्तम्(); i != e; ++i) {
    ध्रुव SourceDelta &IVal = N->get_मानम्(i);
    ध्रुव Delta_वृक्षः_ग्रन्थिः *IChild = अन्तर्->getChild(i);
    यदि (i)
      प्रतिज्ञा(अन्तर्->get_मानम्(i - 1).FileLoc < IVal.FileLoc);
    पूर्णम्_Delta += IVal.Delta;
    पूर्णम्_Delta += IChild->get_पूर्णम्_Delta();

    // The largest value in child #i should be smaller than FileLoc.
    प्रतिज्ञा(IChild->get_मानम्(IChild->get_अङ्कः_मानानि_प्रयुक्तम्() - 1).FileLoc <
           IVal.FileLoc);

    // The smallest value in child #i+1 should be larger than FileLoc.
    प्रतिज्ञा(अन्तर्->getChild(i + 1)->get_मानम्(0).FileLoc > IVal.FileLoc);
    Verify_वृक्षः(IChild);
  }

  पूर्णम्_Delta += अन्तर्->getChild(अन्तर्->get_अङ्कः_मानानि_प्रयुक्तम्())->get_पूर्णम्_Delta();

  प्रतिज्ञा(पूर्णम्_Delta == N->get_पूर्णम्_Delta());
}
#यदि_अन्तः // VERIFY_TREE

स्थिर Delta_वृक्षः_ग्रन्थिः *get_मूलम्(शून्य *मूलम्_2) { प्रत्यागम (Delta_वृक्षः_ग्रन्थिः *)मूलम्_2; }

Delta_वृक्षः::Delta_वृक्षः() { मूलम्_2 = नूतन Delta_वृक्षः_ग्रन्थिः(); }

Delta_वृक्षः::Delta_वृक्षः(ध्रुव Delta_वृक्षः &RHS) {
  // Currently we only support copying when the RHS is empty.
  प्रतिज्ञा(get_मूलम्(RHS.मूलम्_2)->get_अङ्कः_मानानि_प्रयुक्तम्() == 0 &&
         "Can only copy empty tree");
  मूलम्_2 = नूतन Delta_वृक्षः_ग्रन्थिः();
}

Delta_वृक्षः::~Delta_वृक्षः() { get_मूलम्(मूलम्_2)->Destroy(); }

/// getDeltaAt - Return the accumulated delta at the specified file offset.
/// This includes all insertions or delections that occurred *before* the
/// specified file index.
पूर्णाङ्क Delta_वृक्षः::getDeltaAt(अचिह्नित File_सूचकांक) ध्रुव {
  ध्रुव Delta_वृक्षः_ग्रन्थिः *ग्रन्थिः = get_मूलम्(मूलम्_2);

  पूर्णाङ्क परिणाम = 0;

  // Walk down the tree.
  यावत् (सत्य) {
    // For all nodes, include any local deltas before the specified file
    // index by summing them up directly.  Keep track of how many were
    // included.
    अचिह्नित अङ्कः_Vals_Greater = 0;
    प्रति (अचिह्नित e = ग्रन्थिः->get_अङ्कः_मानानि_प्रयुक्तम्(); अङ्कः_Vals_Greater != e;
         ++अङ्कः_Vals_Greater) {
      ध्रुव SourceDelta &Val = ग्रन्थिः->get_मानम्(अङ्कः_Vals_Greater);

      यदि (Val.FileLoc >= File_सूचकांक)
        भङ्ग;
      परिणाम += Val.Delta;
    }

    // If we have an interior node, include information about children and
    // recurse.  Otherwise, if we have a leaf, we're done.
    ध्रुव स्वचालित *अन्तर् = dyn_cast<Delta_वृक्षः_Interior_ग्रन्थिः>(ग्रन्थिः);
    यदि (!अन्तर्)
      प्रत्यागम परिणाम;

    // Include any children to the left of the values we skipped, all of
    // their deltas should be included as well.
    प्रति (अचिह्नित i = 0; i != अङ्कः_Vals_Greater; ++i)
      परिणाम += अन्तर्->getChild(i)->get_पूर्णम्_Delta();

    // If we found exactly the value we were looking for, break off the
    // search early.  There is no need to search the RHS of the value for
    // partial results.
    यदि (अङ्कः_Vals_Greater != ग्रन्थिः->get_अङ्कः_मानानि_प्रयुक्तम्() &&
        ग्रन्थिः->get_मानम्(अङ्कः_Vals_Greater).FileLoc == File_सूचकांक)
      प्रत्यागम परिणाम + अन्तर्->getChild(अङ्कः_Vals_Greater)->get_पूर्णम्_Delta();

    // Otherwise, traverse down the tree.  The selected subtree may be
    // partially included in the range.
    ग्रन्थिः = अन्तर्->getChild(अङ्कः_Vals_Greater);
  }
  // NOT REACHED.
}

/// AddDelta - When a change is made that shifts around the text buffer,
/// this method is used to record that info.  It inserts a delta of 'Delta'
/// into the current DeltaTree at offset FileIndex.
शून्य Delta_वृक्षः::सङ्कलय_Delta(अचिह्नित File_सूचकांक, पूर्णाङ्क Delta) {
  प्रतिज्ञा(Delta && "Adding a noop?");
  Delta_वृक्षः_ग्रन्थिः *My_मूलम् = get_मूलम्(मूलम्_2);

  Delta_वृक्षः_ग्रन्थिः::निवेशय_परिणाम निवेशय_Res;
  यदि (My_मूलम्->DoInsertion(File_सूचकांक, Delta, &निवेशय_Res)) {
    मूलम्_2 = नूतन Delta_वृक्षः_Interior_ग्रन्थिः(निवेशय_Res);
#यदि_परिभाषितम् VERIFY_वृक्षः
    My_मूलम् = मूलम्_2;
#यदि_अन्तः
  }

#यदि_परिभाषितम् VERIFY_वृक्षः
  Verify_वृक्षः(My_मूलम्);
#यदि_अन्तः
}
