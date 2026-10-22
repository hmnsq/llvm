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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/RewriteRope.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Casting.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <algorithm>
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstring>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

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

ਨਾਮਾਕਾਸ਼ {

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Class
//===----------------------------------------------------------------------===//

/// RopePieceBTreeNode - Common base class of RopePieceBTreeLeaf and
/// RopePieceBTreeInterior.  This provides some 'virtual' dispatching methods
/// and a flag that determines which subclass the instance is.  Also
/// important, this node knows the full extend of the node, including any
/// children that it has.  This allows efficient skipping over entire subtrees
/// when looking for an offset in the BTree.
ਵਰਗ Rope_Piece_BTree_ਗੰਢ {
ਸੁਰੱਖਿਅਤ:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  ਗਿਣਤੀ { ਚੌੜਾਈ_ਗੁਣਕ = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  ਤਾਰਕਿਕ Is_ਪੱਤੀ;

  Rope_Piece_BTree_ਗੰਢ(ਤਾਰਕਿਕ is_ਪੱਤੀ) : Is_ਪੱਤੀ(is_ਪੱਤੀ) {}
  ~Rope_Piece_BTree_ਗੰਢ() = ਮੂਲ;

ਜਨਤਕ:
  ਤਾਰਕਿਕ is_ਪੱਤੀ() ਅਚਲ { ਵਾਪਸ Is_ਪੱਤੀ; }
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ_2() ਅਚਲ { ਵਾਪਸ ਮਾਪ; }

  ਖਾਲੀ Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ਗੰਢ *ਵੰਡੋ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ਗੰਢ *ਪਾਓ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ, ਅਚਲ RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  ਖਾਲੀ erase(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਾਈਟਾਂ);
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
ਵਰਗ Rope_Piece_BTree_ਪੱਤੀ : ਜਨਤਕ Rope_Piece_BTree_ਗੰਢ {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਅੰਕ_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * ਚੌੜਾਈ_ਗੁਣਕ];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  Rope_Piece_BTree_ਪੱਤੀ **Prev_ਪੱਤੀ = ਖਾਲੀ_ਸੂਚਕ;
  Rope_Piece_BTree_ਪੱਤੀ *ਅਗਲਾ_ਪੱਤੀ = ਖਾਲੀ_ਸੂਚਕ;

ਜਨਤਕ:
  Rope_Piece_BTree_ਪੱਤੀ() : Rope_Piece_BTree_ਗੰਢ(ਸੱਚ) {}

  ~Rope_Piece_BTree_ਪੱਤੀ() {
    ਜੇ (Prev_ਪੱਤੀ || ਅਗਲਾ_ਪੱਤੀ)
      remove_From_ਪੱਤੀ_ਵਿੱਚ_ਕ੍ਰਮ();
    clear();
  }

  ਤਾਰਕਿਕ is_ਭਰਿਆ() ਅਚਲ { ਵਾਪਸ ਅੰਕ_Pieces == 2 * ਚੌੜਾਈ_ਗੁਣਕ; }

  /// clear - Remove all rope pieces from this leaf.
  ਖਾਲੀ clear() {
    ਜਦੋਂ (ਅੰਕ_Pieces)
      Pieces[--ਅੰਕ_Pieces] = RopePiece();
    ਮਾਪ = 0;
  }

  ਬਿਨਾਨਿਸ਼ਾਨੀ get_ਅੰਕ_Pieces() ਅਚਲ { ਵਾਪਸ ਅੰਕ_Pieces; }

  ਅਚਲ RopePiece &getPiece(ਬਿਨਾਨਿਸ਼ਾਨੀ i) ਅਚਲ {
    ਪ੍ਰਤਿਜ੍ਞਾ(i < get_ਅੰਕ_Pieces() && "Invalid piece ID");
    ਵਾਪਸ Pieces[i];
  }

  ਅਚਲ Rope_Piece_BTree_ਪੱਤੀ *get_ਅਗਲਾ_ਪੱਤੀ_ਵਿੱਚ_ਕ੍ਰਮ() ਅਚਲ { ਵਾਪਸ ਅਗਲਾ_ਪੱਤੀ; }

  ਖਾਲੀ ਪਾਓ_After_ਪੱਤੀ_ਵਿੱਚ_ਕ੍ਰਮ(Rope_Piece_BTree_ਪੱਤੀ *ਗੰਢ) {
    ਪ੍ਰਤਿਜ੍ਞਾ(!Prev_ਪੱਤੀ && !ਅਗਲਾ_ਪੱਤੀ && "Already in ordering");

    ਅਗਲਾ_ਪੱਤੀ = ਗੰਢ->ਅਗਲਾ_ਪੱਤੀ;
    ਜੇ (ਅਗਲਾ_ਪੱਤੀ)
      ਅਗਲਾ_ਪੱਤੀ->Prev_ਪੱਤੀ = &ਅਗਲਾ_ਪੱਤੀ;
    Prev_ਪੱਤੀ = &ਗੰਢ->ਅਗਲਾ_ਪੱਤੀ;
    ਗੰਢ->ਅਗਲਾ_ਪੱਤੀ = ਇਹ;
  }

  ਖਾਲੀ remove_From_ਪੱਤੀ_ਵਿੱਚ_ਕ੍ਰਮ() {
    ਜੇ (Prev_ਪੱਤੀ) {
      *Prev_ਪੱਤੀ = ਅਗਲਾ_ਪੱਤੀ;
      ਜੇ (ਅਗਲਾ_ਪੱਤੀ)
        ਅਗਲਾ_ਪੱਤੀ->Prev_ਪੱਤੀ = Prev_ਪੱਤੀ;
    } ਨਹੀਂਤਾਂ ਜੇ (ਅਗਲਾ_ਪੱਤੀ) {
      ਅਗਲਾ_ਪੱਤੀ->Prev_ਪੱਤੀ = ਖਾਲੀ_ਸੂਚਕ;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  ਖਾਲੀ ਭਰਿਆ_Recompute_ਮਾਪ_Locally() {
    ਮਾਪ = 0;
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = get_ਅੰਕ_Pieces(); i != e; ++i)
      ਮਾਪ += getPiece(i).ਮਾਪ_2();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ਗੰਢ *ਵੰਡੋ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ਗੰਢ *ਪਾਓ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ, ਅਚਲ RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  ਖਾਲੀ erase(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਾਈਟਾਂ);

  ਸਥਿਰ ਤਾਰਕਿਕ classof(ਅਚਲ Rope_Piece_BTree_ਗੰਢ *N) { ਵਾਪਸ N->is_ਪੱਤੀ(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ਗੰਢ *Rope_Piece_BTree_ਪੱਤੀ::ਵੰਡੋ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  ਜੇ (ਵਿੱਥ == 0 || ਵਿੱਥ == ਮਾਪ_2()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  }

  // Find the piece that this offset lands in.
  ਬਿਨਾਨਿਸ਼ਾਨੀ PieceOffs = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0;
  ਜਦੋਂ (ਵਿੱਥ >= PieceOffs + Pieces[i].ਮਾਪ_2()) {
    PieceOffs += Pieces[i].ਮਾਪ_2();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  ਜੇ (PieceOffs == ਵਿੱਥ)
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  ਬਿਨਾਨਿਸ਼ਾਨੀ Intra_Piece_ਵਿੱਥ = ਵਿੱਥ - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece ਪੂਛ(Pieces[i].Str_ਅੰਕੜੇ, Pieces[i].ਸ਼ੁਰੂ_Offs + Intra_Piece_ਵਿੱਥ,
                 Pieces[i].ਸਿਰਾ_Offs);
  ਮਾਪ -= Pieces[i].ਮਾਪ_2();
  Pieces[i].ਸਿਰਾ_Offs = Pieces[i].ਸ਼ੁਰੂ_Offs + Intra_Piece_ਵਿੱਥ;
  ਮਾਪ += Pieces[i].ਮਾਪ_2();

  ਵਾਪਸ ਪਾਓ(ਵਿੱਥ, ਪੂਛ);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ਗੰਢ *Rope_Piece_BTree_ਪੱਤੀ::ਪਾਓ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ,
                                               ਅਚਲ RopePiece &R) {
  // If this node is not full, insert the piece.
  ਜੇ (!is_ਭਰਿਆ()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = get_ਅੰਕ_Pieces();
    ਜੇ (ਵਿੱਥ == ਮਾਪ_2()) {
      // Fastpath for a common case.
      i = e;
    } ਨਹੀਂਤਾਂ {
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਥਾਂ_Offs = 0;
      ਲਈ (; ਵਿੱਥ > ਥਾਂ_Offs; ++i)
        ਥਾਂ_Offs += getPiece(i).ਮਾਪ_2();
      ਪ੍ਰਤਿਜ੍ਞਾ(ਥਾਂ_Offs == ਵਿੱਥ && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    ਲਈ (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++ਅੰਕ_Pieces;
    ਮਾਪ += R.ਮਾਪ_2();
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  Rope_Piece_BTree_ਪੱਤੀ *New_ਗੰਢ = ਨਵਾਂ Rope_Piece_BTree_ਪੱਤੀ();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[ਚੌੜਾਈ_ਗੁਣਕ], &Pieces[2 * ਚੌੜਾਈ_ਗੁਣਕ],
            &New_ਗੰਢ->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[ਚੌੜਾਈ_ਗੁਣਕ], &Pieces[2 * ਚੌੜਾਈ_ਗੁਣਕ], RopePiece());

  // Decrease the number of values in the two nodes.
  New_ਗੰਢ->ਅੰਕ_Pieces = ਅੰਕ_Pieces = ਚੌੜਾਈ_ਗੁਣਕ;

  // Recompute the two nodes' size.
  New_ਗੰਢ->ਭਰਿਆ_Recompute_ਮਾਪ_Locally();
  ਭਰਿਆ_Recompute_ਮਾਪ_Locally();

  // Update the list of leaves.
  New_ਗੰਢ->ਪਾਓ_After_ਪੱਤੀ_ਵਿੱਚ_ਕ੍ਰਮ(ਇਹ);

  // These insertions can't fail.
  ਜੇ (ਇਹ->ਮਾਪ_2() >= ਵਿੱਥ)
    ਇਹ->ਪਾਓ(ਵਿੱਥ, R);
  ਨਹੀਂਤਾਂ
    New_ਗੰਢ->ਪਾਓ(ਵਿੱਥ - ਇਹ->ਮਾਪ_2(), R);
  ਵਾਪਸ New_ਗੰਢ;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
ਖਾਲੀ Rope_Piece_BTree_ਪੱਤੀ::erase(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਾਈਟਾਂ) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  ਬਿਨਾਨਿਸ਼ਾਨੀ PieceOffs = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0;
  ਲਈ (; ਵਿੱਥ > PieceOffs; ++i)
    PieceOffs += getPiece(i).ਮਾਪ_2();
  ਪ੍ਰਤਿਜ੍ਞਾ(PieceOffs == ਵਿੱਥ && "Split didn't occur before erase!");

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_Piece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  ਲਈ (; ਵਿੱਥ + ਅੰਕ_ਬਾਈਟਾਂ > PieceOffs + getPiece(i).ਮਾਪ_2(); ++i)
    PieceOffs += getPiece(i).ਮਾਪ_2();

  // If we exactly include the last one, include it in the region to delete.
  ਜੇ (ਵਿੱਥ + ਅੰਕ_ਬਾਈਟਾਂ == PieceOffs + getPiece(i).ਮਾਪ_2()) {
    PieceOffs += getPiece(i).ਮਾਪ_2();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  ਜੇ (i != ਸ਼ੁਰੂ_Piece) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Deleted = i - ਸ਼ੁਰੂ_Piece;
    ਲਈ (; i != get_ਅੰਕ_Pieces(); ++i)
      Pieces[i - ਅੰਕ_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_ਅੰਕ_Pieces() - ਅੰਕ_Deleted], &Pieces[get_ਅੰਕ_Pieces()],
              RopePiece());
    ਅੰਕ_Pieces -= ਅੰਕ_Deleted;

    ਬਿਨਾਨਿਸ਼ਾਨੀ Cover_ਬਾਈਟਾਂ = PieceOffs - ਵਿੱਥ;
    ਅੰਕ_ਬਾਈਟਾਂ -= Cover_ਬਾਈਟਾਂ;
    ਮਾਪ -= Cover_ਬਾਈਟਾਂ;
  }

  // If we completely removed some stuff, we could be done.
  ਜੇ (ਅੰਕ_ਬਾਈਟਾਂ == 0)
    ਵਾਪਸ;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  ਪ੍ਰਤਿਜ੍ਞਾ(getPiece(ਸ਼ੁਰੂ_Piece).ਮਾਪ_2() > ਅੰਕ_ਬਾਈਟਾਂ);
  Pieces[ਸ਼ੁਰੂ_Piece].ਸ਼ੁਰੂ_Offs += ਅੰਕ_ਬਾਈਟਾਂ;

  // The size of this node just shrunk by NumBytes.
  ਮਾਪ -= ਅੰਕ_ਬਾਈਟਾਂ;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

ਨਾਮਾਕਾਸ਼ {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
ਵਰਗ RopePieceBTreeInterior : ਜਨਤਕ Rope_Piece_BTree_ਗੰਢ {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਅੰਕ_Children = 0;

  Rope_Piece_BTree_ਗੰਢ *Children[2 * ਚੌੜਾਈ_ਗੁਣਕ];

ਜਨਤਕ:
  RopePieceBTreeInterior() : Rope_Piece_BTree_ਗੰਢ(ਝੂਠ) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_ਗੰਢ *LHS, Rope_Piece_BTree_ਗੰਢ *RHS)
      : Rope_Piece_BTree_ਗੰਢ(ਝੂਠ) {
    Children[0] = LHS;
    Children[1] = RHS;
    ਅੰਕ_Children = 2;
    ਮਾਪ = LHS->ਮਾਪ_2() + RHS->ਮਾਪ_2();
  }

  ~RopePieceBTreeInterior() {
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = get_ਅੰਕ_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  ਤਾਰਕਿਕ is_ਭਰਿਆ() ਅਚਲ { ਵਾਪਸ ਅੰਕ_Children == 2 * ਚੌੜਾਈ_ਗੁਣਕ; }

  ਬਿਨਾਨਿਸ਼ਾਨੀ get_ਅੰਕ_Children() ਅਚਲ { ਵਾਪਸ ਅੰਕ_Children; }

  ਅਚਲ Rope_Piece_BTree_ਗੰਢ *get_ਬੱਚਾ(ਬਿਨਾਨਿਸ਼ਾਨੀ i) ਅਚਲ {
    ਪ੍ਰਤਿਜ੍ਞਾ(i < ਅੰਕ_Children && "invalid child #");
    ਵਾਪਸ Children[i];
  }

  Rope_Piece_BTree_ਗੰਢ *get_ਬੱਚਾ(ਬਿਨਾਨਿਸ਼ਾਨੀ i) {
    ਪ੍ਰਤਿਜ੍ਞਾ(i < ਅੰਕ_Children && "invalid child #");
    ਵਾਪਸ Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  ਖਾਲੀ ਭਰਿਆ_Recompute_ਮਾਪ_Locally() {
    ਮਾਪ = 0;
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = get_ਅੰਕ_Children(); i != e; ++i)
      ਮਾਪ += get_ਬੱਚਾ(i)->ਮਾਪ_2();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ਗੰਢ *ਵੰਡੋ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ਗੰਢ *ਪਾਓ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ, ਅਚਲ RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_ਗੰਢ *Handle_ਬੱਚਾ_Piece(ਬਿਨਾਨਿਸ਼ਾਨੀ i, Rope_Piece_BTree_ਗੰਢ *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  ਖਾਲੀ erase(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਾਈਟਾਂ);

  ਸਥਿਰ ਤਾਰਕਿਕ classof(ਅਚਲ Rope_Piece_BTree_ਗੰਢ *N) { ਵਾਪਸ !N->is_ਪੱਤੀ(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ਗੰਢ *RopePieceBTreeInterior::ਵੰਡੋ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ) {
  // Figure out which child to split.
  ਜੇ (ਵਿੱਥ == 0 || ਵਿੱਥ == ਮਾਪ_2())
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ; // If we have an exact offset, we're already split.

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਬੱਚਾ_ਵਿੱਥ = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0;
  ਲਈ (; ਵਿੱਥ >= ਬੱਚਾ_ਵਿੱਥ + get_ਬੱਚਾ(i)->ਮਾਪ_2(); ++i)
    ਬੱਚਾ_ਵਿੱਥ += get_ਬੱਚਾ(i)->ਮਾਪ_2();

  // If already split there, we're done.
  ਜੇ (ਬੱਚਾ_ਵਿੱਥ == ਵਿੱਥ)
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

  // Otherwise, recursively split the child.
  ਜੇ (Rope_Piece_BTree_ਗੰਢ *RHS = get_ਬੱਚਾ(i)->ਵੰਡੋ(ਵਿੱਥ - ਬੱਚਾ_ਵਿੱਥ))
    ਵਾਪਸ Handle_ਬੱਚਾ_Piece(i, RHS);
  ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ਗੰਢ *RopePieceBTreeInterior::ਪਾਓ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ,
                                                   ਅਚਲ RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = get_ਅੰਕ_Children();

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਬੱਚਾ_Offs = 0;
  ਜੇ (ਵਿੱਥ == ਮਾਪ_2()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    ਬੱਚਾ_Offs = ਮਾਪ_2() - get_ਬੱਚਾ(i)->ਮਾਪ_2();
  } ਨਹੀਂਤਾਂ {
    ਲਈ (; ਵਿੱਥ > ਬੱਚਾ_Offs + get_ਬੱਚਾ(i)->ਮਾਪ_2(); ++i)
      ਬੱਚਾ_Offs += get_ਬੱਚਾ(i)->ਮਾਪ_2();
  }

  ਮਾਪ += R.ਮਾਪ_2();

  // Insert at the end of this child.
  ਜੇ (Rope_Piece_BTree_ਗੰਢ *RHS = get_ਬੱਚਾ(i)->ਪਾਓ(ਵਿੱਥ - ਬੱਚਾ_Offs, R))
    ਵਾਪਸ Handle_ਬੱਚਾ_Piece(i, RHS);

  ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_ਗੰਢ *
RopePieceBTreeInterior::Handle_ਬੱਚਾ_Piece(ਬਿਨਾਨਿਸ਼ਾਨੀ i, Rope_Piece_BTree_ਗੰਢ *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  ਜੇ (!is_ਭਰਿਆ()) {
    // Insert RHS after child 'i'.
    ਜੇ (i + 1 != get_ਅੰਕ_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_ਅੰਕ_Children() - i - 1) * ਆਕਾਰ(Children[0]));
    Children[i + 1] = RHS;
    ++ਅੰਕ_Children;
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_ਗੰਢ = ਨਵਾਂ RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&New_ਗੰਢ->Children[0], &Children[ਚੌੜਾਈ_ਗੁਣਕ],
         ਚੌੜਾਈ_ਗੁਣਕ * ਆਕਾਰ(Children[0]));

  // Decrease the number of values in the two nodes.
  New_ਗੰਢ->ਅੰਕ_Children = ਅੰਕ_Children = ਚੌੜਾਈ_ਗੁਣਕ;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  ਜੇ (i < ਚੌੜਾਈ_ਗੁਣਕ)
    ਇਹ->Handle_ਬੱਚਾ_Piece(i, RHS);
  ਨਹੀਂਤਾਂ
    New_ਗੰਢ->Handle_ਬੱਚਾ_Piece(i - ਚੌੜਾਈ_ਗੁਣਕ, RHS);

  // Recompute the two nodes' size.
  New_ਗੰਢ->ਭਰਿਆ_Recompute_ਮਾਪ_Locally();
  ਭਰਿਆ_Recompute_ਮਾਪ_Locally();
  ਵਾਪਸ New_ਗੰਢ;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
ਖਾਲੀ RopePieceBTreeInterior::erase(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਾਈਟਾਂ) {
  // This will shrink this node by NumBytes.
  ਮਾਪ -= ਅੰਕ_ਬਾਈਟਾਂ;

  // Find the first child that overlaps with Offset.
  ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0;
  ਲਈ (; ਵਿੱਥ >= get_ਬੱਚਾ(i)->ਮਾਪ_2(); ++i)
    ਵਿੱਥ -= get_ਬੱਚਾ(i)->ਮਾਪ_2();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  ਜਦੋਂ (ਅੰਕ_ਬਾਈਟਾਂ) {
    Rope_Piece_BTree_ਗੰਢ *Cur_ਬੱਚਾ = get_ਬੱਚਾ(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    ਜੇ (ਵਿੱਥ + ਅੰਕ_ਬਾਈਟਾਂ < Cur_ਬੱਚਾ->ਮਾਪ_2()) {
      Cur_ਬੱਚਾ->erase(ਵਿੱਥ, ਅੰਕ_ਬਾਈਟਾਂ);
      ਵਾਪਸ;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    ਜੇ (ਵਿੱਥ) {
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਈਟਾਂ_From_ਬੱਚਾ = Cur_ਬੱਚਾ->ਮਾਪ_2() - ਵਿੱਥ;
      Cur_ਬੱਚਾ->erase(ਵਿੱਥ, ਬਾਈਟਾਂ_From_ਬੱਚਾ);
      ਅੰਕ_ਬਾਈਟਾਂ -= ਬਾਈਟਾਂ_From_ਬੱਚਾ;
      // Start at the beginning of the next child.
      ਵਿੱਥ = 0;
      ++i;
      ਜਾਰੀ;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    ਅੰਕ_ਬਾਈਟਾਂ -= Cur_ਬੱਚਾ->ਮਾਪ_2();
    Cur_ਬੱਚਾ->Destroy();
    --ਅੰਕ_Children;
    ਜੇ (i != get_ਅੰਕ_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_ਅੰਕ_Children() - i) * ਆਕਾਰ(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

ਖਾਲੀ Rope_Piece_BTree_ਗੰਢ::Destroy() {
  ਜੇ (ਸਵੈਚਾਲਿਤ *ਪੱਤੀ = dyn_cast<Rope_Piece_BTree_ਪੱਤੀ>(ਇਹ))
    ਮਿਟਾਓ ਪੱਤੀ;
  ਨਹੀਂਤਾਂ
    ਮਿਟਾਓ cast<RopePieceBTreeInterior>(ਇਹ);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ਗੰਢ *Rope_Piece_BTree_ਗੰਢ::ਵੰਡੋ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਵਿੱਥ <= ਮਾਪ_2() && "Invalid offset to split!");
  ਜੇ (ਸਵੈਚਾਲਿਤ *ਪੱਤੀ = dyn_cast<Rope_Piece_BTree_ਪੱਤੀ>(ਇਹ))
    ਵਾਪਸ ਪੱਤੀ->ਵੰਡੋ(ਵਿੱਥ);
  ਵਾਪਸ cast<RopePieceBTreeInterior>(ਇਹ)->ਵੰਡੋ(ਵਿੱਥ);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ਗੰਢ *Rope_Piece_BTree_ਗੰਢ::ਪਾਓ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ,
                                               ਅਚਲ RopePiece &R) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਵਿੱਥ <= ਮਾਪ_2() && "Invalid offset to insert!");
  ਜੇ (ਸਵੈਚਾਲਿਤ *ਪੱਤੀ = dyn_cast<Rope_Piece_BTree_ਪੱਤੀ>(ਇਹ))
    ਵਾਪਸ ਪੱਤੀ->ਪਾਓ(ਵਿੱਥ, R);
  ਵਾਪਸ cast<RopePieceBTreeInterior>(ਇਹ)->ਪਾਓ(ਵਿੱਥ, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
ਖਾਲੀ Rope_Piece_BTree_ਗੰਢ::erase(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਾਈਟਾਂ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਵਿੱਥ + ਅੰਕ_ਬਾਈਟਾਂ <= ਮਾਪ_2() && "Invalid offset to erase!");
  ਜੇ (ਸਵੈਚਾਲਿਤ *ਪੱਤੀ = dyn_cast<Rope_Piece_BTree_ਪੱਤੀ>(ਇਹ))
    ਵਾਪਸ ਪੱਤੀ->erase(ਵਿੱਥ, ਅੰਕ_ਬਾਈਟਾਂ);
  ਵਾਪਸ cast<RopePieceBTreeInterior>(ਇਹ)->erase(ਵਿੱਥ, ਅੰਕ_ਬਾਈਟਾਂ);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

ਸਥਿਰ ਅਚਲ Rope_Piece_BTree_ਪੱਤੀ *getCN(ਅਚਲ ਖਾਲੀ *P) {
  ਵਾਪਸ static_cast<ਅਚਲ Rope_Piece_BTree_ਪੱਤੀ *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(ਅਚਲ ਖਾਲੀ *n) {
  ਅਚਲ ਸਵੈਚਾਲਿਤ *N = static_cast<ਅਚਲ Rope_Piece_BTree_ਗੰਢ *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  ਜਦੋਂ (ਅਚਲ ਸਵੈਚਾਲਿਤ *ਵਿੱਚ_2 = dyn_cast<RopePieceBTreeInterior>(N))
    N = ਵਿੱਚ_2->get_ਬੱਚਾ(0);

  // We must have at least one leaf.
  Cur_ਗੰਢ = cast<Rope_Piece_BTree_ਪੱਤੀ>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  ਜਦੋਂ (Cur_ਗੰਢ && getCN(Cur_ਗੰਢ)->get_ਅੰਕ_Pieces() == 0)
    Cur_ਗੰਢ = getCN(Cur_ਗੰਢ)->get_ਅਗਲਾ_ਪੱਤੀ_ਵਿੱਚ_ਕ੍ਰਮ();

  ਜੇ (Cur_ਗੰਢ)
    CurPiece = &getCN(Cur_ਗੰਢ)->getPiece(0);
  ਨਹੀਂਤਾਂ // Empty tree, this is an end() iterator.
    CurPiece = ਖਾਲੀ_ਸੂਚਕ;
  CurChar = 0;
}

ਖਾਲੀ RopePieceBTreeIterator::Move_ਮੰਜ਼ਿਲ_ਅਗਲਾ_Piece() {
  ਜੇ (CurPiece !=
      &getCN(Cur_ਗੰਢ)->getPiece(getCN(Cur_ਗੰਢ)->get_ਅੰਕ_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    ਵਾਪਸ;
  }

  // Find the next non-empty leaf node.
  ਕਰੋ
    Cur_ਗੰਢ = getCN(Cur_ਗੰਢ)->get_ਅਗਲਾ_ਪੱਤੀ_ਵਿੱਚ_ਕ੍ਰਮ();
  ਜਦੋਂ (Cur_ਗੰਢ && getCN(Cur_ਗੰਢ)->get_ਅੰਕ_Pieces() == 0);

  ਜੇ (Cur_ਗੰਢ)
    CurPiece = &getCN(Cur_ਗੰਢ)->getPiece(0);
  ਨਹੀਂਤਾਂ // Hit end().
    CurPiece = ਖਾਲੀ_ਸੂਚਕ;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

ਸਥਿਰ Rope_Piece_BTree_ਗੰਢ *get_ਜੜ੍ਹ(ਖਾਲੀ *P) {
  ਵਾਪਸ static_cast<Rope_Piece_BTree_ਗੰਢ *>(P);
}

RopePieceBTree::RopePieceBTree() { ਜੜ੍ਹ_2 = ਨਵਾਂ Rope_Piece_BTree_ਪੱਤੀ(); }

RopePieceBTree::RopePieceBTree(ਅਚਲ RopePieceBTree &RHS) {
  ਪ੍ਰਤਿਜ੍ਞਾ(RHS.ਖਾਲੀ_ਥਾਵਾਂ() && "Can't copy non-empty tree yet");
  ਜੜ੍ਹ_2 = ਨਵਾਂ Rope_Piece_BTree_ਪੱਤੀ();
}

RopePieceBTree::~RopePieceBTree() { get_ਜੜ੍ਹ(ਜੜ੍ਹ_2)->Destroy(); }

ਬਿਨਾਨਿਸ਼ਾਨੀ RopePieceBTree::ਮਾਪ_2() ਅਚਲ { ਵਾਪਸ get_ਜੜ੍ਹ(ਜੜ੍ਹ_2)->ਮਾਪ_2(); }

ਖਾਲੀ RopePieceBTree::clear() {
  ਜੇ (ਸਵੈਚਾਲਿਤ *ਪੱਤੀ = dyn_cast<Rope_Piece_BTree_ਪੱਤੀ>(get_ਜੜ੍ਹ(ਜੜ੍ਹ_2)))
    ਪੱਤੀ->clear();
  ਨਹੀਂਤਾਂ {
    get_ਜੜ੍ਹ(ਜੜ੍ਹ_2)->Destroy();
    ਜੜ੍ਹ_2 = ਨਵਾਂ Rope_Piece_BTree_ਪੱਤੀ();
  }
}

ਖਾਲੀ RopePieceBTree::ਪਾਓ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ, ਅਚਲ RopePiece &R) {
  // #1. Split at Offset.
  ਜੇ (Rope_Piece_BTree_ਗੰਢ *RHS = get_ਜੜ੍ਹ(ਜੜ੍ਹ_2)->ਵੰਡੋ(ਵਿੱਥ))
    ਜੜ੍ਹ_2 = ਨਵਾਂ RopePieceBTreeInterior(get_ਜੜ੍ਹ(ਜੜ੍ਹ_2), RHS);

  // #2. Do the insertion.
  ਜੇ (Rope_Piece_BTree_ਗੰਢ *RHS = get_ਜੜ੍ਹ(ਜੜ੍ਹ_2)->ਪਾਓ(ਵਿੱਥ, R))
    ਜੜ੍ਹ_2 = ਨਵਾਂ RopePieceBTreeInterior(get_ਜੜ੍ਹ(ਜੜ੍ਹ_2), RHS);
}

ਖਾਲੀ RopePieceBTree::erase(ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਾਈਟਾਂ) {
  // #1. Split at Offset.
  ਜੇ (Rope_Piece_BTree_ਗੰਢ *RHS = get_ਜੜ੍ਹ(ਜੜ੍ਹ_2)->ਵੰਡੋ(ਵਿੱਥ))
    ਜੜ੍ਹ_2 = ਨਵਾਂ RopePieceBTreeInterior(get_ਜੜ੍ਹ(ਜੜ੍ਹ_2), RHS);

  // #2. Do the erasing.
  get_ਜੜ੍ਹ(ਜੜ੍ਹ_2)->erase(ਵਿੱਥ, ਅੰਕ_ਬਾਈਟਾਂ);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(ਅਚਲ ਅੱਖਰ *ਸ਼ੁਰੂ, ਅਚਲ ਅੱਖਰ *ਸਿਰਾ_2) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ Len = ਸਿਰਾ_2 - ਸ਼ੁਰੂ;
  ਪ੍ਰਤਿਜ੍ਞਾ(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  ਜੇ (AllocOffs + Len <= Alloc_Chunk_ਮਾਪ) {
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(Alloc_ਬਫਰ->ਅੰਕੜੇ_2 + AllocOffs, ਸ਼ੁਰੂ, Len);
    AllocOffs += Len;
    ਵਾਪਸ RopePiece(Alloc_ਬਫਰ, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  ਜੇ (Len > Alloc_Chunk_ਮਾਪ) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ = ਸਿਰਾ_2 - ਸ਼ੁਰੂ + ਆਕਾਰ(Rope_Ref_ਗਿਣਤੀ_String) - 1;
    ਸਵੈਚਾਲਿਤ *Res = reinterpret_cast<Rope_Ref_ਗਿਣਤੀ_String *>(ਨਵਾਂ ਅੱਖਰ[ਮਾਪ]);
    Res->Ref_ਗਿਣਤੀ = 0;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(Res->ਅੰਕੜੇ_2, ਸ਼ੁਰੂ, ਸਿਰਾ_2 - ਸ਼ੁਰੂ);
    ਵਾਪਸ RopePiece(Res, 0, ਸਿਰਾ_2 - ਸ਼ੁਰੂ);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  ਬਿਨਾਨਿਸ਼ਾਨੀ Alloc_ਮਾਪ = offsetof(Rope_Ref_ਗਿਣਤੀ_String, ਅੰਕੜੇ_2) + Alloc_Chunk_ਮਾਪ;
  ਸਵੈਚਾਲਿਤ *Res = reinterpret_cast<Rope_Ref_ਗਿਣਤੀ_String *>(ਨਵਾਂ ਅੱਖਰ[Alloc_ਮਾਪ]);
  Res->Ref_ਗਿਣਤੀ = 0;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(Res->ਅੰਕੜੇ_2, ਸ਼ੁਰੂ, Len);
  Alloc_ਬਫਰ = Res;
  AllocOffs = Len;

  ਵਾਪਸ RopePiece(Alloc_ਬਫਰ, 0, Len);
}
