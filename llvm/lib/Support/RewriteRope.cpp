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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/RewriteRope.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Casting.h"
#ಅನ್ತರ್ಭಾವ <algorithm>
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <cstring>

using ನಾಮಾಕಾಶ llvm;

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

ನಾಮಾಕಾಶ {

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Class
//===----------------------------------------------------------------------===//

/// RopePieceBTreeNode - Common base class of RopePieceBTreeLeaf and
/// RopePieceBTreeInterior.  This provides some 'virtual' dispatching methods
/// and a flag that determines which subclass the instance is.  Also
/// important, this node knows the full extend of the node, including any
/// children that it has.  This allows efficient skipping over entire subtrees
/// when looking for an offset in the BTree.
ವರ್ಗ Rope_Piece_BTree_ಗಂಟು {
ರಕ್ಷಿತ:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  ಗಣನಾ { WidthFactor = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  unsigned Size = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  ತಾರ್ಕಿಕ IsLeaf;

  Rope_Piece_BTree_ಗಂಟು(ತಾರ್ಕಿಕ isLeaf) : IsLeaf(isLeaf) {}
  ~Rope_Piece_BTree_ಗಂಟು() = ಸ್ವಭಾವ;

ಸಾರ್ವಜನಿಕ:
  ತಾರ್ಕಿಕ isLeaf() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ IsLeaf; }
  unsigned size() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Size; }

  ಶೂನ್ಯ Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ಗಂಟು *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ಗಂಟು *ಸೇರಿಸು(unsigned Offset, ಧ್ರುವ RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  ಶೂನ್ಯ erase(unsigned Offset, unsigned ಅಂಕೆ_Bytes);
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
ವರ್ಗ RopePieceBTreeLeaf : ಸಾರ್ವಜನಿಕ Rope_Piece_BTree_ಗಂಟು {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  unsigned ವರ್ಣ ಅಂಕೆ_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * WidthFactor];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  RopePieceBTreeLeaf **PrevLeaf = nullptr;
  RopePieceBTreeLeaf *NextLeaf = nullptr;

ಸಾರ್ವಜನಿಕ:
  RopePieceBTreeLeaf() : Rope_Piece_BTree_ಗಂಟು(ಸತ್ಯ) {}

  ~RopePieceBTreeLeaf() {
    ಯದಿ (PrevLeaf || NextLeaf)
      removeFromLeafInOrder();
    clear();
  }

  ತಾರ್ಕಿಕ isFull() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ_Pieces == 2 * WidthFactor; }

  /// clear - Remove all rope pieces from this leaf.
  ಶೂನ್ಯ clear() {
    ಯಾವತ್ (ಅಂಕೆ_Pieces)
      Pieces[--ಅಂಕೆ_Pieces] = RopePiece();
    Size = 0;
  }

  unsigned get_ಅಂಕೆ_Pieces() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ_Pieces; }

  ಧ್ರುವ RopePiece &getPiece(unsigned i) ಧ್ರುವ {
    ಪ್ರತಿಜ್ಞಾ(i < get_ಅಂಕೆ_Pieces() && "Invalid piece ID");
    ಪ್ರತ್ಯಾಗಮ Pieces[i];
  }

  ಧ್ರುವ RopePieceBTreeLeaf *getNextLeafInOrder() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ NextLeaf; }

  ಶೂನ್ಯ ಸೇರಿಸು_After_Leaf_In_Order(RopePieceBTreeLeaf *ಗಂಟು) {
    ಪ್ರತಿಜ್ಞಾ(!PrevLeaf && !NextLeaf && "Already in ordering");

    NextLeaf = ಗಂಟು->NextLeaf;
    ಯದಿ (NextLeaf)
      NextLeaf->PrevLeaf = &NextLeaf;
    PrevLeaf = &ಗಂಟು->NextLeaf;
    ಗಂಟು->NextLeaf = ಏತತ್;
  }

  ಶೂನ್ಯ removeFromLeafInOrder() {
    ಯದಿ (PrevLeaf) {
      *PrevLeaf = NextLeaf;
      ಯದಿ (NextLeaf)
        NextLeaf->PrevLeaf = PrevLeaf;
    } ಅನ್ಯಥಾ ಯದಿ (NextLeaf) {
      NextLeaf->PrevLeaf = nullptr;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  ಶೂನ್ಯ FullRecomputeSizeLocally() {
    Size = 0;
    ಪ್ರತಿ (unsigned i = 0, e = get_ಅಂಕೆ_Pieces(); i != e; ++i)
      Size += getPiece(i).size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ಗಂಟು *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ಗಂಟು *ಸೇರಿಸು(unsigned Offset, ಧ್ರುವ RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  ಶೂನ್ಯ erase(unsigned Offset, unsigned ಅಂಕೆ_Bytes);

  ಸ್ಥಿರ ತಾರ್ಕಿಕ classof(ಧ್ರುವ Rope_Piece_BTree_ಗಂಟು *N) { ಪ್ರತ್ಯಾಗಮ N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ಗಂಟು *RopePieceBTreeLeaf::split(unsigned Offset) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  ಯದಿ (Offset == 0 || Offset == size()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    ಪ್ರತ್ಯಾಗಮ nullptr;
  }

  // Find the piece that this offset lands in.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  ಯಾವತ್ (Offset >= PieceOffs + Pieces[i].size()) {
    PieceOffs += Pieces[i].size();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  ಯದಿ (PieceOffs == Offset)
    ಪ್ರತ್ಯಾಗಮ nullptr;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  unsigned IntraPieceOffset = Offset - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece Tail(Pieces[i].StrData, Pieces[i].StartOffs + IntraPieceOffset,
                 Pieces[i].EndOffs);
  Size -= Pieces[i].size();
  Pieces[i].EndOffs = Pieces[i].StartOffs + IntraPieceOffset;
  Size += Pieces[i].size();

  ಪ್ರತ್ಯಾಗಮ ಸೇರಿಸು(Offset, Tail);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ಗಂಟು *RopePieceBTreeLeaf::ಸೇರಿಸು(unsigned Offset,
                                               ಧ್ರುವ RopePiece &R) {
  // If this node is not full, insert the piece.
  ಯದಿ (!isFull()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    unsigned i = 0, e = get_ಅಂಕೆ_Pieces();
    ಯದಿ (Offset == size()) {
      // Fastpath for a common case.
      i = e;
    } ಅನ್ಯಥಾ {
      unsigned SlotOffs = 0;
      ಪ್ರತಿ (; Offset > SlotOffs; ++i)
        SlotOffs += getPiece(i).size();
      ಪ್ರತಿಜ್ಞಾ(SlotOffs == Offset && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    ಪ್ರತಿ (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++ಅಂಕೆ_Pieces;
    Size += R.size();
    ಪ್ರತ್ಯಾಗಮ nullptr;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  RopePieceBTreeLeaf *New_ಗಂಟು = ನೂತನ RopePieceBTreeLeaf();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[WidthFactor], &Pieces[2 * WidthFactor],
            &New_ಗಂಟು->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[WidthFactor], &Pieces[2 * WidthFactor], RopePiece());

  // Decrease the number of values in the two nodes.
  New_ಗಂಟು->ಅಂಕೆ_Pieces = ಅಂಕೆ_Pieces = WidthFactor;

  // Recompute the two nodes' size.
  New_ಗಂಟು->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();

  // Update the list of leaves.
  New_ಗಂಟು->ಸೇರಿಸು_After_Leaf_In_Order(ಏತತ್);

  // These insertions can't fail.
  ಯದಿ (ಏತತ್->size() >= Offset)
    ಏತತ್->ಸೇರಿಸು(Offset, R);
  ಅನ್ಯಥಾ
    New_ಗಂಟು->ಸೇರಿಸು(Offset - ಏತತ್->size(), R);
  ಪ್ರತ್ಯಾಗಮ New_ಗಂಟು;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
ಶೂನ್ಯ RopePieceBTreeLeaf::erase(unsigned Offset, unsigned ಅಂಕೆ_Bytes) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  ಪ್ರತಿ (; Offset > PieceOffs; ++i)
    PieceOffs += getPiece(i).size();
  ಪ್ರತಿಜ್ಞಾ(PieceOffs == Offset && "Split didn't occur before erase!");

  unsigned StartPiece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  ಪ್ರತಿ (; Offset + ಅಂಕೆ_Bytes > PieceOffs + getPiece(i).size(); ++i)
    PieceOffs += getPiece(i).size();

  // If we exactly include the last one, include it in the region to delete.
  ಯದಿ (Offset + ಅಂಕೆ_Bytes == PieceOffs + getPiece(i).size()) {
    PieceOffs += getPiece(i).size();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  ಯದಿ (i != StartPiece) {
    unsigned ಅಂಕೆ_Deleted = i - StartPiece;
    ಪ್ರತಿ (; i != get_ಅಂಕೆ_Pieces(); ++i)
      Pieces[i - ಅಂಕೆ_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_ಅಂಕೆ_Pieces() - ಅಂಕೆ_Deleted], &Pieces[get_ಅಂಕೆ_Pieces()],
              RopePiece());
    ಅಂಕೆ_Pieces -= ಅಂಕೆ_Deleted;

    unsigned CoverBytes = PieceOffs - Offset;
    ಅಂಕೆ_Bytes -= CoverBytes;
    Size -= CoverBytes;
  }

  // If we completely removed some stuff, we could be done.
  ಯದಿ (ಅಂಕೆ_Bytes == 0)
    ಪ್ರತ್ಯಾಗಮ;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  ಪ್ರತಿಜ್ಞಾ(getPiece(StartPiece).size() > ಅಂಕೆ_Bytes);
  Pieces[StartPiece].StartOffs += ಅಂಕೆ_Bytes;

  // The size of this node just shrunk by NumBytes.
  Size -= ಅಂಕೆ_Bytes;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

ನಾಮಾಕಾಶ {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
ವರ್ಗ RopePieceBTreeInterior : ಸಾರ್ವಜನಿಕ Rope_Piece_BTree_ಗಂಟು {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  unsigned ವರ್ಣ ಅಂಕೆ_Children = 0;

  Rope_Piece_BTree_ಗಂಟು *Children[2 * WidthFactor];

ಸಾರ್ವಜನಿಕ:
  RopePieceBTreeInterior() : Rope_Piece_BTree_ಗಂಟು(ಅಸತ್ಯ) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_ಗಂಟು *LHS, Rope_Piece_BTree_ಗಂಟು *RHS)
      : Rope_Piece_BTree_ಗಂಟು(ಅಸತ್ಯ) {
    Children[0] = LHS;
    Children[1] = RHS;
    ಅಂಕೆ_Children = 2;
    Size = LHS->size() + RHS->size();
  }

  ~RopePieceBTreeInterior() {
    ಪ್ರತಿ (unsigned i = 0, e = get_ಅಂಕೆ_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  ತಾರ್ಕಿಕ isFull() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ_Children == 2 * WidthFactor; }

  unsigned get_ಅಂಕೆ_Children() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ_Children; }

  ಧ್ರುವ Rope_Piece_BTree_ಗಂಟು *getChild(unsigned i) ಧ್ರುವ {
    ಪ್ರತಿಜ್ಞಾ(i < ಅಂಕೆ_Children && "invalid child #");
    ಪ್ರತ್ಯಾಗಮ Children[i];
  }

  Rope_Piece_BTree_ಗಂಟು *getChild(unsigned i) {
    ಪ್ರತಿಜ್ಞಾ(i < ಅಂಕೆ_Children && "invalid child #");
    ಪ್ರತ್ಯಾಗಮ Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  ಶೂನ್ಯ FullRecomputeSizeLocally() {
    Size = 0;
    ಪ್ರತಿ (unsigned i = 0, e = get_ಅಂಕೆ_Children(); i != e; ++i)
      Size += getChild(i)->size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ಗಂಟು *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ಗಂಟು *ಸೇರಿಸು(unsigned Offset, ಧ್ರುವ RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_ಗಂಟು *HandleChildPiece(unsigned i, Rope_Piece_BTree_ಗಂಟು *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  ಶೂನ್ಯ erase(unsigned Offset, unsigned ಅಂಕೆ_Bytes);

  ಸ್ಥಿರ ತಾರ್ಕಿಕ classof(ಧ್ರುವ Rope_Piece_BTree_ಗಂಟು *N) { ಪ್ರತ್ಯಾಗಮ !N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ಗಂಟು *RopePieceBTreeInterior::split(unsigned Offset) {
  // Figure out which child to split.
  ಯದಿ (Offset == 0 || Offset == size())
    ಪ್ರತ್ಯಾಗಮ nullptr; // If we have an exact offset, we're already split.

  unsigned ChildOffset = 0;
  unsigned i = 0;
  ಪ್ರತಿ (; Offset >= ChildOffset + getChild(i)->size(); ++i)
    ChildOffset += getChild(i)->size();

  // If already split there, we're done.
  ಯದಿ (ChildOffset == Offset)
    ಪ್ರತ್ಯಾಗಮ nullptr;

  // Otherwise, recursively split the child.
  ಯದಿ (Rope_Piece_BTree_ಗಂಟು *RHS = getChild(i)->split(Offset - ChildOffset))
    ಪ್ರತ್ಯಾಗಮ HandleChildPiece(i, RHS);
  ಪ್ರತ್ಯಾಗಮ nullptr; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ಗಂಟು *RopePieceBTreeInterior::ಸೇರಿಸು(unsigned Offset,
                                                   ಧ್ರುವ RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  unsigned i = 0, e = get_ಅಂಕೆ_Children();

  unsigned ChildOffs = 0;
  ಯದಿ (Offset == size()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    ChildOffs = size() - getChild(i)->size();
  } ಅನ್ಯಥಾ {
    ಪ್ರತಿ (; Offset > ChildOffs + getChild(i)->size(); ++i)
      ChildOffs += getChild(i)->size();
  }

  Size += R.size();

  // Insert at the end of this child.
  ಯದಿ (Rope_Piece_BTree_ಗಂಟು *RHS = getChild(i)->ಸೇರಿಸು(Offset - ChildOffs, R))
    ಪ್ರತ್ಯಾಗಮ HandleChildPiece(i, RHS);

  ಪ್ರತ್ಯಾಗಮ nullptr;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_ಗಂಟು *
RopePieceBTreeInterior::HandleChildPiece(unsigned i, Rope_Piece_BTree_ಗಂಟು *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  ಯದಿ (!isFull()) {
    // Insert RHS after child 'i'.
    ಯದಿ (i + 1 != get_ಅಂಕೆ_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_ಅಂಕೆ_Children() - i - 1) * ಪರಿಮಾಣ(Children[0]));
    Children[i + 1] = RHS;
    ++ಅಂಕೆ_Children;
    ಪ್ರತ್ಯಾಗಮ nullptr;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_ಗಂಟು = ನೂತನ RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  memcpy(&New_ಗಂಟು->Children[0], &Children[WidthFactor],
         WidthFactor * ಪರಿಮಾಣ(Children[0]));

  // Decrease the number of values in the two nodes.
  New_ಗಂಟು->ಅಂಕೆ_Children = ಅಂಕೆ_Children = WidthFactor;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  ಯದಿ (i < WidthFactor)
    ಏತತ್->HandleChildPiece(i, RHS);
  ಅನ್ಯಥಾ
    New_ಗಂಟು->HandleChildPiece(i - WidthFactor, RHS);

  // Recompute the two nodes' size.
  New_ಗಂಟು->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();
  ಪ್ರತ್ಯಾಗಮ New_ಗಂಟು;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
ಶೂನ್ಯ RopePieceBTreeInterior::erase(unsigned Offset, unsigned ಅಂಕೆ_Bytes) {
  // This will shrink this node by NumBytes.
  Size -= ಅಂಕೆ_Bytes;

  // Find the first child that overlaps with Offset.
  unsigned i = 0;
  ಪ್ರತಿ (; Offset >= getChild(i)->size(); ++i)
    Offset -= getChild(i)->size();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  ಯಾವತ್ (ಅಂಕೆ_Bytes) {
    Rope_Piece_BTree_ಗಂಟು *CurChild = getChild(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    ಯದಿ (Offset + ಅಂಕೆ_Bytes < CurChild->size()) {
      CurChild->erase(Offset, ಅಂಕೆ_Bytes);
      ಪ್ರತ್ಯಾಗಮ;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    ಯದಿ (Offset) {
      unsigned BytesFromChild = CurChild->size() - Offset;
      CurChild->erase(Offset, BytesFromChild);
      ಅಂಕೆ_Bytes -= BytesFromChild;
      // Start at the beginning of the next child.
      Offset = 0;
      ++i;
      ಅನುವರ್ತ;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    ಅಂಕೆ_Bytes -= CurChild->size();
    CurChild->Destroy();
    --ಅಂಕೆ_Children;
    ಯದಿ (i != get_ಅಂಕೆ_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_ಅಂಕೆ_Children() - i) * ಪರಿಮಾಣ(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

ಶೂನ್ಯ Rope_Piece_BTree_ಗಂಟು::Destroy() {
  ಯದಿ (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(ಏತತ್))
    delete Leaf;
  ಅನ್ಯಥಾ
    delete cast<RopePieceBTreeInterior>(ಏತತ್);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ಗಂಟು *Rope_Piece_BTree_ಗಂಟು::split(unsigned Offset) {
  ಪ್ರತಿಜ್ಞಾ(Offset <= size() && "Invalid offset to split!");
  ಯದಿ (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(ಏತತ್))
    ಪ್ರತ್ಯಾಗಮ Leaf->split(Offset);
  ಪ್ರತ್ಯಾಗಮ cast<RopePieceBTreeInterior>(ಏತತ್)->split(Offset);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ಗಂಟು *Rope_Piece_BTree_ಗಂಟು::ಸೇರಿಸು(unsigned Offset,
                                               ಧ್ರುವ RopePiece &R) {
  ಪ್ರತಿಜ್ಞಾ(Offset <= size() && "Invalid offset to insert!");
  ಯದಿ (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(ಏತತ್))
    ಪ್ರತ್ಯಾಗಮ Leaf->ಸೇರಿಸು(Offset, R);
  ಪ್ರತ್ಯಾಗಮ cast<RopePieceBTreeInterior>(ಏತತ್)->ಸೇರಿಸು(Offset, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
ಶೂನ್ಯ Rope_Piece_BTree_ಗಂಟು::erase(unsigned Offset, unsigned ಅಂಕೆ_Bytes) {
  ಪ್ರತಿಜ್ಞಾ(Offset + ಅಂಕೆ_Bytes <= size() && "Invalid offset to erase!");
  ಯದಿ (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(ಏತತ್))
    ಪ್ರತ್ಯಾಗಮ Leaf->erase(Offset, ಅಂಕೆ_Bytes);
  ಪ್ರತ್ಯಾಗಮ cast<RopePieceBTreeInterior>(ಏತತ್)->erase(Offset, ಅಂಕೆ_Bytes);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

ಸ್ಥಿರ ಧ್ರುವ RopePieceBTreeLeaf *getCN(ಧ್ರುವ ಶೂನ್ಯ *P) {
  ಪ್ರತ್ಯಾಗಮ static_cast<ಧ್ರುವ RopePieceBTreeLeaf *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(ಧ್ರುವ ಶೂನ್ಯ *n) {
  ಧ್ರುವ auto *N = static_cast<ಧ್ರುವ Rope_Piece_BTree_ಗಂಟು *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  ಯಾವತ್ (ಧ್ರುವ auto *IN = dyn_cast<RopePieceBTreeInterior>(N))
    N = IN->getChild(0);

  // We must have at least one leaf.
  Cur_ಗಂಟು = cast<RopePieceBTreeLeaf>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  ಯಾವತ್ (Cur_ಗಂಟು && getCN(Cur_ಗಂಟು)->get_ಅಂಕೆ_Pieces() == 0)
    Cur_ಗಂಟು = getCN(Cur_ಗಂಟು)->getNextLeafInOrder();

  ಯದಿ (Cur_ಗಂಟು)
    CurPiece = &getCN(Cur_ಗಂಟು)->getPiece(0);
  ಅನ್ಯಥಾ // Empty tree, this is an end() iterator.
    CurPiece = nullptr;
  CurChar = 0;
}

ಶೂನ್ಯ RopePieceBTreeIterator::MoveToNextPiece() {
  ಯದಿ (CurPiece !=
      &getCN(Cur_ಗಂಟು)->getPiece(getCN(Cur_ಗಂಟು)->get_ಅಂಕೆ_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    ಪ್ರತ್ಯಾಗಮ;
  }

  // Find the next non-empty leaf node.
  ಕುರು
    Cur_ಗಂಟು = getCN(Cur_ಗಂಟು)->getNextLeafInOrder();
  ಯಾವತ್ (Cur_ಗಂಟು && getCN(Cur_ಗಂಟು)->get_ಅಂಕೆ_Pieces() == 0);

  ಯದಿ (Cur_ಗಂಟು)
    CurPiece = &getCN(Cur_ಗಂಟು)->getPiece(0);
  ಅನ್ಯಥಾ // Hit end().
    CurPiece = nullptr;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

ಸ್ಥಿರ Rope_Piece_BTree_ಗಂಟು *get_ಬೇರು(ಶೂನ್ಯ *P) {
  ಪ್ರತ್ಯಾಗಮ static_cast<Rope_Piece_BTree_ಗಂಟು *>(P);
}

RopePieceBTree::RopePieceBTree() { ಬೇರು_2 = ನೂತನ RopePieceBTreeLeaf(); }

RopePieceBTree::RopePieceBTree(ಧ್ರುವ RopePieceBTree &RHS) {
  ಪ್ರತಿಜ್ಞಾ(RHS.empty() && "Can't copy non-empty tree yet");
  ಬೇರು_2 = ನೂತನ RopePieceBTreeLeaf();
}

RopePieceBTree::~RopePieceBTree() { get_ಬೇರು(ಬೇರು_2)->Destroy(); }

unsigned RopePieceBTree::size() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ get_ಬೇರು(ಬೇರು_2)->size(); }

ಶೂನ್ಯ RopePieceBTree::clear() {
  ಯದಿ (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(get_ಬೇರು(ಬೇರು_2)))
    Leaf->clear();
  ಅನ್ಯಥಾ {
    get_ಬೇರು(ಬೇರು_2)->Destroy();
    ಬೇರು_2 = ನೂತನ RopePieceBTreeLeaf();
  }
}

ಶೂನ್ಯ RopePieceBTree::ಸೇರಿಸು(unsigned Offset, ಧ್ರುವ RopePiece &R) {
  // #1. Split at Offset.
  ಯದಿ (Rope_Piece_BTree_ಗಂಟು *RHS = get_ಬೇರು(ಬೇರು_2)->split(Offset))
    ಬೇರು_2 = ನೂತನ RopePieceBTreeInterior(get_ಬೇರು(ಬೇರು_2), RHS);

  // #2. Do the insertion.
  ಯದಿ (Rope_Piece_BTree_ಗಂಟು *RHS = get_ಬೇರು(ಬೇರು_2)->ಸೇರಿಸು(Offset, R))
    ಬೇರು_2 = ನೂತನ RopePieceBTreeInterior(get_ಬೇರು(ಬೇರು_2), RHS);
}

ಶೂನ್ಯ RopePieceBTree::erase(unsigned Offset, unsigned ಅಂಕೆ_Bytes) {
  // #1. Split at Offset.
  ಯದಿ (Rope_Piece_BTree_ಗಂಟು *RHS = get_ಬೇರು(ಬೇರು_2)->split(Offset))
    ಬೇರು_2 = ನೂತನ RopePieceBTreeInterior(get_ಬೇರು(ಬೇರು_2), RHS);

  // #2. Do the erasing.
  get_ಬೇರು(ಬೇರು_2)->erase(Offset, ಅಂಕೆ_Bytes);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(ಧ್ರುವ ವರ್ಣ *Start, ಧ್ರುವ ವರ್ಣ *End) {
  unsigned Len = End - Start;
  ಪ್ರತಿಜ್ಞಾ(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  ಯದಿ (AllocOffs + Len <= AllocChunkSize) {
    memcpy(AllocBuffer->Data + AllocOffs, Start, Len);
    AllocOffs += Len;
    ಪ್ರತ್ಯಾಗಮ RopePiece(AllocBuffer, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  ಯದಿ (Len > AllocChunkSize) {
    unsigned Size = End - Start + ಪರಿಮಾಣ(Rope_Ref_ಎಣಿಕೆ_String) - 1;
    auto *Res = reinterpret_cast<Rope_Ref_ಎಣಿಕೆ_String *>(ನೂತನ ವರ್ಣ[Size]);
    Res->Ref_ಎಣಿಕೆ = 0;
    memcpy(Res->Data, Start, End - Start);
    ಪ್ರತ್ಯಾಗಮ RopePiece(Res, 0, End - Start);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  unsigned AllocSize = offsetof(Rope_Ref_ಎಣಿಕೆ_String, Data) + AllocChunkSize;
  auto *Res = reinterpret_cast<Rope_Ref_ಎಣಿಕೆ_String *>(ನೂತನ ವರ್ಣ[AllocSize]);
  Res->Ref_ಎಣಿಕೆ = 0;
  memcpy(Res->Data, Start, Len);
  AllocBuffer = Res;
  AllocOffs = Len;

  ಪ್ರತ್ಯಾಗಮ RopePiece(AllocBuffer, 0, Len);
}
