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

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/RewriteRope.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Casting.h"
#ଅନ୍ତର୍ଭାଵ <algorithm>
#ଅନ୍ତର୍ଭାଵ <cassert>
#ଅନ୍ତର୍ଭାଵ <cstring>

using ନାମାକାଶ llvm;

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

ନାମାକାଶ {

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Class
//===----------------------------------------------------------------------===//

/// RopePieceBTreeNode - Common base class of RopePieceBTreeLeaf and
/// RopePieceBTreeInterior.  This provides some 'virtual' dispatching methods
/// and a flag that determines which subclass the instance is.  Also
/// important, this node knows the full extend of the node, including any
/// children that it has.  This allows efficient skipping over entire subtrees
/// when looking for an offset in the BTree.
ଶ୍ରେଣୀ Rope_Piece_BTree_ଗଣ୍ଠି {
ସୁରକ୍ଷିତ:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  ଗଣନା { WidthFactor = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  unsigned Size = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  ତାର୍କିକ IsLeaf;

  Rope_Piece_BTree_ଗଣ୍ଠି(ତାର୍କିକ isLeaf) : IsLeaf(isLeaf) {}
  ~Rope_Piece_BTree_ଗଣ୍ଠି() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

ସର୍ବସାଧାରଣ:
  ତାର୍କିକ isLeaf() ଅଚଳ { ଫେରନ୍ତୁ IsLeaf; }
  unsigned size() ଅଚଳ { ଫେରନ୍ତୁ Size; }

  ଶୂନ୍ୟ Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ଗଣ୍ଠି *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ଗଣ୍ଠି *ଭର୍ତ୍ତି(unsigned Offset, ଅଚଳ RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  ଶୂନ୍ୟ erase(unsigned Offset, unsigned ଅଙ୍କ_Bytes);
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
ଶ୍ରେଣୀ RopePieceBTreeLeaf : ସର୍ବସାଧାରଣ Rope_Piece_BTree_ଗଣ୍ଠି {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  unsigned ଅକ୍ଷର ଅଙ୍କ_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * WidthFactor];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  RopePieceBTreeLeaf **PrevLeaf = nullptr;
  RopePieceBTreeLeaf *NextLeaf = nullptr;

ସର୍ବସାଧାରଣ:
  RopePieceBTreeLeaf() : Rope_Piece_BTree_ଗଣ୍ଠି(ସତ୍ୟ) {}

  ~RopePieceBTreeLeaf() {
    ଯଦି (PrevLeaf || NextLeaf)
      removeFromLeafInOrder();
    clear();
  }

  ତାର୍କିକ isFull() ଅଚଳ { ଫେରନ୍ତୁ ଅଙ୍କ_Pieces == 2 * WidthFactor; }

  /// clear - Remove all rope pieces from this leaf.
  ଶୂନ୍ୟ clear() {
    ଯେତେବେଳେ (ଅଙ୍କ_Pieces)
      Pieces[--ଅଙ୍କ_Pieces] = RopePiece();
    Size = 0;
  }

  unsigned get_ଅଙ୍କ_Pieces() ଅଚଳ { ଫେରନ୍ତୁ ଅଙ୍କ_Pieces; }

  ଅଚଳ RopePiece &getPiece(unsigned i) ଅଚଳ {
    ପ୍ରତିଜ୍ଞା(i < get_ଅଙ୍କ_Pieces() && "Invalid piece ID");
    ଫେରନ୍ତୁ Pieces[i];
  }

  ଅଚଳ RopePieceBTreeLeaf *getNextLeafInOrder() ଅଚଳ { ଫେରନ୍ତୁ NextLeaf; }

  ଶୂନ୍ୟ ଭର୍ତ୍ତି_After_Leaf_In_Order(RopePieceBTreeLeaf *ଗଣ୍ଠି) {
    ପ୍ରତିଜ୍ଞା(!PrevLeaf && !NextLeaf && "Already in ordering");

    NextLeaf = ଗଣ୍ଠି->NextLeaf;
    ଯଦି (NextLeaf)
      NextLeaf->PrevLeaf = &NextLeaf;
    PrevLeaf = &ଗଣ୍ଠି->NextLeaf;
    ଗଣ୍ଠି->NextLeaf = ଏହି;
  }

  ଶୂନ୍ୟ removeFromLeafInOrder() {
    ଯଦି (PrevLeaf) {
      *PrevLeaf = NextLeaf;
      ଯଦି (NextLeaf)
        NextLeaf->PrevLeaf = PrevLeaf;
    } ଅନ୍ୟଥା ଯଦି (NextLeaf) {
      NextLeaf->PrevLeaf = nullptr;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  ଶୂନ୍ୟ FullRecomputeSizeLocally() {
    Size = 0;
    ପାଇଁ (unsigned i = 0, e = get_ଅଙ୍କ_Pieces(); i != e; ++i)
      Size += getPiece(i).size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ଗଣ୍ଠି *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ଗଣ୍ଠି *ଭର୍ତ୍ତି(unsigned Offset, ଅଚଳ RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  ଶୂନ୍ୟ erase(unsigned Offset, unsigned ଅଙ୍କ_Bytes);

  ସ୍ଥିର ତାର୍କିକ classof(ଅଚଳ Rope_Piece_BTree_ଗଣ୍ଠି *N) { ଫେରନ୍ତୁ N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ଗଣ୍ଠି *RopePieceBTreeLeaf::split(unsigned Offset) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  ଯଦି (Offset == 0 || Offset == size()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    ଫେରନ୍ତୁ nullptr;
  }

  // Find the piece that this offset lands in.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  ଯେତେବେଳେ (Offset >= PieceOffs + Pieces[i].size()) {
    PieceOffs += Pieces[i].size();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  ଯଦି (PieceOffs == Offset)
    ଫେରନ୍ତୁ nullptr;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  unsigned IntraPieceOffset = Offset - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece Tail(Pieces[i].StrData, Pieces[i].StartOffs + IntraPieceOffset,
                 Pieces[i].EndOffs);
  Size -= Pieces[i].size();
  Pieces[i].EndOffs = Pieces[i].StartOffs + IntraPieceOffset;
  Size += Pieces[i].size();

  ଫେରନ୍ତୁ ଭର୍ତ୍ତି(Offset, Tail);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ଗଣ୍ଠି *RopePieceBTreeLeaf::ଭର୍ତ୍ତି(unsigned Offset,
                                               ଅଚଳ RopePiece &R) {
  // If this node is not full, insert the piece.
  ଯଦି (!isFull()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    unsigned i = 0, e = get_ଅଙ୍କ_Pieces();
    ଯଦି (Offset == size()) {
      // Fastpath for a common case.
      i = e;
    } ଅନ୍ୟଥା {
      unsigned SlotOffs = 0;
      ପାଇଁ (; Offset > SlotOffs; ++i)
        SlotOffs += getPiece(i).size();
      ପ୍ରତିଜ୍ଞା(SlotOffs == Offset && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    ପାଇଁ (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++ଅଙ୍କ_Pieces;
    Size += R.size();
    ଫେରନ୍ତୁ nullptr;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  RopePieceBTreeLeaf *New_ଗଣ୍ଠି = ନୂତନ RopePieceBTreeLeaf();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[WidthFactor], &Pieces[2 * WidthFactor],
            &New_ଗଣ୍ଠି->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[WidthFactor], &Pieces[2 * WidthFactor], RopePiece());

  // Decrease the number of values in the two nodes.
  New_ଗଣ୍ଠି->ଅଙ୍କ_Pieces = ଅଙ୍କ_Pieces = WidthFactor;

  // Recompute the two nodes' size.
  New_ଗଣ୍ଠି->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();

  // Update the list of leaves.
  New_ଗଣ୍ଠି->ଭର୍ତ୍ତି_After_Leaf_In_Order(ଏହି);

  // These insertions can't fail.
  ଯଦି (ଏହି->size() >= Offset)
    ଏହି->ଭର୍ତ୍ତି(Offset, R);
  ଅନ୍ୟଥା
    New_ଗଣ୍ଠି->ଭର୍ତ୍ତି(Offset - ଏହି->size(), R);
  ଫେରନ୍ତୁ New_ଗଣ୍ଠି;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
ଶୂନ୍ୟ RopePieceBTreeLeaf::erase(unsigned Offset, unsigned ଅଙ୍କ_Bytes) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  ପାଇଁ (; Offset > PieceOffs; ++i)
    PieceOffs += getPiece(i).size();
  ପ୍ରତିଜ୍ଞା(PieceOffs == Offset && "Split didn't occur before erase!");

  unsigned StartPiece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  ପାଇଁ (; Offset + ଅଙ୍କ_Bytes > PieceOffs + getPiece(i).size(); ++i)
    PieceOffs += getPiece(i).size();

  // If we exactly include the last one, include it in the region to delete.
  ଯଦି (Offset + ଅଙ୍କ_Bytes == PieceOffs + getPiece(i).size()) {
    PieceOffs += getPiece(i).size();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  ଯଦି (i != StartPiece) {
    unsigned ଅଙ୍କ_Deleted = i - StartPiece;
    ପାଇଁ (; i != get_ଅଙ୍କ_Pieces(); ++i)
      Pieces[i - ଅଙ୍କ_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_ଅଙ୍କ_Pieces() - ଅଙ୍କ_Deleted], &Pieces[get_ଅଙ୍କ_Pieces()],
              RopePiece());
    ଅଙ୍କ_Pieces -= ଅଙ୍କ_Deleted;

    unsigned CoverBytes = PieceOffs - Offset;
    ଅଙ୍କ_Bytes -= CoverBytes;
    Size -= CoverBytes;
  }

  // If we completely removed some stuff, we could be done.
  ଯଦି (ଅଙ୍କ_Bytes == 0)
    ଫେରନ୍ତୁ;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  ପ୍ରତିଜ୍ଞା(getPiece(StartPiece).size() > ଅଙ୍କ_Bytes);
  Pieces[StartPiece].StartOffs += ଅଙ୍କ_Bytes;

  // The size of this node just shrunk by NumBytes.
  Size -= ଅଙ୍କ_Bytes;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

ନାମାକାଶ {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
ଶ୍ରେଣୀ RopePieceBTreeInterior : ସର୍ବସାଧାରଣ Rope_Piece_BTree_ଗଣ୍ଠି {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  unsigned ଅକ୍ଷର ଅଙ୍କ_Children = 0;

  Rope_Piece_BTree_ଗଣ୍ଠି *Children[2 * WidthFactor];

ସର୍ବସାଧାରଣ:
  RopePieceBTreeInterior() : Rope_Piece_BTree_ଗଣ୍ଠି(ମିଥ୍ୟା) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_ଗଣ୍ଠି *LHS, Rope_Piece_BTree_ଗଣ୍ଠି *RHS)
      : Rope_Piece_BTree_ଗଣ୍ଠି(ମିଥ୍ୟା) {
    Children[0] = LHS;
    Children[1] = RHS;
    ଅଙ୍କ_Children = 2;
    Size = LHS->size() + RHS->size();
  }

  ~RopePieceBTreeInterior() {
    ପାଇଁ (unsigned i = 0, e = get_ଅଙ୍କ_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  ତାର୍କିକ isFull() ଅଚଳ { ଫେରନ୍ତୁ ଅଙ୍କ_Children == 2 * WidthFactor; }

  unsigned get_ଅଙ୍କ_Children() ଅଚଳ { ଫେରନ୍ତୁ ଅଙ୍କ_Children; }

  ଅଚଳ Rope_Piece_BTree_ଗଣ୍ଠି *getChild(unsigned i) ଅଚଳ {
    ପ୍ରତିଜ୍ଞା(i < ଅଙ୍କ_Children && "invalid child #");
    ଫେରନ୍ତୁ Children[i];
  }

  Rope_Piece_BTree_ଗଣ୍ଠି *getChild(unsigned i) {
    ପ୍ରତିଜ୍ଞା(i < ଅଙ୍କ_Children && "invalid child #");
    ଫେରନ୍ତୁ Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  ଶୂନ୍ୟ FullRecomputeSizeLocally() {
    Size = 0;
    ପାଇଁ (unsigned i = 0, e = get_ଅଙ୍କ_Children(); i != e; ++i)
      Size += getChild(i)->size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ଗଣ୍ଠି *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ଗଣ୍ଠି *ଭର୍ତ୍ତି(unsigned Offset, ଅଚଳ RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_ଗଣ୍ଠି *HandleChildPiece(unsigned i, Rope_Piece_BTree_ଗଣ୍ଠି *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  ଶୂନ୍ୟ erase(unsigned Offset, unsigned ଅଙ୍କ_Bytes);

  ସ୍ଥିର ତାର୍କିକ classof(ଅଚଳ Rope_Piece_BTree_ଗଣ୍ଠି *N) { ଫେରନ୍ତୁ !N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ଗଣ୍ଠି *RopePieceBTreeInterior::split(unsigned Offset) {
  // Figure out which child to split.
  ଯଦି (Offset == 0 || Offset == size())
    ଫେରନ୍ତୁ nullptr; // If we have an exact offset, we're already split.

  unsigned ChildOffset = 0;
  unsigned i = 0;
  ପାଇଁ (; Offset >= ChildOffset + getChild(i)->size(); ++i)
    ChildOffset += getChild(i)->size();

  // If already split there, we're done.
  ଯଦି (ChildOffset == Offset)
    ଫେରନ୍ତୁ nullptr;

  // Otherwise, recursively split the child.
  ଯଦି (Rope_Piece_BTree_ଗଣ୍ଠି *RHS = getChild(i)->split(Offset - ChildOffset))
    ଫେରନ୍ତୁ HandleChildPiece(i, RHS);
  ଫେରନ୍ତୁ nullptr; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ଗଣ୍ଠି *RopePieceBTreeInterior::ଭର୍ତ୍ତି(unsigned Offset,
                                                   ଅଚଳ RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  unsigned i = 0, e = get_ଅଙ୍କ_Children();

  unsigned ChildOffs = 0;
  ଯଦି (Offset == size()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    ChildOffs = size() - getChild(i)->size();
  } ଅନ୍ୟଥା {
    ପାଇଁ (; Offset > ChildOffs + getChild(i)->size(); ++i)
      ChildOffs += getChild(i)->size();
  }

  Size += R.size();

  // Insert at the end of this child.
  ଯଦି (Rope_Piece_BTree_ଗଣ୍ଠି *RHS = getChild(i)->ଭର୍ତ୍ତି(Offset - ChildOffs, R))
    ଫେରନ୍ତୁ HandleChildPiece(i, RHS);

  ଫେରନ୍ତୁ nullptr;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_ଗଣ୍ଠି *
RopePieceBTreeInterior::HandleChildPiece(unsigned i, Rope_Piece_BTree_ଗଣ୍ଠି *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  ଯଦି (!isFull()) {
    // Insert RHS after child 'i'.
    ଯଦି (i + 1 != get_ଅଙ୍କ_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_ଅଙ୍କ_Children() - i - 1) * ଆକାର(Children[0]));
    Children[i + 1] = RHS;
    ++ଅଙ୍କ_Children;
    ଫେରନ୍ତୁ nullptr;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_ଗଣ୍ଠି = ନୂତନ RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  memcpy(&New_ଗଣ୍ଠି->Children[0], &Children[WidthFactor],
         WidthFactor * ଆକାର(Children[0]));

  // Decrease the number of values in the two nodes.
  New_ଗଣ୍ଠି->ଅଙ୍କ_Children = ଅଙ୍କ_Children = WidthFactor;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  ଯଦି (i < WidthFactor)
    ଏହି->HandleChildPiece(i, RHS);
  ଅନ୍ୟଥା
    New_ଗଣ୍ଠି->HandleChildPiece(i - WidthFactor, RHS);

  // Recompute the two nodes' size.
  New_ଗଣ୍ଠି->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();
  ଫେରନ୍ତୁ New_ଗଣ୍ଠି;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
ଶୂନ୍ୟ RopePieceBTreeInterior::erase(unsigned Offset, unsigned ଅଙ୍କ_Bytes) {
  // This will shrink this node by NumBytes.
  Size -= ଅଙ୍କ_Bytes;

  // Find the first child that overlaps with Offset.
  unsigned i = 0;
  ପାଇଁ (; Offset >= getChild(i)->size(); ++i)
    Offset -= getChild(i)->size();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  ଯେତେବେଳେ (ଅଙ୍କ_Bytes) {
    Rope_Piece_BTree_ଗଣ୍ଠି *CurChild = getChild(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    ଯଦି (Offset + ଅଙ୍କ_Bytes < CurChild->size()) {
      CurChild->erase(Offset, ଅଙ୍କ_Bytes);
      ଫେରନ୍ତୁ;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    ଯଦି (Offset) {
      unsigned BytesFromChild = CurChild->size() - Offset;
      CurChild->erase(Offset, BytesFromChild);
      ଅଙ୍କ_Bytes -= BytesFromChild;
      // Start at the beginning of the next child.
      Offset = 0;
      ++i;
      ଜାରି;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    ଅଙ୍କ_Bytes -= CurChild->size();
    CurChild->Destroy();
    --ଅଙ୍କ_Children;
    ଯଦି (i != get_ଅଙ୍କ_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_ଅଙ୍କ_Children() - i) * ଆକାର(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

ଶୂନ୍ୟ Rope_Piece_BTree_ଗଣ୍ଠି::Destroy() {
  ଯଦି (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(ଏହି))
    delete Leaf;
  ଅନ୍ୟଥା
    delete cast<RopePieceBTreeInterior>(ଏହି);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ଗଣ୍ଠି *Rope_Piece_BTree_ଗଣ୍ଠି::split(unsigned Offset) {
  ପ୍ରତିଜ୍ଞା(Offset <= size() && "Invalid offset to split!");
  ଯଦି (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(ଏହି))
    ଫେରନ୍ତୁ Leaf->split(Offset);
  ଫେରନ୍ତୁ cast<RopePieceBTreeInterior>(ଏହି)->split(Offset);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ଗଣ୍ଠି *Rope_Piece_BTree_ଗଣ୍ଠି::ଭର୍ତ୍ତି(unsigned Offset,
                                               ଅଚଳ RopePiece &R) {
  ପ୍ରତିଜ୍ଞା(Offset <= size() && "Invalid offset to insert!");
  ଯଦି (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(ଏହି))
    ଫେରନ୍ତୁ Leaf->ଭର୍ତ୍ତି(Offset, R);
  ଫେରନ୍ତୁ cast<RopePieceBTreeInterior>(ଏହି)->ଭର୍ତ୍ତି(Offset, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
ଶୂନ୍ୟ Rope_Piece_BTree_ଗଣ୍ଠି::erase(unsigned Offset, unsigned ଅଙ୍କ_Bytes) {
  ପ୍ରତିଜ୍ଞା(Offset + ଅଙ୍କ_Bytes <= size() && "Invalid offset to erase!");
  ଯଦି (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(ଏହି))
    ଫେରନ୍ତୁ Leaf->erase(Offset, ଅଙ୍କ_Bytes);
  ଫେରନ୍ତୁ cast<RopePieceBTreeInterior>(ଏହି)->erase(Offset, ଅଙ୍କ_Bytes);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

ସ୍ଥିର ଅଚଳ RopePieceBTreeLeaf *getCN(ଅଚଳ ଶୂନ୍ୟ *P) {
  ଫେରନ୍ତୁ static_cast<ଅଚଳ RopePieceBTreeLeaf *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(ଅଚଳ ଶୂନ୍ୟ *n) {
  ଅଚଳ auto *N = static_cast<ଅଚଳ Rope_Piece_BTree_ଗଣ୍ଠି *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  ଯେତେବେଳେ (ଅଚଳ auto *IN = dyn_cast<RopePieceBTreeInterior>(N))
    N = IN->getChild(0);

  // We must have at least one leaf.
  Cur_ଗଣ୍ଠି = cast<RopePieceBTreeLeaf>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  ଯେତେବେଳେ (Cur_ଗଣ୍ଠି && getCN(Cur_ଗଣ୍ଠି)->get_ଅଙ୍କ_Pieces() == 0)
    Cur_ଗଣ୍ଠି = getCN(Cur_ଗଣ୍ଠି)->getNextLeafInOrder();

  ଯଦି (Cur_ଗଣ୍ଠି)
    CurPiece = &getCN(Cur_ଗଣ୍ଠି)->getPiece(0);
  ଅନ୍ୟଥା // Empty tree, this is an end() iterator.
    CurPiece = nullptr;
  CurChar = 0;
}

ଶୂନ୍ୟ RopePieceBTreeIterator::MoveToNextPiece() {
  ଯଦି (CurPiece !=
      &getCN(Cur_ଗଣ୍ଠି)->getPiece(getCN(Cur_ଗଣ୍ଠି)->get_ଅଙ୍କ_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    ଫେରନ୍ତୁ;
  }

  // Find the next non-empty leaf node.
  କର
    Cur_ଗଣ୍ଠି = getCN(Cur_ଗଣ୍ଠି)->getNextLeafInOrder();
  ଯେତେବେଳେ (Cur_ଗଣ୍ଠି && getCN(Cur_ଗଣ୍ଠି)->get_ଅଙ୍କ_Pieces() == 0);

  ଯଦି (Cur_ଗଣ୍ଠି)
    CurPiece = &getCN(Cur_ଗଣ୍ଠି)->getPiece(0);
  ଅନ୍ୟଥା // Hit end().
    CurPiece = nullptr;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

ସ୍ଥିର Rope_Piece_BTree_ଗଣ୍ଠି *get_ମୂଳ(ଶୂନ୍ୟ *P) {
  ଫେରନ୍ତୁ static_cast<Rope_Piece_BTree_ଗଣ୍ଠି *>(P);
}

RopePieceBTree::RopePieceBTree() { ମୂଳ_2 = ନୂତନ RopePieceBTreeLeaf(); }

RopePieceBTree::RopePieceBTree(ଅଚଳ RopePieceBTree &RHS) {
  ପ୍ରତିଜ୍ଞା(RHS.empty() && "Can't copy non-empty tree yet");
  ମୂଳ_2 = ନୂତନ RopePieceBTreeLeaf();
}

RopePieceBTree::~RopePieceBTree() { get_ମୂଳ(ମୂଳ_2)->Destroy(); }

unsigned RopePieceBTree::size() ଅଚଳ { ଫେରନ୍ତୁ get_ମୂଳ(ମୂଳ_2)->size(); }

ଶୂନ୍ୟ RopePieceBTree::clear() {
  ଯଦି (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(get_ମୂଳ(ମୂଳ_2)))
    Leaf->clear();
  ଅନ୍ୟଥା {
    get_ମୂଳ(ମୂଳ_2)->Destroy();
    ମୂଳ_2 = ନୂତନ RopePieceBTreeLeaf();
  }
}

ଶୂନ୍ୟ RopePieceBTree::ଭର୍ତ୍ତି(unsigned Offset, ଅଚଳ RopePiece &R) {
  // #1. Split at Offset.
  ଯଦି (Rope_Piece_BTree_ଗଣ୍ଠି *RHS = get_ମୂଳ(ମୂଳ_2)->split(Offset))
    ମୂଳ_2 = ନୂତନ RopePieceBTreeInterior(get_ମୂଳ(ମୂଳ_2), RHS);

  // #2. Do the insertion.
  ଯଦି (Rope_Piece_BTree_ଗଣ୍ଠି *RHS = get_ମୂଳ(ମୂଳ_2)->ଭର୍ତ୍ତି(Offset, R))
    ମୂଳ_2 = ନୂତନ RopePieceBTreeInterior(get_ମୂଳ(ମୂଳ_2), RHS);
}

ଶୂନ୍ୟ RopePieceBTree::erase(unsigned Offset, unsigned ଅଙ୍କ_Bytes) {
  // #1. Split at Offset.
  ଯଦି (Rope_Piece_BTree_ଗଣ୍ଠି *RHS = get_ମୂଳ(ମୂଳ_2)->split(Offset))
    ମୂଳ_2 = ନୂତନ RopePieceBTreeInterior(get_ମୂଳ(ମୂଳ_2), RHS);

  // #2. Do the erasing.
  get_ମୂଳ(ମୂଳ_2)->erase(Offset, ଅଙ୍କ_Bytes);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(ଅଚଳ ଅକ୍ଷର *Start, ଅଚଳ ଅକ୍ଷର *End) {
  unsigned Len = End - Start;
  ପ୍ରତିଜ୍ଞା(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  ଯଦି (AllocOffs + Len <= AllocChunkSize) {
    memcpy(AllocBuffer->Data + AllocOffs, Start, Len);
    AllocOffs += Len;
    ଫେରନ୍ତୁ RopePiece(AllocBuffer, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  ଯଦି (Len > AllocChunkSize) {
    unsigned Size = End - Start + ଆକାର(Rope_Ref_ଗଣନା_String) - 1;
    auto *Res = reinterpret_cast<Rope_Ref_ଗଣନା_String *>(ନୂତନ ଅକ୍ଷର[Size]);
    Res->Ref_ଗଣନା = 0;
    memcpy(Res->Data, Start, End - Start);
    ଫେରନ୍ତୁ RopePiece(Res, 0, End - Start);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  unsigned AllocSize = offsetof(Rope_Ref_ଗଣନା_String, Data) + AllocChunkSize;
  auto *Res = reinterpret_cast<Rope_Ref_ଗଣନା_String *>(ନୂତନ ଅକ୍ଷର[AllocSize]);
  Res->Ref_ଗଣନା = 0;
  memcpy(Res->Data, Start, Len);
  AllocBuffer = Res;
  AllocOffs = Len;

  ଫେରନ୍ତୁ RopePiece(AllocBuffer, 0, Len);
}
