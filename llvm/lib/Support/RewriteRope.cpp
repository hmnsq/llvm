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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

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
ಸಂರಕ್ಷಿತ:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  ಎಣಿಕೆ { ಅಗಲ_ಗುಣಕ = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2 = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  ತಾರ್ಕಿಕ Is_ಎಲೆ;

  Rope_Piece_BTree_ಗಂಟು(ತಾರ್ಕಿಕ is_ಎಲೆ) : Is_ಎಲೆ(is_ಎಲೆ) {}
  ~Rope_Piece_BTree_ಗಂಟು() = ಪೂರ್ವನಿಯೋಜಿತ;

ಸಾರ್ವಜನಿಕ:
  ತಾರ್ಕಿಕ is_ಎಲೆ() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Is_ಎಲೆ; }
  ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_3() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2; }

  ಶೂನ್ಯ Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ಗಂಟು *ವಿಭಜಿಸು(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ಗಂಟು *ಸೇರಿಸು(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ, ಸ್ಥಿರಾಂಕ RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  ಶೂನ್ಯ erase(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ, ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬೈಟುಗಳು);
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
ವರ್ಗ Rope_Piece_BTree_ಎಲೆ : ಸಾರ್ವಜನಿಕ Rope_Piece_BTree_ಗಂಟು {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಅಂಕೆ_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * ಅಗಲ_ಗುಣಕ];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  Rope_Piece_BTree_ಎಲೆ **Prev_ಎಲೆ = ಶೂನ್ಯಸೂಚಕ;
  Rope_Piece_BTree_ಎಲೆ *ಮುಂದಿನ_ಎಲೆ = ಶೂನ್ಯಸೂಚಕ;

ಸಾರ್ವಜನಿಕ:
  Rope_Piece_BTree_ಎಲೆ() : Rope_Piece_BTree_ಗಂಟು(ನಿಜ) {}

  ~Rope_Piece_BTree_ಎಲೆ() {
    ಆದರೆ (Prev_ಎಲೆ || ಮುಂದಿನ_ಎಲೆ)
      remove_From_ಎಲೆ_ಒಳ_ಕ್ರಮ();
    clear();
  }

  ತಾರ್ಕಿಕ is_ತುಂಬಿದ() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ಅಂಕೆ_Pieces == 2 * ಅಗಲ_ಗುಣಕ; }

  /// clear - Remove all rope pieces from this leaf.
  ಶೂನ್ಯ clear() {
    ಆಗ (ಅಂಕೆ_Pieces)
      Pieces[--ಅಂಕೆ_Pieces] = RopePiece();
    ಗಾತ್ರ_2 = 0;
  }

  ಚಿಹ್ನೆರಹಿತ get_ಅಂಕೆ_Pieces() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ಅಂಕೆ_Pieces; }

  ಸ್ಥಿರಾಂಕ RopePiece &getPiece(ಚಿಹ್ನೆರಹಿತ i) ಸ್ಥಿರಾಂಕ {
    ಪ್ರತಿಜ್ಞಾ(i < get_ಅಂಕೆ_Pieces() && "Invalid piece ID");
    ಹಿಂತಿರುಗಿ Pieces[i];
  }

  ಸ್ಥಿರಾಂಕ Rope_Piece_BTree_ಎಲೆ *get_ಮುಂದಿನ_ಎಲೆ_ಒಳ_ಕ್ರಮ() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ಮುಂದಿನ_ಎಲೆ; }

  ಶೂನ್ಯ ಸೇರಿಸು_After_ಎಲೆ_ಒಳ_ಕ್ರಮ(Rope_Piece_BTree_ಎಲೆ *ಗಂಟು) {
    ಪ್ರತಿಜ್ಞಾ(!Prev_ಎಲೆ && !ಮುಂದಿನ_ಎಲೆ && "Already in ordering");

    ಮುಂದಿನ_ಎಲೆ = ಗಂಟು->ಮುಂದಿನ_ಎಲೆ;
    ಆದರೆ (ಮುಂದಿನ_ಎಲೆ)
      ಮುಂದಿನ_ಎಲೆ->Prev_ಎಲೆ = &ಮುಂದಿನ_ಎಲೆ;
    Prev_ಎಲೆ = &ಗಂಟು->ಮುಂದಿನ_ಎಲೆ;
    ಗಂಟು->ಮುಂದಿನ_ಎಲೆ = ಇದು;
  }

  ಶೂನ್ಯ remove_From_ಎಲೆ_ಒಳ_ಕ್ರಮ() {
    ಆದರೆ (Prev_ಎಲೆ) {
      *Prev_ಎಲೆ = ಮುಂದಿನ_ಎಲೆ;
      ಆದರೆ (ಮುಂದಿನ_ಎಲೆ)
        ಮುಂದಿನ_ಎಲೆ->Prev_ಎಲೆ = Prev_ಎಲೆ;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮುಂದಿನ_ಎಲೆ) {
      ಮುಂದಿನ_ಎಲೆ->Prev_ಎಲೆ = ಶೂನ್ಯಸೂಚಕ;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  ಶೂನ್ಯ ತುಂಬಿದ_Recompute_ಗಾತ್ರ_Locally() {
    ಗಾತ್ರ_2 = 0;
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = get_ಅಂಕೆ_Pieces(); i != e; ++i)
      ಗಾತ್ರ_2 += getPiece(i).ಗಾತ್ರ_3();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ಗಂಟು *ವಿಭಜಿಸು(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ಗಂಟು *ಸೇರಿಸು(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ, ಸ್ಥಿರಾಂಕ RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  ಶೂನ್ಯ erase(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ, ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬೈಟುಗಳು);

  ಸ್ಥಿರ ತಾರ್ಕಿಕ classof(ಸ್ಥಿರಾಂಕ Rope_Piece_BTree_ಗಂಟು *N) { ಹಿಂತಿರುಗಿ N->is_ಎಲೆ(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ಗಂಟು *Rope_Piece_BTree_ಎಲೆ::ವಿಭಜಿಸು(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  ಆದರೆ (ಸ್ಥಾನಾಂತರ == 0 || ಸ್ಥಾನಾಂತರ == ಗಾತ್ರ_3()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  }

  // Find the piece that this offset lands in.
  ಚಿಹ್ನೆರಹಿತ PieceOffs = 0;
  ಚಿಹ್ನೆರಹಿತ i = 0;
  ಆಗ (ಸ್ಥಾನಾಂತರ >= PieceOffs + Pieces[i].ಗಾತ್ರ_3()) {
    PieceOffs += Pieces[i].ಗಾತ್ರ_3();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  ಆದರೆ (PieceOffs == ಸ್ಥಾನಾಂತರ)
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  ಚಿಹ್ನೆರಹಿತ Intra_Piece_ಸ್ಥಾನಾಂತರ = ಸ್ಥಾನಾಂತರ - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece ಬಾಲ(Pieces[i].Str_ದತ್ತಾಂಶ, Pieces[i].ಆರಂಭ_Offs + Intra_Piece_ಸ್ಥಾನಾಂತರ,
                 Pieces[i].ಕೊನೆ_Offs);
  ಗಾತ್ರ_2 -= Pieces[i].ಗಾತ್ರ_3();
  Pieces[i].ಕೊನೆ_Offs = Pieces[i].ಆರಂಭ_Offs + Intra_Piece_ಸ್ಥಾನಾಂತರ;
  ಗಾತ್ರ_2 += Pieces[i].ಗಾತ್ರ_3();

  ಹಿಂತಿರುಗಿ ಸೇರಿಸು(ಸ್ಥಾನಾಂತರ, ಬಾಲ);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ಗಂಟು *Rope_Piece_BTree_ಎಲೆ::ಸೇರಿಸು(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ,
                                               ಸ್ಥಿರಾಂಕ RopePiece &R) {
  // If this node is not full, insert the piece.
  ಆದರೆ (!is_ತುಂಬಿದ()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    ಚಿಹ್ನೆರಹಿತ i = 0, e = get_ಅಂಕೆ_Pieces();
    ಆದರೆ (ಸ್ಥಾನಾಂತರ == ಗಾತ್ರ_3()) {
      // Fastpath for a common case.
      i = e;
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನ_Offs = 0;
      ಗಾಗಿ (; ಸ್ಥಾನಾಂತರ > ಸ್ಥಾನ_Offs; ++i)
        ಸ್ಥಾನ_Offs += getPiece(i).ಗಾತ್ರ_3();
      ಪ್ರತಿಜ್ಞಾ(ಸ್ಥಾನ_Offs == ಸ್ಥಾನಾಂತರ && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    ಗಾಗಿ (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++ಅಂಕೆ_Pieces;
    ಗಾತ್ರ_2 += R.ಗಾತ್ರ_3();
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  Rope_Piece_BTree_ಎಲೆ *New_ಗಂಟು = ಹೊಸ Rope_Piece_BTree_ಎಲೆ();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[ಅಗಲ_ಗುಣಕ], &Pieces[2 * ಅಗಲ_ಗುಣಕ],
            &New_ಗಂಟು->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[ಅಗಲ_ಗುಣಕ], &Pieces[2 * ಅಗಲ_ಗುಣಕ], RopePiece());

  // Decrease the number of values in the two nodes.
  New_ಗಂಟು->ಅಂಕೆ_Pieces = ಅಂಕೆ_Pieces = ಅಗಲ_ಗುಣಕ;

  // Recompute the two nodes' size.
  New_ಗಂಟು->ತುಂಬಿದ_Recompute_ಗಾತ್ರ_Locally();
  ತುಂಬಿದ_Recompute_ಗಾತ್ರ_Locally();

  // Update the list of leaves.
  New_ಗಂಟು->ಸೇರಿಸು_After_ಎಲೆ_ಒಳ_ಕ್ರಮ(ಇದು);

  // These insertions can't fail.
  ಆದರೆ (ಇದು->ಗಾತ್ರ_3() >= ಸ್ಥಾನಾಂತರ)
    ಇದು->ಸೇರಿಸು(ಸ್ಥಾನಾಂತರ, R);
  ಇಲ್ಲದಿದ್ದರೆ
    New_ಗಂಟು->ಸೇರಿಸು(ಸ್ಥಾನಾಂತರ - ಇದು->ಗಾತ್ರ_3(), R);
  ಹಿಂತಿರುಗಿ New_ಗಂಟು;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
ಶೂನ್ಯ Rope_Piece_BTree_ಎಲೆ::erase(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ, ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬೈಟುಗಳು) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  ಚಿಹ್ನೆರಹಿತ PieceOffs = 0;
  ಚಿಹ್ನೆರಹಿತ i = 0;
  ಗಾಗಿ (; ಸ್ಥಾನಾಂತರ > PieceOffs; ++i)
    PieceOffs += getPiece(i).ಗಾತ್ರ_3();
  ಪ್ರತಿಜ್ಞಾ(PieceOffs == ಸ್ಥಾನಾಂತರ && "Split didn't occur before erase!");

  ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_Piece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  ಗಾಗಿ (; ಸ್ಥಾನಾಂತರ + ಅಂಕೆ_ಬೈಟುಗಳು > PieceOffs + getPiece(i).ಗಾತ್ರ_3(); ++i)
    PieceOffs += getPiece(i).ಗಾತ್ರ_3();

  // If we exactly include the last one, include it in the region to delete.
  ಆದರೆ (ಸ್ಥಾನಾಂತರ + ಅಂಕೆ_ಬೈಟುಗಳು == PieceOffs + getPiece(i).ಗಾತ್ರ_3()) {
    PieceOffs += getPiece(i).ಗಾತ್ರ_3();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  ಆದರೆ (i != ಆರಂಭ_Piece) {
    ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Deleted = i - ಆರಂಭ_Piece;
    ಗಾಗಿ (; i != get_ಅಂಕೆ_Pieces(); ++i)
      Pieces[i - ಅಂಕೆ_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_ಅಂಕೆ_Pieces() - ಅಂಕೆ_Deleted], &Pieces[get_ಅಂಕೆ_Pieces()],
              RopePiece());
    ಅಂಕೆ_Pieces -= ಅಂಕೆ_Deleted;

    ಚಿಹ್ನೆರಹಿತ Cover_ಬೈಟುಗಳು = PieceOffs - ಸ್ಥಾನಾಂತರ;
    ಅಂಕೆ_ಬೈಟುಗಳು -= Cover_ಬೈಟುಗಳು;
    ಗಾತ್ರ_2 -= Cover_ಬೈಟುಗಳು;
  }

  // If we completely removed some stuff, we could be done.
  ಆದರೆ (ಅಂಕೆ_ಬೈಟುಗಳು == 0)
    ಹಿಂತಿರುಗಿ;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  ಪ್ರತಿಜ್ಞಾ(getPiece(ಆರಂಭ_Piece).ಗಾತ್ರ_3() > ಅಂಕೆ_ಬೈಟುಗಳು);
  Pieces[ಆರಂಭ_Piece].ಆರಂಭ_Offs += ಅಂಕೆ_ಬೈಟುಗಳು;

  // The size of this node just shrunk by NumBytes.
  ಗಾತ್ರ_2 -= ಅಂಕೆ_ಬೈಟುಗಳು;
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
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಅಂಕೆ_Children = 0;

  Rope_Piece_BTree_ಗಂಟು *Children[2 * ಅಗಲ_ಗುಣಕ];

ಸಾರ್ವಜನಿಕ:
  RopePieceBTreeInterior() : Rope_Piece_BTree_ಗಂಟು(ಸುಳ್ಳು) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_ಗಂಟು *LHS, Rope_Piece_BTree_ಗಂಟು *RHS)
      : Rope_Piece_BTree_ಗಂಟು(ಸುಳ್ಳು) {
    Children[0] = LHS;
    Children[1] = RHS;
    ಅಂಕೆ_Children = 2;
    ಗಾತ್ರ_2 = LHS->ಗಾತ್ರ_3() + RHS->ಗಾತ್ರ_3();
  }

  ~RopePieceBTreeInterior() {
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = get_ಅಂಕೆ_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  ತಾರ್ಕಿಕ is_ತುಂಬಿದ() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ಅಂಕೆ_Children == 2 * ಅಗಲ_ಗುಣಕ; }

  ಚಿಹ್ನೆರಹಿತ get_ಅಂಕೆ_Children() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ಅಂಕೆ_Children; }

  ಸ್ಥಿರಾಂಕ Rope_Piece_BTree_ಗಂಟು *get_ಮಗು(ಚಿಹ್ನೆರಹಿತ i) ಸ್ಥಿರಾಂಕ {
    ಪ್ರತಿಜ್ಞಾ(i < ಅಂಕೆ_Children && "invalid child #");
    ಹಿಂತಿರುಗಿ Children[i];
  }

  Rope_Piece_BTree_ಗಂಟು *get_ಮಗು(ಚಿಹ್ನೆರಹಿತ i) {
    ಪ್ರತಿಜ್ಞಾ(i < ಅಂಕೆ_Children && "invalid child #");
    ಹಿಂತಿರುಗಿ Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  ಶೂನ್ಯ ತುಂಬಿದ_Recompute_ಗಾತ್ರ_Locally() {
    ಗಾತ್ರ_2 = 0;
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = get_ಅಂಕೆ_Children(); i != e; ++i)
      ಗಾತ್ರ_2 += get_ಮಗು(i)->ಗಾತ್ರ_3();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ಗಂಟು *ವಿಭಜಿಸು(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ಗಂಟು *ಸೇರಿಸು(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ, ಸ್ಥಿರಾಂಕ RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_ಗಂಟು *Handle_ಮಗು_Piece(ಚಿಹ್ನೆರಹಿತ i, Rope_Piece_BTree_ಗಂಟು *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  ಶೂನ್ಯ erase(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ, ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬೈಟುಗಳು);

  ಸ್ಥಿರ ತಾರ್ಕಿಕ classof(ಸ್ಥಿರಾಂಕ Rope_Piece_BTree_ಗಂಟು *N) { ಹಿಂತಿರುಗಿ !N->is_ಎಲೆ(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ಗಂಟು *RopePieceBTreeInterior::ವಿಭಜಿಸು(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ) {
  // Figure out which child to split.
  ಆದರೆ (ಸ್ಥಾನಾಂತರ == 0 || ಸ್ಥಾನಾಂತರ == ಗಾತ್ರ_3())
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ; // If we have an exact offset, we're already split.

  ಚಿಹ್ನೆರಹಿತ ಮಗು_ಸ್ಥಾನಾಂತರ = 0;
  ಚಿಹ್ನೆರಹಿತ i = 0;
  ಗಾಗಿ (; ಸ್ಥಾನಾಂತರ >= ಮಗು_ಸ್ಥಾನಾಂತರ + get_ಮಗು(i)->ಗಾತ್ರ_3(); ++i)
    ಮಗು_ಸ್ಥಾನಾಂತರ += get_ಮಗು(i)->ಗಾತ್ರ_3();

  // If already split there, we're done.
  ಆದರೆ (ಮಗು_ಸ್ಥಾನಾಂತರ == ಸ್ಥಾನಾಂತರ)
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

  // Otherwise, recursively split the child.
  ಆದರೆ (Rope_Piece_BTree_ಗಂಟು *RHS = get_ಮಗು(i)->ವಿಭಜಿಸು(ಸ್ಥಾನಾಂತರ - ಮಗು_ಸ್ಥಾನಾಂತರ))
    ಹಿಂತಿರುಗಿ Handle_ಮಗು_Piece(i, RHS);
  ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ಗಂಟು *RopePieceBTreeInterior::ಸೇರಿಸು(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ,
                                                   ಸ್ಥಿರಾಂಕ RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  ಚಿಹ್ನೆರಹಿತ i = 0, e = get_ಅಂಕೆ_Children();

  ಚಿಹ್ನೆರಹಿತ ಮಗು_Offs = 0;
  ಆದರೆ (ಸ್ಥಾನಾಂತರ == ಗಾತ್ರ_3()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    ಮಗು_Offs = ಗಾತ್ರ_3() - get_ಮಗು(i)->ಗಾತ್ರ_3();
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಗಾಗಿ (; ಸ್ಥಾನಾಂತರ > ಮಗು_Offs + get_ಮಗು(i)->ಗಾತ್ರ_3(); ++i)
      ಮಗು_Offs += get_ಮಗು(i)->ಗಾತ್ರ_3();
  }

  ಗಾತ್ರ_2 += R.ಗಾತ್ರ_3();

  // Insert at the end of this child.
  ಆದರೆ (Rope_Piece_BTree_ಗಂಟು *RHS = get_ಮಗು(i)->ಸೇರಿಸು(ಸ್ಥಾನಾಂತರ - ಮಗು_Offs, R))
    ಹಿಂತಿರುಗಿ Handle_ಮಗು_Piece(i, RHS);

  ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_ಗಂಟು *
RopePieceBTreeInterior::Handle_ಮಗು_Piece(ಚಿಹ್ನೆರಹಿತ i, Rope_Piece_BTree_ಗಂಟು *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  ಆದರೆ (!is_ತುಂಬಿದ()) {
    // Insert RHS after child 'i'.
    ಆದರೆ (i + 1 != get_ಅಂಕೆ_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_ಅಂಕೆ_Children() - i - 1) * ಗಾತ್ರ(Children[0]));
    Children[i + 1] = RHS;
    ++ಅಂಕೆ_Children;
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_ಗಂಟು = ಹೊಸ RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  ಸ್ಮರಣೆ_ನಕಲು(&New_ಗಂಟು->Children[0], &Children[ಅಗಲ_ಗುಣಕ],
         ಅಗಲ_ಗುಣಕ * ಗಾತ್ರ(Children[0]));

  // Decrease the number of values in the two nodes.
  New_ಗಂಟು->ಅಂಕೆ_Children = ಅಂಕೆ_Children = ಅಗಲ_ಗುಣಕ;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  ಆದರೆ (i < ಅಗಲ_ಗುಣಕ)
    ಇದು->Handle_ಮಗು_Piece(i, RHS);
  ಇಲ್ಲದಿದ್ದರೆ
    New_ಗಂಟು->Handle_ಮಗು_Piece(i - ಅಗಲ_ಗುಣಕ, RHS);

  // Recompute the two nodes' size.
  New_ಗಂಟು->ತುಂಬಿದ_Recompute_ಗಾತ್ರ_Locally();
  ತುಂಬಿದ_Recompute_ಗಾತ್ರ_Locally();
  ಹಿಂತಿರುಗಿ New_ಗಂಟು;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
ಶೂನ್ಯ RopePieceBTreeInterior::erase(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ, ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬೈಟುಗಳು) {
  // This will shrink this node by NumBytes.
  ಗಾತ್ರ_2 -= ಅಂಕೆ_ಬೈಟುಗಳು;

  // Find the first child that overlaps with Offset.
  ಚಿಹ್ನೆರಹಿತ i = 0;
  ಗಾಗಿ (; ಸ್ಥಾನಾಂತರ >= get_ಮಗು(i)->ಗಾತ್ರ_3(); ++i)
    ಸ್ಥಾನಾಂತರ -= get_ಮಗು(i)->ಗಾತ್ರ_3();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  ಆಗ (ಅಂಕೆ_ಬೈಟುಗಳು) {
    Rope_Piece_BTree_ಗಂಟು *Cur_ಮಗು = get_ಮಗು(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    ಆದರೆ (ಸ್ಥಾನಾಂತರ + ಅಂಕೆ_ಬೈಟುಗಳು < Cur_ಮಗು->ಗಾತ್ರ_3()) {
      Cur_ಮಗು->erase(ಸ್ಥಾನಾಂತರ, ಅಂಕೆ_ಬೈಟುಗಳು);
      ಹಿಂತಿರುಗಿ;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    ಆದರೆ (ಸ್ಥಾನಾಂತರ) {
      ಚಿಹ್ನೆರಹಿತ ಬೈಟುಗಳು_From_ಮಗು = Cur_ಮಗು->ಗಾತ್ರ_3() - ಸ್ಥಾನಾಂತರ;
      Cur_ಮಗು->erase(ಸ್ಥಾನಾಂತರ, ಬೈಟುಗಳು_From_ಮಗು);
      ಅಂಕೆ_ಬೈಟುಗಳು -= ಬೈಟುಗಳು_From_ಮಗು;
      // Start at the beginning of the next child.
      ಸ್ಥಾನಾಂತರ = 0;
      ++i;
      ಮುಂದುವರಿ;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    ಅಂಕೆ_ಬೈಟುಗಳು -= Cur_ಮಗು->ಗಾತ್ರ_3();
    Cur_ಮಗು->Destroy();
    --ಅಂಕೆ_Children;
    ಆದರೆ (i != get_ಅಂಕೆ_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_ಅಂಕೆ_Children() - i) * ಗಾತ್ರ(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

ಶೂನ್ಯ Rope_Piece_BTree_ಗಂಟು::Destroy() {
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *ಎಲೆ = dyn_cast<Rope_Piece_BTree_ಎಲೆ>(ಇದು))
    ಅಳಿಸು ಎಲೆ;
  ಇಲ್ಲದಿದ್ದರೆ
    ಅಳಿಸು cast<RopePieceBTreeInterior>(ಇದು);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ಗಂಟು *Rope_Piece_BTree_ಗಂಟು::ವಿಭಜಿಸು(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ) {
  ಪ್ರತಿಜ್ಞಾ(ಸ್ಥಾನಾಂತರ <= ಗಾತ್ರ_3() && "Invalid offset to split!");
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *ಎಲೆ = dyn_cast<Rope_Piece_BTree_ಎಲೆ>(ಇದು))
    ಹಿಂತಿರುಗಿ ಎಲೆ->ವಿಭಜಿಸು(ಸ್ಥಾನಾಂತರ);
  ಹಿಂತಿರುಗಿ cast<RopePieceBTreeInterior>(ಇದು)->ವಿಭಜಿಸು(ಸ್ಥಾನಾಂತರ);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ಗಂಟು *Rope_Piece_BTree_ಗಂಟು::ಸೇರಿಸು(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ,
                                               ಸ್ಥಿರಾಂಕ RopePiece &R) {
  ಪ್ರತಿಜ್ಞಾ(ಸ್ಥಾನಾಂತರ <= ಗಾತ್ರ_3() && "Invalid offset to insert!");
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *ಎಲೆ = dyn_cast<Rope_Piece_BTree_ಎಲೆ>(ಇದು))
    ಹಿಂತಿರುಗಿ ಎಲೆ->ಸೇರಿಸು(ಸ್ಥಾನಾಂತರ, R);
  ಹಿಂತಿರುಗಿ cast<RopePieceBTreeInterior>(ಇದು)->ಸೇರಿಸು(ಸ್ಥಾನಾಂತರ, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
ಶೂನ್ಯ Rope_Piece_BTree_ಗಂಟು::erase(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ, ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬೈಟುಗಳು) {
  ಪ್ರತಿಜ್ಞಾ(ಸ್ಥಾನಾಂತರ + ಅಂಕೆ_ಬೈಟುಗಳು <= ಗಾತ್ರ_3() && "Invalid offset to erase!");
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *ಎಲೆ = dyn_cast<Rope_Piece_BTree_ಎಲೆ>(ಇದು))
    ಹಿಂತಿರುಗಿ ಎಲೆ->erase(ಸ್ಥಾನಾಂತರ, ಅಂಕೆ_ಬೈಟುಗಳು);
  ಹಿಂತಿರುಗಿ cast<RopePieceBTreeInterior>(ಇದು)->erase(ಸ್ಥಾನಾಂತರ, ಅಂಕೆ_ಬೈಟುಗಳು);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ Rope_Piece_BTree_ಎಲೆ *getCN(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *P) {
  ಹಿಂತಿರುಗಿ static_cast<ಸ್ಥಿರಾಂಕ Rope_Piece_BTree_ಎಲೆ *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *n) {
  ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ *N = static_cast<ಸ್ಥಿರಾಂಕ Rope_Piece_BTree_ಗಂಟು *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  ಆಗ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ *ಒಳ = dyn_cast<RopePieceBTreeInterior>(N))
    N = ಒಳ->get_ಮಗು(0);

  // We must have at least one leaf.
  Cur_ಗಂಟು = cast<Rope_Piece_BTree_ಎಲೆ>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  ಆಗ (Cur_ಗಂಟು && getCN(Cur_ಗಂಟು)->get_ಅಂಕೆ_Pieces() == 0)
    Cur_ಗಂಟು = getCN(Cur_ಗಂಟು)->get_ಮುಂದಿನ_ಎಲೆ_ಒಳ_ಕ್ರಮ();

  ಆದರೆ (Cur_ಗಂಟು)
    CurPiece = &getCN(Cur_ಗಂಟು)->getPiece(0);
  ಇಲ್ಲದಿದ್ದರೆ // Empty tree, this is an end() iterator.
    CurPiece = ಶೂನ್ಯಸೂಚಕ;
  CurChar = 0;
}

ಶೂನ್ಯ RopePieceBTreeIterator::Move_ಗಮ್ಯ_ಮುಂದಿನ_Piece() {
  ಆದರೆ (CurPiece !=
      &getCN(Cur_ಗಂಟು)->getPiece(getCN(Cur_ಗಂಟು)->get_ಅಂಕೆ_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    ಹಿಂತಿರುಗಿ;
  }

  // Find the next non-empty leaf node.
  ಮಾಡು
    Cur_ಗಂಟು = getCN(Cur_ಗಂಟು)->get_ಮುಂದಿನ_ಎಲೆ_ಒಳ_ಕ್ರಮ();
  ಆಗ (Cur_ಗಂಟು && getCN(Cur_ಗಂಟು)->get_ಅಂಕೆ_Pieces() == 0);

  ಆದರೆ (Cur_ಗಂಟು)
    CurPiece = &getCN(Cur_ಗಂಟು)->getPiece(0);
  ಇಲ್ಲದಿದ್ದರೆ // Hit end().
    CurPiece = ಶೂನ್ಯಸೂಚಕ;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

ಸ್ಥಿರ Rope_Piece_BTree_ಗಂಟು *get_ಬೇರು(ಶೂನ್ಯ *P) {
  ಹಿಂತಿರುಗಿ static_cast<Rope_Piece_BTree_ಗಂಟು *>(P);
}

RopePieceBTree::RopePieceBTree() { ಬೇರು_2 = ಹೊಸ Rope_Piece_BTree_ಎಲೆ(); }

RopePieceBTree::RopePieceBTree(ಸ್ಥಿರಾಂಕ RopePieceBTree &RHS) {
  ಪ್ರತಿಜ್ಞಾ(RHS.ಖಾಲಿಗಳು() && "Can't copy non-empty tree yet");
  ಬೇರು_2 = ಹೊಸ Rope_Piece_BTree_ಎಲೆ();
}

RopePieceBTree::~RopePieceBTree() { get_ಬೇರು(ಬೇರು_2)->Destroy(); }

ಚಿಹ್ನೆರಹಿತ RopePieceBTree::ಗಾತ್ರ_3() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ get_ಬೇರು(ಬೇರು_2)->ಗಾತ್ರ_3(); }

ಶೂನ್ಯ RopePieceBTree::clear() {
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ *ಎಲೆ = dyn_cast<Rope_Piece_BTree_ಎಲೆ>(get_ಬೇರು(ಬೇರು_2)))
    ಎಲೆ->clear();
  ಇಲ್ಲದಿದ್ದರೆ {
    get_ಬೇರು(ಬೇರು_2)->Destroy();
    ಬೇರು_2 = ಹೊಸ Rope_Piece_BTree_ಎಲೆ();
  }
}

ಶೂನ್ಯ RopePieceBTree::ಸೇರಿಸು(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ, ಸ್ಥಿರಾಂಕ RopePiece &R) {
  // #1. Split at Offset.
  ಆದರೆ (Rope_Piece_BTree_ಗಂಟು *RHS = get_ಬೇರು(ಬೇರು_2)->ವಿಭಜಿಸು(ಸ್ಥಾನಾಂತರ))
    ಬೇರು_2 = ಹೊಸ RopePieceBTreeInterior(get_ಬೇರು(ಬೇರು_2), RHS);

  // #2. Do the insertion.
  ಆದರೆ (Rope_Piece_BTree_ಗಂಟು *RHS = get_ಬೇರು(ಬೇರು_2)->ಸೇರಿಸು(ಸ್ಥಾನಾಂತರ, R))
    ಬೇರು_2 = ಹೊಸ RopePieceBTreeInterior(get_ಬೇರು(ಬೇರು_2), RHS);
}

ಶೂನ್ಯ RopePieceBTree::erase(ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ, ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬೈಟುಗಳು) {
  // #1. Split at Offset.
  ಆದರೆ (Rope_Piece_BTree_ಗಂಟು *RHS = get_ಬೇರು(ಬೇರು_2)->ವಿಭಜಿಸು(ಸ್ಥಾನಾಂತರ))
    ಬೇರು_2 = ಹೊಸ RopePieceBTreeInterior(get_ಬೇರು(ಬೇರು_2), RHS);

  // #2. Do the erasing.
  get_ಬೇರು(ಬೇರು_2)->erase(ಸ್ಥಾನಾಂತರ, ಅಂಕೆ_ಬೈಟುಗಳು);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆರಂಭ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆ_2) {
  ಚಿಹ್ನೆರಹಿತ Len = ಕೊನೆ_2 - ಆರಂಭ;
  ಪ್ರತಿಜ್ಞಾ(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  ಆದರೆ (AllocOffs + Len <= Alloc_Chunk_ಗಾತ್ರ) {
    ಸ್ಮರಣೆ_ನಕಲು(Alloc_ಬಫರ್->ದತ್ತಾಂಶ_2 + AllocOffs, ಆರಂಭ, Len);
    AllocOffs += Len;
    ಹಿಂತಿರುಗಿ RopePiece(Alloc_ಬಫರ್, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  ಆದರೆ (Len > Alloc_Chunk_ಗಾತ್ರ) {
    ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2 = ಕೊನೆ_2 - ಆರಂಭ + ಗಾತ್ರ(Rope_Ref_ಎಣಿಕೆ_String) - 1;
    ಸ್ವಯಂಚಾಲಿತ *Res = reinterpret_cast<Rope_Ref_ಎಣಿಕೆ_String *>(ಹೊಸ ಅಕ್ಷರ[ಗಾತ್ರ_2]);
    Res->Ref_ಎಣಿಕೆ = 0;
    ಸ್ಮರಣೆ_ನಕಲು(Res->ದತ್ತಾಂಶ_2, ಆರಂಭ, ಕೊನೆ_2 - ಆರಂಭ);
    ಹಿಂತಿರುಗಿ RopePiece(Res, 0, ಕೊನೆ_2 - ಆರಂಭ);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  ಚಿಹ್ನೆರಹಿತ Alloc_ಗಾತ್ರ = offsetof(Rope_Ref_ಎಣಿಕೆ_String, ದತ್ತಾಂಶ_2) + Alloc_Chunk_ಗಾತ್ರ;
  ಸ್ವಯಂಚಾಲಿತ *Res = reinterpret_cast<Rope_Ref_ಎಣಿಕೆ_String *>(ಹೊಸ ಅಕ್ಷರ[Alloc_ಗಾತ್ರ]);
  Res->Ref_ಎಣಿಕೆ = 0;
  ಸ್ಮರಣೆ_ನಕಲು(Res->ದತ್ತಾಂಶ_2, ಆರಂಭ, Len);
  Alloc_ಬಫರ್ = Res;
  AllocOffs = Len;

  ಹಿಂತಿರುಗಿ RopePiece(Alloc_ಬಫರ್, 0, Len);
}
