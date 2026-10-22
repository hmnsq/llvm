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

#ضمن "llvm/ADT/RewriteRope.h"
#ضمن "llvm/Support/Casting.h"
#ضمن <algorithm>
#ضمن <cassert>
#ضمن <cstring>

ورتوں نطاق llvm;

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

نطاق {

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Class
//===----------------------------------------------------------------------===//

/// RopePieceBTreeNode - Common base class of RopePieceBTreeLeaf and
/// RopePieceBTreeInterior.  This provides some 'virtual' dispatching methods
/// and a flag that determines which subclass the instance is.  Also
/// important, this node knows the full extend of the node, including any
/// children that it has.  This allows efficient skipping over entire subtrees
/// when looking for an offset in the BTree.
صنف Rope_Piece_BTree_گرہ {
محمي:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  تعداد { چوڑائی_ضارب = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  بنانشانی ناپ = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  منطقي Is_پتی;

  Rope_Piece_BTree_گرہ(منطقي is_پتی) : Is_پتی(is_پتی) {}
  ~Rope_Piece_BTree_گرہ() = افتراضي;

عام:
  منطقي is_پتی() ثابت { أرجع Is_پتی; }
  بنانشانی ناپ_2() ثابت { أرجع ناپ; }

  فراغ Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گرہ *ونڈو(بنانشانی وتھ);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گرہ *داخل_کرو(بنانشانی وتھ, ثابت RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  فراغ erase(بنانشانی وتھ, بنانشانی عدد_بائٹاں);
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
صنف Rope_Piece_BTree_پتی : عام Rope_Piece_BTree_گرہ {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  بنانشانی حرف عدد_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * چوڑائی_ضارب];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  Rope_Piece_BTree_پتی **Prev_پتی = خالی_سوچک;
  Rope_Piece_BTree_پتی *اگلا_پتی = خالی_سوچک;

عام:
  Rope_Piece_BTree_پتی() : Rope_Piece_BTree_گرہ(سچ) {}

  ~Rope_Piece_BTree_پتی() {
    إذا (Prev_پتی || اگلا_پتی)
      remove_From_پتی_وچ_ترتیب();
    clear();
  }

  منطقي is_بھریا() ثابت { أرجع عدد_Pieces == 2 * چوڑائی_ضارب; }

  /// clear - Remove all rope pieces from this leaf.
  فراغ clear() {
    طالما (عدد_Pieces)
      Pieces[--عدد_Pieces] = RopePiece();
    ناپ = 0;
  }

  بنانشانی get_عدد_Pieces() ثابت { أرجع عدد_Pieces; }

  ثابت RopePiece &getPiece(بنانشانی i) ثابت {
    أكد(i < get_عدد_Pieces() && "Invalid piece ID");
    أرجع Pieces[i];
  }

  ثابت Rope_Piece_BTree_پتی *get_اگلا_پتی_وچ_ترتیب() ثابت { أرجع اگلا_پتی; }

  فراغ داخل_کرو_After_پتی_وچ_ترتیب(Rope_Piece_BTree_پتی *گرہ) {
    أكد(!Prev_پتی && !اگلا_پتی && "Already in ordering");

    اگلا_پتی = گرہ->اگلا_پتی;
    إذا (اگلا_پتی)
      اگلا_پتی->Prev_پتی = &اگلا_پتی;
    Prev_پتی = &گرہ->اگلا_پتی;
    گرہ->اگلا_پتی = هذا;
  }

  فراغ remove_From_پتی_وچ_ترتیب() {
    إذا (Prev_پتی) {
      *Prev_پتی = اگلا_پتی;
      إذا (اگلا_پتی)
        اگلا_پتی->Prev_پتی = Prev_پتی;
    } وإلا إذا (اگلا_پتی) {
      اگلا_پتی->Prev_پتی = خالی_سوچک;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  فراغ بھریا_Recompute_ناپ_Locally() {
    ناپ = 0;
    لكل (بنانشانی i = 0, e = get_عدد_Pieces(); i != e; ++i)
      ناپ += getPiece(i).ناپ_2();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گرہ *ونڈو(بنانشانی وتھ);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گرہ *داخل_کرو(بنانشانی وتھ, ثابت RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  فراغ erase(بنانشانی وتھ, بنانشانی عدد_بائٹاں);

  ساكن منطقي classof(ثابت Rope_Piece_BTree_گرہ *N) { أرجع N->is_پتی(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گرہ *Rope_Piece_BTree_پتی::ونڈو(بنانشانی وتھ) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  إذا (وتھ == 0 || وتھ == ناپ_2()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    أرجع خالی_سوچک;
  }

  // Find the piece that this offset lands in.
  بنانشانی PieceOffs = 0;
  بنانشانی i = 0;
  طالما (وتھ >= PieceOffs + Pieces[i].ناپ_2()) {
    PieceOffs += Pieces[i].ناپ_2();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  إذا (PieceOffs == وتھ)
    أرجع خالی_سوچک;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  بنانشانی Intra_Piece_وتھ = وتھ - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece پوچھ(Pieces[i].Str_انکڑے, Pieces[i].شروع_Offs + Intra_Piece_وتھ,
                 Pieces[i].سرا_Offs);
  ناپ -= Pieces[i].ناپ_2();
  Pieces[i].سرا_Offs = Pieces[i].شروع_Offs + Intra_Piece_وتھ;
  ناپ += Pieces[i].ناپ_2();

  أرجع داخل_کرو(وتھ, پوچھ);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گرہ *Rope_Piece_BTree_پتی::داخل_کرو(بنانشانی وتھ,
                                               ثابت RopePiece &R) {
  // If this node is not full, insert the piece.
  إذا (!is_بھریا()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    بنانشانی i = 0, e = get_عدد_Pieces();
    إذا (وتھ == ناپ_2()) {
      // Fastpath for a common case.
      i = e;
    } وإلا {
      بنانشانی تھاں_Offs = 0;
      لكل (; وتھ > تھاں_Offs; ++i)
        تھاں_Offs += getPiece(i).ناپ_2();
      أكد(تھاں_Offs == وتھ && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    لكل (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++عدد_Pieces;
    ناپ += R.ناپ_2();
    أرجع خالی_سوچک;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  Rope_Piece_BTree_پتی *New_گرہ = جديد Rope_Piece_BTree_پتی();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[چوڑائی_ضارب], &Pieces[2 * چوڑائی_ضارب],
            &New_گرہ->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[چوڑائی_ضارب], &Pieces[2 * چوڑائی_ضارب], RopePiece());

  // Decrease the number of values in the two nodes.
  New_گرہ->عدد_Pieces = عدد_Pieces = چوڑائی_ضارب;

  // Recompute the two nodes' size.
  New_گرہ->بھریا_Recompute_ناپ_Locally();
  بھریا_Recompute_ناپ_Locally();

  // Update the list of leaves.
  New_گرہ->داخل_کرو_After_پتی_وچ_ترتیب(هذا);

  // These insertions can't fail.
  إذا (هذا->ناپ_2() >= وتھ)
    هذا->داخل_کرو(وتھ, R);
  وإلا
    New_گرہ->داخل_کرو(وتھ - هذا->ناپ_2(), R);
  أرجع New_گرہ;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
فراغ Rope_Piece_BTree_پتی::erase(بنانشانی وتھ, بنانشانی عدد_بائٹاں) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  بنانشانی PieceOffs = 0;
  بنانشانی i = 0;
  لكل (; وتھ > PieceOffs; ++i)
    PieceOffs += getPiece(i).ناپ_2();
  أكد(PieceOffs == وتھ && "Split didn't occur before erase!");

  بنانشانی شروع_Piece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  لكل (; وتھ + عدد_بائٹاں > PieceOffs + getPiece(i).ناپ_2(); ++i)
    PieceOffs += getPiece(i).ناپ_2();

  // If we exactly include the last one, include it in the region to delete.
  إذا (وتھ + عدد_بائٹاں == PieceOffs + getPiece(i).ناپ_2()) {
    PieceOffs += getPiece(i).ناپ_2();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  إذا (i != شروع_Piece) {
    بنانشانی عدد_Deleted = i - شروع_Piece;
    لكل (; i != get_عدد_Pieces(); ++i)
      Pieces[i - عدد_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_عدد_Pieces() - عدد_Deleted], &Pieces[get_عدد_Pieces()],
              RopePiece());
    عدد_Pieces -= عدد_Deleted;

    بنانشانی Cover_بائٹاں = PieceOffs - وتھ;
    عدد_بائٹاں -= Cover_بائٹاں;
    ناپ -= Cover_بائٹاں;
  }

  // If we completely removed some stuff, we could be done.
  إذا (عدد_بائٹاں == 0)
    أرجع;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  أكد(getPiece(شروع_Piece).ناپ_2() > عدد_بائٹاں);
  Pieces[شروع_Piece].شروع_Offs += عدد_بائٹاں;

  // The size of this node just shrunk by NumBytes.
  ناپ -= عدد_بائٹاں;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

نطاق {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
صنف RopePieceBTreeInterior : عام Rope_Piece_BTree_گرہ {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  بنانشانی حرف عدد_Children = 0;

  Rope_Piece_BTree_گرہ *Children[2 * چوڑائی_ضارب];

عام:
  RopePieceBTreeInterior() : Rope_Piece_BTree_گرہ(خطأ) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_گرہ *LHS, Rope_Piece_BTree_گرہ *RHS)
      : Rope_Piece_BTree_گرہ(خطأ) {
    Children[0] = LHS;
    Children[1] = RHS;
    عدد_Children = 2;
    ناپ = LHS->ناپ_2() + RHS->ناپ_2();
  }

  ~RopePieceBTreeInterior() {
    لكل (بنانشانی i = 0, e = get_عدد_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  منطقي is_بھریا() ثابت { أرجع عدد_Children == 2 * چوڑائی_ضارب; }

  بنانشانی get_عدد_Children() ثابت { أرجع عدد_Children; }

  ثابت Rope_Piece_BTree_گرہ *get_بچہ(بنانشانی i) ثابت {
    أكد(i < عدد_Children && "invalid child #");
    أرجع Children[i];
  }

  Rope_Piece_BTree_گرہ *get_بچہ(بنانشانی i) {
    أكد(i < عدد_Children && "invalid child #");
    أرجع Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  فراغ بھریا_Recompute_ناپ_Locally() {
    ناپ = 0;
    لكل (بنانشانی i = 0, e = get_عدد_Children(); i != e; ++i)
      ناپ += get_بچہ(i)->ناپ_2();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گرہ *ونڈو(بنانشانی وتھ);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گرہ *داخل_کرو(بنانشانی وتھ, ثابت RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_گرہ *Handle_بچہ_Piece(بنانشانی i, Rope_Piece_BTree_گرہ *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  فراغ erase(بنانشانی وتھ, بنانشانی عدد_بائٹاں);

  ساكن منطقي classof(ثابت Rope_Piece_BTree_گرہ *N) { أرجع !N->is_پتی(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گرہ *RopePieceBTreeInterior::ونڈو(بنانشانی وتھ) {
  // Figure out which child to split.
  إذا (وتھ == 0 || وتھ == ناپ_2())
    أرجع خالی_سوچک; // If we have an exact offset, we're already split.

  بنانشانی بچہ_وتھ = 0;
  بنانشانی i = 0;
  لكل (; وتھ >= بچہ_وتھ + get_بچہ(i)->ناپ_2(); ++i)
    بچہ_وتھ += get_بچہ(i)->ناپ_2();

  // If already split there, we're done.
  إذا (بچہ_وتھ == وتھ)
    أرجع خالی_سوچک;

  // Otherwise, recursively split the child.
  إذا (Rope_Piece_BTree_گرہ *RHS = get_بچہ(i)->ونڈو(وتھ - بچہ_وتھ))
    أرجع Handle_بچہ_Piece(i, RHS);
  أرجع خالی_سوچک; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گرہ *RopePieceBTreeInterior::داخل_کرو(بنانشانی وتھ,
                                                   ثابت RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  بنانشانی i = 0, e = get_عدد_Children();

  بنانشانی بچہ_Offs = 0;
  إذا (وتھ == ناپ_2()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    بچہ_Offs = ناپ_2() - get_بچہ(i)->ناپ_2();
  } وإلا {
    لكل (; وتھ > بچہ_Offs + get_بچہ(i)->ناپ_2(); ++i)
      بچہ_Offs += get_بچہ(i)->ناپ_2();
  }

  ناپ += R.ناپ_2();

  // Insert at the end of this child.
  إذا (Rope_Piece_BTree_گرہ *RHS = get_بچہ(i)->داخل_کرو(وتھ - بچہ_Offs, R))
    أرجع Handle_بچہ_Piece(i, RHS);

  أرجع خالی_سوچک;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_گرہ *
RopePieceBTreeInterior::Handle_بچہ_Piece(بنانشانی i, Rope_Piece_BTree_گرہ *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  إذا (!is_بھریا()) {
    // Insert RHS after child 'i'.
    إذا (i + 1 != get_عدد_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_عدد_Children() - i - 1) * حجم(Children[0]));
    Children[i + 1] = RHS;
    ++عدد_Children;
    أرجع خالی_سوچک;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_گرہ = جديد RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  حافظہ_نقل(&New_گرہ->Children[0], &Children[چوڑائی_ضارب],
         چوڑائی_ضارب * حجم(Children[0]));

  // Decrease the number of values in the two nodes.
  New_گرہ->عدد_Children = عدد_Children = چوڑائی_ضارب;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  إذا (i < چوڑائی_ضارب)
    هذا->Handle_بچہ_Piece(i, RHS);
  وإلا
    New_گرہ->Handle_بچہ_Piece(i - چوڑائی_ضارب, RHS);

  // Recompute the two nodes' size.
  New_گرہ->بھریا_Recompute_ناپ_Locally();
  بھریا_Recompute_ناپ_Locally();
  أرجع New_گرہ;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
فراغ RopePieceBTreeInterior::erase(بنانشانی وتھ, بنانشانی عدد_بائٹاں) {
  // This will shrink this node by NumBytes.
  ناپ -= عدد_بائٹاں;

  // Find the first child that overlaps with Offset.
  بنانشانی i = 0;
  لكل (; وتھ >= get_بچہ(i)->ناپ_2(); ++i)
    وتھ -= get_بچہ(i)->ناپ_2();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  طالما (عدد_بائٹاں) {
    Rope_Piece_BTree_گرہ *Cur_بچہ = get_بچہ(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    إذا (وتھ + عدد_بائٹاں < Cur_بچہ->ناپ_2()) {
      Cur_بچہ->erase(وتھ, عدد_بائٹاں);
      أرجع;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    إذا (وتھ) {
      بنانشانی بائٹاں_From_بچہ = Cur_بچہ->ناپ_2() - وتھ;
      Cur_بچہ->erase(وتھ, بائٹاں_From_بچہ);
      عدد_بائٹاں -= بائٹاں_From_بچہ;
      // Start at the beginning of the next child.
      وتھ = 0;
      ++i;
      استمر;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    عدد_بائٹاں -= Cur_بچہ->ناپ_2();
    Cur_بچہ->Destroy();
    --عدد_Children;
    إذا (i != get_عدد_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_عدد_Children() - i) * حجم(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

فراغ Rope_Piece_BTree_گرہ::Destroy() {
  إذا (خودکار *پتی = dyn_cast<Rope_Piece_BTree_پتی>(هذا))
    مٹاؤ پتی;
  وإلا
    مٹاؤ cast<RopePieceBTreeInterior>(هذا);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گرہ *Rope_Piece_BTree_گرہ::ونڈو(بنانشانی وتھ) {
  أكد(وتھ <= ناپ_2() && "Invalid offset to split!");
  إذا (خودکار *پتی = dyn_cast<Rope_Piece_BTree_پتی>(هذا))
    أرجع پتی->ونڈو(وتھ);
  أرجع cast<RopePieceBTreeInterior>(هذا)->ونڈو(وتھ);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گرہ *Rope_Piece_BTree_گرہ::داخل_کرو(بنانشانی وتھ,
                                               ثابت RopePiece &R) {
  أكد(وتھ <= ناپ_2() && "Invalid offset to insert!");
  إذا (خودکار *پتی = dyn_cast<Rope_Piece_BTree_پتی>(هذا))
    أرجع پتی->داخل_کرو(وتھ, R);
  أرجع cast<RopePieceBTreeInterior>(هذا)->داخل_کرو(وتھ, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
فراغ Rope_Piece_BTree_گرہ::erase(بنانشانی وتھ, بنانشانی عدد_بائٹاں) {
  أكد(وتھ + عدد_بائٹاں <= ناپ_2() && "Invalid offset to erase!");
  إذا (خودکار *پتی = dyn_cast<Rope_Piece_BTree_پتی>(هذا))
    أرجع پتی->erase(وتھ, عدد_بائٹاں);
  أرجع cast<RopePieceBTreeInterior>(هذا)->erase(وتھ, عدد_بائٹاں);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

ساكن ثابت Rope_Piece_BTree_پتی *getCN(ثابت فراغ *P) {
  أرجع static_cast<ثابت Rope_Piece_BTree_پتی *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(ثابت فراغ *n) {
  ثابت خودکار *N = static_cast<ثابت Rope_Piece_BTree_گرہ *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  طالما (ثابت خودکار *وچ = dyn_cast<RopePieceBTreeInterior>(N))
    N = وچ->get_بچہ(0);

  // We must have at least one leaf.
  Cur_گرہ = cast<Rope_Piece_BTree_پتی>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  طالما (Cur_گرہ && getCN(Cur_گرہ)->get_عدد_Pieces() == 0)
    Cur_گرہ = getCN(Cur_گرہ)->get_اگلا_پتی_وچ_ترتیب();

  إذا (Cur_گرہ)
    CurPiece = &getCN(Cur_گرہ)->getPiece(0);
  وإلا // Empty tree, this is an end() iterator.
    CurPiece = خالی_سوچک;
  CurChar = 0;
}

فراغ RopePieceBTreeIterator::Move_منزل_اگلا_Piece() {
  إذا (CurPiece !=
      &getCN(Cur_گرہ)->getPiece(getCN(Cur_گرہ)->get_عدد_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    أرجع;
  }

  // Find the next non-empty leaf node.
  افعل
    Cur_گرہ = getCN(Cur_گرہ)->get_اگلا_پتی_وچ_ترتیب();
  طالما (Cur_گرہ && getCN(Cur_گرہ)->get_عدد_Pieces() == 0);

  إذا (Cur_گرہ)
    CurPiece = &getCN(Cur_گرہ)->getPiece(0);
  وإلا // Hit end().
    CurPiece = خالی_سوچک;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

ساكن Rope_Piece_BTree_گرہ *get_جڑ(فراغ *P) {
  أرجع static_cast<Rope_Piece_BTree_گرہ *>(P);
}

RopePieceBTree::RopePieceBTree() { جڑ_2 = جديد Rope_Piece_BTree_پتی(); }

RopePieceBTree::RopePieceBTree(ثابت RopePieceBTree &RHS) {
  أكد(RHS.خالی_تھاواں() && "Can't copy non-empty tree yet");
  جڑ_2 = جديد Rope_Piece_BTree_پتی();
}

RopePieceBTree::~RopePieceBTree() { get_جڑ(جڑ_2)->Destroy(); }

بنانشانی RopePieceBTree::ناپ_2() ثابت { أرجع get_جڑ(جڑ_2)->ناپ_2(); }

فراغ RopePieceBTree::clear() {
  إذا (خودکار *پتی = dyn_cast<Rope_Piece_BTree_پتی>(get_جڑ(جڑ_2)))
    پتی->clear();
  وإلا {
    get_جڑ(جڑ_2)->Destroy();
    جڑ_2 = جديد Rope_Piece_BTree_پتی();
  }
}

فراغ RopePieceBTree::داخل_کرو(بنانشانی وتھ, ثابت RopePiece &R) {
  // #1. Split at Offset.
  إذا (Rope_Piece_BTree_گرہ *RHS = get_جڑ(جڑ_2)->ونڈو(وتھ))
    جڑ_2 = جديد RopePieceBTreeInterior(get_جڑ(جڑ_2), RHS);

  // #2. Do the insertion.
  إذا (Rope_Piece_BTree_گرہ *RHS = get_جڑ(جڑ_2)->داخل_کرو(وتھ, R))
    جڑ_2 = جديد RopePieceBTreeInterior(get_جڑ(جڑ_2), RHS);
}

فراغ RopePieceBTree::erase(بنانشانی وتھ, بنانشانی عدد_بائٹاں) {
  // #1. Split at Offset.
  إذا (Rope_Piece_BTree_گرہ *RHS = get_جڑ(جڑ_2)->ونڈو(وتھ))
    جڑ_2 = جديد RopePieceBTreeInterior(get_جڑ(جڑ_2), RHS);

  // #2. Do the erasing.
  get_جڑ(جڑ_2)->erase(وتھ, عدد_بائٹاں);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(ثابت حرف *شروع, ثابت حرف *سرا_2) {
  بنانشانی Len = سرا_2 - شروع;
  أكد(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  إذا (AllocOffs + Len <= Alloc_Chunk_ناپ) {
    حافظہ_نقل(Alloc_بفر->انکڑے_2 + AllocOffs, شروع, Len);
    AllocOffs += Len;
    أرجع RopePiece(Alloc_بفر, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  إذا (Len > Alloc_Chunk_ناپ) {
    بنانشانی ناپ = سرا_2 - شروع + حجم(Rope_Ref_گنتی_String) - 1;
    خودکار *Res = reinterpret_cast<Rope_Ref_گنتی_String *>(جديد حرف[ناپ]);
    Res->Ref_گنتی = 0;
    حافظہ_نقل(Res->انکڑے_2, شروع, سرا_2 - شروع);
    أرجع RopePiece(Res, 0, سرا_2 - شروع);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  بنانشانی Alloc_ناپ = offsetof(Rope_Ref_گنتی_String, انکڑے_2) + Alloc_Chunk_ناپ;
  خودکار *Res = reinterpret_cast<Rope_Ref_گنتی_String *>(جديد حرف[Alloc_ناپ]);
  Res->Ref_گنتی = 0;
  حافظہ_نقل(Res->انکڑے_2, شروع, Len);
  Alloc_بفر = Res;
  AllocOffs = Len;

  أرجع RopePiece(Alloc_بفر, 0, Len);
}
