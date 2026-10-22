//===- RewriteRope.cpp - Rope specialized for rewriter --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements the RewriteRope class, which is a powerful string.
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/ADT/RewriteRope.h"
#అన్తర్భావ "llvm/Support/Casting.h"
#అన్తర్భావ <algorithm>
#అన్తర్భావ <cassert>
#అన్తర్భావ <cstring>

ఉపయోగించి నామాకాశ llvm;

/// RewriteRope is a "strong" string class, designed to make insertions and
/// deletions in the middle of the string nearly constant time (really, they are
/// O(log N), but with a very low constant factor).
///
/// The implementation of this datastructure is a conceptual linear sequence of
/// RopePiece elements.  Each RopePiece represents a view on a separately
/// allocated and reference counted string.  This means that splitting a very
/// long string can be done in constant time by splitting a RopePiece that
/// references the whole string into two rope pieces that reference each half.
/// Once split, another string can be inserted in between the two halves by
/// inserting a RopePiece in between the two others.  All of this is very
/// inexpensive: it takes time proportional to the number of RopePieces, not the
/// length of the strings they represent.
///
/// While a linear sequences of RopePieces is the conceptual model, the actual
/// implementation captures them in an adapted B+ Tree.  Using a B+ tree (which
/// is a tree that keeps the values in the leaves and has where each node
/// contains a reasonable number of pointers to children/values) allows us to
/// maintain efficient operation when the RewriteRope contains a *huge* number
/// of RopePieces.  The basic idea of the B+ Tree is that it allows us to find
/// the RopePiece corresponding to some offset very efficiently, and it
/// automatically balances itself on insertions of RopePieces (which can happen
/// for both insertions and erases of string ranges).
///
/// The one wrinkle on the theory is that we don't attempt to keep the tree
/// properly balanced when erases happen.  Erases of string data can both insert
/// new RopePieces (e.g. when the middle of some other rope piece is deleted,
/// which results in two rope pieces, which is just like an insert) or it can
/// reduce the number of RopePieces maintained by the B+Tree.  In the case when
/// the number of RopePieces is reduced, we don't attempt to maintain the
/// standard 'invariant' that each node in the tree contains at least
/// 'WidthFactor' children/values.  For our use cases, this doesn't seem to
/// matter.
///
/// The implementation below is primarily implemented in terms of three classes:
///   RopePieceBTreeNode - Common base class for:
///
///     RopePieceBTreeLeaf - Directly manages up to '2*WidthFactor' RopePiece
///          nodes.  This directly represents a chunk of the string with those
///          RopePieces concatenated.
///     RopePieceBTreeInterior - An interior node in the B+ Tree, which manages
///          up to '2*WidthFactor' other nodes in the tree.

నామాకాశ {

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Class
//===----------------------------------------------------------------------===//

/// RopePieceBTreeNode - Common base class of RopePieceBTreeLeaf and
/// RopePieceBTreeInterior.  This provides some 'virtual' dispatching methods
/// and a flag that determines which subclass the instance is.  Also
/// important, this node knows the full extend of the node, including any
/// children that it has.  This allows efficient skipping over entire subtrees
/// when looking for an offset in the BTree.
తరగతి Rope_Piece_BTree_కణుపు {
రక్షిత:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  గణన { వెడల్పు_గుణకం = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  గుర్తులేని కొలత = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  తార్కిక Is_ఆకు;

  Rope_Piece_BTree_కణుపు(తార్కిక is_ఆకు) : Is_ఆకు(is_ఆకు) {}
  ~Rope_Piece_BTree_కణుపు() = అప్రమేయ;

బహిరంగ:
  తార్కిక is_ఆకు() స్థిరరాశి { తిరిగి Is_ఆకు; }
  గుర్తులేని కొలత_2() స్థిరరాశి { తిరిగి కొలత; }

  శూన్య Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_కణుపు *విభజించు(గుర్తులేని స్థానభ్రంశం);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_కణుపు *చొప్పించు(గుర్తులేని స్థానభ్రంశం, స్థిరరాశి RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  శూన్య erase(గుర్తులేని స్థానభ్రంశం, గుర్తులేని అంకె_బైట్లు);
};

//===----------------------------------------------------------------------===//
// RopePieceBTreeLeaf Class
//===----------------------------------------------------------------------===//

/// RopePieceBTreeLeaf - Directly manages up to '2*WidthFactor' RopePiece
/// nodes.  This directly represents a chunk of the string with those
/// RopePieces concatenated.  Since this is a B+Tree, all values (in this case
/// instances of RopePiece) are stored in leaves like this.  To make iteration
/// over the leaves efficient, they maintain a singly linked list through the
/// NextLeaf field.  This allows the B+Tree forward iterator to be constant
/// time for all increments.
తరగతి Rope_Piece_BTree_ఆకు : బహిరంగ Rope_Piece_BTree_కణుపు {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  గుర్తులేని అక్షరం అంకె_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * వెడల్పు_గుణకం];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  Rope_Piece_BTree_ఆకు **Prev_ఆకు = శూన్యసూచిక;
  Rope_Piece_BTree_ఆకు *తదుపరి_ఆకు = శూన్యసూచిక;

బహిరంగ:
  Rope_Piece_BTree_ఆకు() : Rope_Piece_BTree_కణుపు(నిజం) {}

  ~Rope_Piece_BTree_ఆకు() {
    ఒకవేళ (Prev_ఆకు || తదుపరి_ఆకు)
      remove_From_ఆకు_లోపల_క్రమం();
    clear();
  }

  తార్కిక is_నిండిన() స్థిరరాశి { తిరిగి అంకె_Pieces == 2 * వెడల్పు_గుణకం; }

  /// clear - Remove all rope pieces from this leaf.
  శూన్య clear() {
    అయితే (అంకె_Pieces)
      Pieces[--అంకె_Pieces] = RopePiece();
    కొలత = 0;
  }

  గుర్తులేని get_అంకె_Pieces() స్థిరరాశి { తిరిగి అంకె_Pieces; }

  స్థిరరాశి RopePiece &getPiece(గుర్తులేని i) స్థిరరాశి {
    ప్రతిజ్ఞా(i < get_అంకె_Pieces() && "Invalid piece ID");
    తిరిగి Pieces[i];
  }

  స్థిరరాశి Rope_Piece_BTree_ఆకు *get_తదుపరి_ఆకు_లోపల_క్రమం() స్థిరరాశి { తిరిగి తదుపరి_ఆకు; }

  శూన్య చొప్పించు_After_ఆకు_లోపల_క్రమం(Rope_Piece_BTree_ఆకు *కణుపు) {
    ప్రతిజ్ఞా(!Prev_ఆకు && !తదుపరి_ఆకు && "Already in ordering");

    తదుపరి_ఆకు = కణుపు->తదుపరి_ఆకు;
    ఒకవేళ (తదుపరి_ఆకు)
      తదుపరి_ఆకు->Prev_ఆకు = &తదుపరి_ఆకు;
    Prev_ఆకు = &కణుపు->తదుపరి_ఆకు;
    కణుపు->తదుపరి_ఆకు = ఇది;
  }

  శూన్య remove_From_ఆకు_లోపల_క్రమం() {
    ఒకవేళ (Prev_ఆకు) {
      *Prev_ఆకు = తదుపరి_ఆకు;
      ఒకవేళ (తదుపరి_ఆకు)
        తదుపరి_ఆకు->Prev_ఆకు = Prev_ఆకు;
    } లేకపోతే ఒకవేళ (తదుపరి_ఆకు) {
      తదుపరి_ఆకు->Prev_ఆకు = శూన్యసూచిక;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  శూన్య నిండిన_Recompute_కొలత_Locally() {
    కొలత = 0;
    కొరకు (గుర్తులేని i = 0, e = get_అంకె_Pieces(); i != e; ++i)
      కొలత += getPiece(i).కొలత_2();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_కణుపు *విభజించు(గుర్తులేని స్థానభ్రంశం);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_కణుపు *చొప్పించు(గుర్తులేని స్థానభ్రంశం, స్థిరరాశి RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  శూన్య erase(గుర్తులేని స్థానభ్రంశం, గుర్తులేని అంకె_బైట్లు);

  స్థిర తార్కిక classof(స్థిరరాశి Rope_Piece_BTree_కణుపు *N) { తిరిగి N->is_ఆకు(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_కణుపు *Rope_Piece_BTree_ఆకు::విభజించు(గుర్తులేని స్థానభ్రంశం) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  ఒకవేళ (స్థానభ్రంశం == 0 || స్థానభ్రంశం == కొలత_2()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    తిరిగి శూన్యసూచిక;
  }

  // Find the piece that this offset lands in.
  గుర్తులేని PieceOffs = 0;
  గుర్తులేని i = 0;
  అయితే (స్థానభ్రంశం >= PieceOffs + Pieces[i].కొలత_2()) {
    PieceOffs += Pieces[i].కొలత_2();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  ఒకవేళ (PieceOffs == స్థానభ్రంశం)
    తిరిగి శూన్యసూచిక;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  గుర్తులేని Intra_Piece_స్థానభ్రంశం = స్థానభ్రంశం - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece తోక(Pieces[i].Str_దత్తాంశం, Pieces[i].ప్రారంభం_Offs + Intra_Piece_స్థానభ్రంశం,
                 Pieces[i].చివర_Offs);
  కొలత -= Pieces[i].కొలత_2();
  Pieces[i].చివర_Offs = Pieces[i].ప్రారంభం_Offs + Intra_Piece_స్థానభ్రంశం;
  కొలత += Pieces[i].కొలత_2();

  తిరిగి చొప్పించు(స్థానభ్రంశం, తోక);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_కణుపు *Rope_Piece_BTree_ఆకు::చొప్పించు(గుర్తులేని స్థానభ్రంశం,
                                               స్థిరరాశి RopePiece &R) {
  // If this node is not full, insert the piece.
  ఒకవేళ (!is_నిండిన()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    గుర్తులేని i = 0, e = get_అంకె_Pieces();
    ఒకవేళ (స్థానభ్రంశం == కొలత_2()) {
      // Fastpath for a common case.
      i = e;
    } లేకపోతే {
      గుర్తులేని స్థానం_Offs = 0;
      కొరకు (; స్థానభ్రంశం > స్థానం_Offs; ++i)
        స్థానం_Offs += getPiece(i).కొలత_2();
      ప్రతిజ్ఞా(స్థానం_Offs == స్థానభ్రంశం && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    కొరకు (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++అంకె_Pieces;
    కొలత += R.కొలత_2();
    తిరిగి శూన్యసూచిక;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  Rope_Piece_BTree_ఆకు *New_కణుపు = కొత్త Rope_Piece_BTree_ఆకు();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[వెడల్పు_గుణకం], &Pieces[2 * వెడల్పు_గుణకం],
            &New_కణుపు->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[వెడల్పు_గుణకం], &Pieces[2 * వెడల్పు_గుణకం], RopePiece());

  // Decrease the number of values in the two nodes.
  New_కణుపు->అంకె_Pieces = అంకె_Pieces = వెడల్పు_గుణకం;

  // Recompute the two nodes' size.
  New_కణుపు->నిండిన_Recompute_కొలత_Locally();
  నిండిన_Recompute_కొలత_Locally();

  // Update the list of leaves.
  New_కణుపు->చొప్పించు_After_ఆకు_లోపల_క్రమం(ఇది);

  // These insertions can't fail.
  ఒకవేళ (ఇది->కొలత_2() >= స్థానభ్రంశం)
    ఇది->చొప్పించు(స్థానభ్రంశం, R);
  లేకపోతే
    New_కణుపు->చొప్పించు(స్థానభ్రంశం - ఇది->కొలత_2(), R);
  తిరిగి New_కణుపు;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
శూన్య Rope_Piece_BTree_ఆకు::erase(గుర్తులేని స్థానభ్రంశం, గుర్తులేని అంకె_బైట్లు) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  గుర్తులేని PieceOffs = 0;
  గుర్తులేని i = 0;
  కొరకు (; స్థానభ్రంశం > PieceOffs; ++i)
    PieceOffs += getPiece(i).కొలత_2();
  ప్రతిజ్ఞా(PieceOffs == స్థానభ్రంశం && "Split didn't occur before erase!");

  గుర్తులేని ప్రారంభం_Piece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  కొరకు (; స్థానభ్రంశం + అంకె_బైట్లు > PieceOffs + getPiece(i).కొలత_2(); ++i)
    PieceOffs += getPiece(i).కొలత_2();

  // If we exactly include the last one, include it in the region to delete.
  ఒకవేళ (స్థానభ్రంశం + అంకె_బైట్లు == PieceOffs + getPiece(i).కొలత_2()) {
    PieceOffs += getPiece(i).కొలత_2();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  ఒకవేళ (i != ప్రారంభం_Piece) {
    గుర్తులేని అంకె_Deleted = i - ప్రారంభం_Piece;
    కొరకు (; i != get_అంకె_Pieces(); ++i)
      Pieces[i - అంకె_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_అంకె_Pieces() - అంకె_Deleted], &Pieces[get_అంకె_Pieces()],
              RopePiece());
    అంకె_Pieces -= అంకె_Deleted;

    గుర్తులేని Cover_బైట్లు = PieceOffs - స్థానభ్రంశం;
    అంకె_బైట్లు -= Cover_బైట్లు;
    కొలత -= Cover_బైట్లు;
  }

  // If we completely removed some stuff, we could be done.
  ఒకవేళ (అంకె_బైట్లు == 0)
    తిరిగి;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  ప్రతిజ్ఞా(getPiece(ప్రారంభం_Piece).కొలత_2() > అంకె_బైట్లు);
  Pieces[ప్రారంభం_Piece].ప్రారంభం_Offs += అంకె_బైట్లు;

  // The size of this node just shrunk by NumBytes.
  కొలత -= అంకె_బైట్లు;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

నామాకాశ {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
తరగతి RopePieceBTreeInterior : బహిరంగ Rope_Piece_BTree_కణుపు {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  గుర్తులేని అక్షరం అంకె_Children = 0;

  Rope_Piece_BTree_కణుపు *Children[2 * వెడల్పు_గుణకం];

బహిరంగ:
  RopePieceBTreeInterior() : Rope_Piece_BTree_కణుపు(అబద్ధం) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_కణుపు *LHS, Rope_Piece_BTree_కణుపు *RHS)
      : Rope_Piece_BTree_కణుపు(అబద్ధం) {
    Children[0] = LHS;
    Children[1] = RHS;
    అంకె_Children = 2;
    కొలత = LHS->కొలత_2() + RHS->కొలత_2();
  }

  ~RopePieceBTreeInterior() {
    కొరకు (గుర్తులేని i = 0, e = get_అంకె_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  తార్కిక is_నిండిన() స్థిరరాశి { తిరిగి అంకె_Children == 2 * వెడల్పు_గుణకం; }

  గుర్తులేని get_అంకె_Children() స్థిరరాశి { తిరిగి అంకె_Children; }

  స్థిరరాశి Rope_Piece_BTree_కణుపు *get_బిడ్డ(గుర్తులేని i) స్థిరరాశి {
    ప్రతిజ్ఞా(i < అంకె_Children && "invalid child #");
    తిరిగి Children[i];
  }

  Rope_Piece_BTree_కణుపు *get_బిడ్డ(గుర్తులేని i) {
    ప్రతిజ్ఞా(i < అంకె_Children && "invalid child #");
    తిరిగి Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  శూన్య నిండిన_Recompute_కొలత_Locally() {
    కొలత = 0;
    కొరకు (గుర్తులేని i = 0, e = get_అంకె_Children(); i != e; ++i)
      కొలత += get_బిడ్డ(i)->కొలత_2();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_కణుపు *విభజించు(గుర్తులేని స్థానభ్రంశం);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_కణుపు *చొప్పించు(గుర్తులేని స్థానభ్రంశం, స్థిరరాశి RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_కణుపు *Handle_బిడ్డ_Piece(గుర్తులేని i, Rope_Piece_BTree_కణుపు *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  శూన్య erase(గుర్తులేని స్థానభ్రంశం, గుర్తులేని అంకె_బైట్లు);

  స్థిర తార్కిక classof(స్థిరరాశి Rope_Piece_BTree_కణుపు *N) { తిరిగి !N->is_ఆకు(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_కణుపు *RopePieceBTreeInterior::విభజించు(గుర్తులేని స్థానభ్రంశం) {
  // Figure out which child to split.
  ఒకవేళ (స్థానభ్రంశం == 0 || స్థానభ్రంశం == కొలత_2())
    తిరిగి శూన్యసూచిక; // If we have an exact offset, we're already split.

  గుర్తులేని బిడ్డ_స్థానభ్రంశం = 0;
  గుర్తులేని i = 0;
  కొరకు (; స్థానభ్రంశం >= బిడ్డ_స్థానభ్రంశం + get_బిడ్డ(i)->కొలత_2(); ++i)
    బిడ్డ_స్థానభ్రంశం += get_బిడ్డ(i)->కొలత_2();

  // If already split there, we're done.
  ఒకవేళ (బిడ్డ_స్థానభ్రంశం == స్థానభ్రంశం)
    తిరిగి శూన్యసూచిక;

  // Otherwise, recursively split the child.
  ఒకవేళ (Rope_Piece_BTree_కణుపు *RHS = get_బిడ్డ(i)->విభజించు(స్థానభ్రంశం - బిడ్డ_స్థానభ్రంశం))
    తిరిగి Handle_బిడ్డ_Piece(i, RHS);
  తిరిగి శూన్యసూచిక; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_కణుపు *RopePieceBTreeInterior::చొప్పించు(గుర్తులేని స్థానభ్రంశం,
                                                   స్థిరరాశి RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  గుర్తులేని i = 0, e = get_అంకె_Children();

  గుర్తులేని బిడ్డ_Offs = 0;
  ఒకవేళ (స్థానభ్రంశం == కొలత_2()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    బిడ్డ_Offs = కొలత_2() - get_బిడ్డ(i)->కొలత_2();
  } లేకపోతే {
    కొరకు (; స్థానభ్రంశం > బిడ్డ_Offs + get_బిడ్డ(i)->కొలత_2(); ++i)
      బిడ్డ_Offs += get_బిడ్డ(i)->కొలత_2();
  }

  కొలత += R.కొలత_2();

  // Insert at the end of this child.
  ఒకవేళ (Rope_Piece_BTree_కణుపు *RHS = get_బిడ్డ(i)->చొప్పించు(స్థానభ్రంశం - బిడ్డ_Offs, R))
    తిరిగి Handle_బిడ్డ_Piece(i, RHS);

  తిరిగి శూన్యసూచిక;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_కణుపు *
RopePieceBTreeInterior::Handle_బిడ్డ_Piece(గుర్తులేని i, Rope_Piece_BTree_కణుపు *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  ఒకవేళ (!is_నిండిన()) {
    // Insert RHS after child 'i'.
    ఒకవేళ (i + 1 != get_అంకె_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_అంకె_Children() - i - 1) * పరిమాణం(Children[0]));
    Children[i + 1] = RHS;
    ++అంకె_Children;
    తిరిగి శూన్యసూచిక;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_కణుపు = కొత్త RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  జ్ఞాపకం_నకలు(&New_కణుపు->Children[0], &Children[వెడల్పు_గుణకం],
         వెడల్పు_గుణకం * పరిమాణం(Children[0]));

  // Decrease the number of values in the two nodes.
  New_కణుపు->అంకె_Children = అంకె_Children = వెడల్పు_గుణకం;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  ఒకవేళ (i < వెడల్పు_గుణకం)
    ఇది->Handle_బిడ్డ_Piece(i, RHS);
  లేకపోతే
    New_కణుపు->Handle_బిడ్డ_Piece(i - వెడల్పు_గుణకం, RHS);

  // Recompute the two nodes' size.
  New_కణుపు->నిండిన_Recompute_కొలత_Locally();
  నిండిన_Recompute_కొలత_Locally();
  తిరిగి New_కణుపు;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
శూన్య RopePieceBTreeInterior::erase(గుర్తులేని స్థానభ్రంశం, గుర్తులేని అంకె_బైట్లు) {
  // This will shrink this node by NumBytes.
  కొలత -= అంకె_బైట్లు;

  // Find the first child that overlaps with Offset.
  గుర్తులేని i = 0;
  కొరకు (; స్థానభ్రంశం >= get_బిడ్డ(i)->కొలత_2(); ++i)
    స్థానభ్రంశం -= get_బిడ్డ(i)->కొలత_2();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  అయితే (అంకె_బైట్లు) {
    Rope_Piece_BTree_కణుపు *Cur_బిడ్డ = get_బిడ్డ(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    ఒకవేళ (స్థానభ్రంశం + అంకె_బైట్లు < Cur_బిడ్డ->కొలత_2()) {
      Cur_బిడ్డ->erase(స్థానభ్రంశం, అంకె_బైట్లు);
      తిరిగి;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    ఒకవేళ (స్థానభ్రంశం) {
      గుర్తులేని బైట్లు_From_బిడ్డ = Cur_బిడ్డ->కొలత_2() - స్థానభ్రంశం;
      Cur_బిడ్డ->erase(స్థానభ్రంశం, బైట్లు_From_బిడ్డ);
      అంకె_బైట్లు -= బైట్లు_From_బిడ్డ;
      // Start at the beginning of the next child.
      స్థానభ్రంశం = 0;
      ++i;
      కొనసాగు;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    అంకె_బైట్లు -= Cur_బిడ్డ->కొలత_2();
    Cur_బిడ్డ->Destroy();
    --అంకె_Children;
    ఒకవేళ (i != get_అంకె_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_అంకె_Children() - i) * పరిమాణం(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

శూన్య Rope_Piece_BTree_కణుపు::Destroy() {
  ఒకవేళ (స్వయంచాలక *ఆకు = dyn_cast<Rope_Piece_BTree_ఆకు>(ఇది))
    తొలగించు ఆకు;
  లేకపోతే
    తొలగించు cast<RopePieceBTreeInterior>(ఇది);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_కణుపు *Rope_Piece_BTree_కణుపు::విభజించు(గుర్తులేని స్థానభ్రంశం) {
  ప్రతిజ్ఞా(స్థానభ్రంశం <= కొలత_2() && "Invalid offset to split!");
  ఒకవేళ (స్వయంచాలక *ఆకు = dyn_cast<Rope_Piece_BTree_ఆకు>(ఇది))
    తిరిగి ఆకు->విభజించు(స్థానభ్రంశం);
  తిరిగి cast<RopePieceBTreeInterior>(ఇది)->విభజించు(స్థానభ్రంశం);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_కణుపు *Rope_Piece_BTree_కణుపు::చొప్పించు(గుర్తులేని స్థానభ్రంశం,
                                               స్థిరరాశి RopePiece &R) {
  ప్రతిజ్ఞా(స్థానభ్రంశం <= కొలత_2() && "Invalid offset to insert!");
  ఒకవేళ (స్వయంచాలక *ఆకు = dyn_cast<Rope_Piece_BTree_ఆకు>(ఇది))
    తిరిగి ఆకు->చొప్పించు(స్థానభ్రంశం, R);
  తిరిగి cast<RopePieceBTreeInterior>(ఇది)->చొప్పించు(స్థానభ్రంశం, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
శూన్య Rope_Piece_BTree_కణుపు::erase(గుర్తులేని స్థానభ్రంశం, గుర్తులేని అంకె_బైట్లు) {
  ప్రతిజ్ఞా(స్థానభ్రంశం + అంకె_బైట్లు <= కొలత_2() && "Invalid offset to erase!");
  ఒకవేళ (స్వయంచాలక *ఆకు = dyn_cast<Rope_Piece_BTree_ఆకు>(ఇది))
    తిరిగి ఆకు->erase(స్థానభ్రంశం, అంకె_బైట్లు);
  తిరిగి cast<RopePieceBTreeInterior>(ఇది)->erase(స్థానభ్రంశం, అంకె_బైట్లు);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

స్థిర స్థిరరాశి Rope_Piece_BTree_ఆకు *getCN(స్థిరరాశి శూన్య *P) {
  తిరిగి static_cast<స్థిరరాశి Rope_Piece_BTree_ఆకు *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(స్థిరరాశి శూన్య *n) {
  స్థిరరాశి స్వయంచాలక *N = static_cast<స్థిరరాశి Rope_Piece_BTree_కణుపు *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  అయితే (స్థిరరాశి స్వయంచాలక *లోపల = dyn_cast<RopePieceBTreeInterior>(N))
    N = లోపల->get_బిడ్డ(0);

  // We must have at least one leaf.
  Cur_కణుపు = cast<Rope_Piece_BTree_ఆకు>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  అయితే (Cur_కణుపు && getCN(Cur_కణుపు)->get_అంకె_Pieces() == 0)
    Cur_కణుపు = getCN(Cur_కణుపు)->get_తదుపరి_ఆకు_లోపల_క్రమం();

  ఒకవేళ (Cur_కణుపు)
    CurPiece = &getCN(Cur_కణుపు)->getPiece(0);
  లేకపోతే // Empty tree, this is an end() iterator.
    CurPiece = శూన్యసూచిక;
  CurChar = 0;
}

శూన్య RopePieceBTreeIterator::Move_గమ్యం_తదుపరి_Piece() {
  ఒకవేళ (CurPiece !=
      &getCN(Cur_కణుపు)->getPiece(getCN(Cur_కణుపు)->get_అంకె_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    తిరిగి;
  }

  // Find the next non-empty leaf node.
  చేయి
    Cur_కణుపు = getCN(Cur_కణుపు)->get_తదుపరి_ఆకు_లోపల_క్రమం();
  అయితే (Cur_కణుపు && getCN(Cur_కణుపు)->get_అంకె_Pieces() == 0);

  ఒకవేళ (Cur_కణుపు)
    CurPiece = &getCN(Cur_కణుపు)->getPiece(0);
  లేకపోతే // Hit end().
    CurPiece = శూన్యసూచిక;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

స్థిర Rope_Piece_BTree_కణుపు *get_మూలం(శూన్య *P) {
  తిరిగి static_cast<Rope_Piece_BTree_కణుపు *>(P);
}

RopePieceBTree::RopePieceBTree() { మూలం_2 = కొత్త Rope_Piece_BTree_ఆకు(); }

RopePieceBTree::RopePieceBTree(స్థిరరాశి RopePieceBTree &RHS) {
  ప్రతిజ్ఞా(RHS.ఖాళీలు() && "Can't copy non-empty tree yet");
  మూలం_2 = కొత్త Rope_Piece_BTree_ఆకు();
}

RopePieceBTree::~RopePieceBTree() { get_మూలం(మూలం_2)->Destroy(); }

గుర్తులేని RopePieceBTree::కొలత_2() స్థిరరాశి { తిరిగి get_మూలం(మూలం_2)->కొలత_2(); }

శూన్య RopePieceBTree::clear() {
  ఒకవేళ (స్వయంచాలక *ఆకు = dyn_cast<Rope_Piece_BTree_ఆకు>(get_మూలం(మూలం_2)))
    ఆకు->clear();
  లేకపోతే {
    get_మూలం(మూలం_2)->Destroy();
    మూలం_2 = కొత్త Rope_Piece_BTree_ఆకు();
  }
}

శూన్య RopePieceBTree::చొప్పించు(గుర్తులేని స్థానభ్రంశం, స్థిరరాశి RopePiece &R) {
  // #1. Split at Offset.
  ఒకవేళ (Rope_Piece_BTree_కణుపు *RHS = get_మూలం(మూలం_2)->విభజించు(స్థానభ్రంశం))
    మూలం_2 = కొత్త RopePieceBTreeInterior(get_మూలం(మూలం_2), RHS);

  // #2. Do the insertion.
  ఒకవేళ (Rope_Piece_BTree_కణుపు *RHS = get_మూలం(మూలం_2)->చొప్పించు(స్థానభ్రంశం, R))
    మూలం_2 = కొత్త RopePieceBTreeInterior(get_మూలం(మూలం_2), RHS);
}

శూన్య RopePieceBTree::erase(గుర్తులేని స్థానభ్రంశం, గుర్తులేని అంకె_బైట్లు) {
  // #1. Split at Offset.
  ఒకవేళ (Rope_Piece_BTree_కణుపు *RHS = get_మూలం(మూలం_2)->విభజించు(స్థానభ్రంశం))
    మూలం_2 = కొత్త RopePieceBTreeInterior(get_మూలం(మూలం_2), RHS);

  // #2. Do the erasing.
  get_మూలం(మూలం_2)->erase(స్థానభ్రంశం, అంకె_బైట్లు);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(స్థిరరాశి అక్షరం *ప్రారంభం, స్థిరరాశి అక్షరం *చివర_2) {
  గుర్తులేని Len = చివర_2 - ప్రారంభం;
  ప్రతిజ్ఞా(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  ఒకవేళ (AllocOffs + Len <= Alloc_Chunk_కొలత) {
    జ్ఞాపకం_నకలు(Alloc_బఫరు->దత్తాంశం_2 + AllocOffs, ప్రారంభం, Len);
    AllocOffs += Len;
    తిరిగి RopePiece(Alloc_బఫరు, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  ఒకవేళ (Len > Alloc_Chunk_కొలత) {
    గుర్తులేని కొలత = చివర_2 - ప్రారంభం + పరిమాణం(Rope_Ref_లెక్క_String) - 1;
    స్వయంచాలక *Res = reinterpret_cast<Rope_Ref_లెక్క_String *>(కొత్త అక్షరం[కొలత]);
    Res->Ref_లెక్క = 0;
    జ్ఞాపకం_నకలు(Res->దత్తాంశం_2, ప్రారంభం, చివర_2 - ప్రారంభం);
    తిరిగి RopePiece(Res, 0, చివర_2 - ప్రారంభం);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  గుర్తులేని Alloc_కొలత = offsetof(Rope_Ref_లెక్క_String, దత్తాంశం_2) + Alloc_Chunk_కొలత;
  స్వయంచాలక *Res = reinterpret_cast<Rope_Ref_లెక్క_String *>(కొత్త అక్షరం[Alloc_కొలత]);
  Res->Ref_లెక్క = 0;
  జ్ఞాపకం_నకలు(Res->దత్తాంశం_2, ప్రారంభం, Len);
  Alloc_బఫరు = Res;
  AllocOffs = Len;

  తిరిగి RopePiece(Alloc_బఫరు, 0, Len);
}
