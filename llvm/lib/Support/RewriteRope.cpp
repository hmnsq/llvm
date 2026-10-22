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

استعمال نطاق llvm;

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
صنف Rope_Piece_BTree_ڳنڍ {
محمي:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  تعداد { ويڪر_ضرب_ڪندڙ = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  بنانشان سائيز = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  منطقي Is_پن;

  Rope_Piece_BTree_ڳنڍ(منطقي is_پن) : Is_پن(is_پن) {}
  ~Rope_Piece_BTree_ڳنڍ() = افتراضي;

عام:
  منطقي is_پن() ثابت { أرجع Is_پن; }
  بنانشان سائيز_2() ثابت { أرجع سائيز; }

  فراغ Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ڳنڍ *ورهايو(بنانشان وٿي);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ڳنڍ *وجهو(بنانشان وٿي, ثابت RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  فراغ erase(بنانشان وٿي, بنانشان انگ_بائيٽون);
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
صنف Rope_Piece_BTree_پن : عام Rope_Piece_BTree_ڳنڍ {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  بنانشان حرف انگ_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * ويڪر_ضرب_ڪندڙ];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  Rope_Piece_BTree_پن **Prev_پن = خالي_اشارو;
  Rope_Piece_BTree_پن *ايندڙ_پن = خالي_اشارو;

عام:
  Rope_Piece_BTree_پن() : Rope_Piece_BTree_ڳنڍ(سچ) {}

  ~Rope_Piece_BTree_پن() {
    إذا (Prev_پن || ايندڙ_پن)
      remove_From_پن_اندريون_ترتيب();
    clear();
  }

  منطقي is_ڀريل() ثابت { أرجع انگ_Pieces == 2 * ويڪر_ضرب_ڪندڙ; }

  /// clear - Remove all rope pieces from this leaf.
  فراغ clear() {
    طالما (انگ_Pieces)
      Pieces[--انگ_Pieces] = RopePiece();
    سائيز = 0;
  }

  بنانشان get_انگ_Pieces() ثابت { أرجع انگ_Pieces; }

  ثابت RopePiece &getPiece(بنانشان i) ثابت {
    أكد(i < get_انگ_Pieces() && "Invalid piece ID");
    أرجع Pieces[i];
  }

  ثابت Rope_Piece_BTree_پن *get_ايندڙ_پن_اندريون_ترتيب() ثابت { أرجع ايندڙ_پن; }

  فراغ وجهو_After_پن_اندريون_ترتيب(Rope_Piece_BTree_پن *ڳنڍ) {
    أكد(!Prev_پن && !ايندڙ_پن && "Already in ordering");

    ايندڙ_پن = ڳنڍ->ايندڙ_پن;
    إذا (ايندڙ_پن)
      ايندڙ_پن->Prev_پن = &ايندڙ_پن;
    Prev_پن = &ڳنڍ->ايندڙ_پن;
    ڳنڍ->ايندڙ_پن = هذا;
  }

  فراغ remove_From_پن_اندريون_ترتيب() {
    إذا (Prev_پن) {
      *Prev_پن = ايندڙ_پن;
      إذا (ايندڙ_پن)
        ايندڙ_پن->Prev_پن = Prev_پن;
    } وإلا إذا (ايندڙ_پن) {
      ايندڙ_پن->Prev_پن = خالي_اشارو;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  فراغ ڀريل_Recompute_سائيز_Locally() {
    سائيز = 0;
    لكل (بنانشان i = 0, e = get_انگ_Pieces(); i != e; ++i)
      سائيز += getPiece(i).سائيز_2();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ڳنڍ *ورهايو(بنانشان وٿي);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ڳنڍ *وجهو(بنانشان وٿي, ثابت RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  فراغ erase(بنانشان وٿي, بنانشان انگ_بائيٽون);

  ساكن منطقي classof(ثابت Rope_Piece_BTree_ڳنڍ *N) { أرجع N->is_پن(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ڳنڍ *Rope_Piece_BTree_پن::ورهايو(بنانشان وٿي) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  إذا (وٿي == 0 || وٿي == سائيز_2()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    أرجع خالي_اشارو;
  }

  // Find the piece that this offset lands in.
  بنانشان PieceOffs = 0;
  بنانشان i = 0;
  طالما (وٿي >= PieceOffs + Pieces[i].سائيز_2()) {
    PieceOffs += Pieces[i].سائيز_2();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  إذا (PieceOffs == وٿي)
    أرجع خالي_اشارو;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  بنانشان Intra_Piece_وٿي = وٿي - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece پڇ(Pieces[i].Str_معلومات, Pieces[i].شروعات_Offs + Intra_Piece_وٿي,
                 Pieces[i].پڇاڙي_Offs);
  سائيز -= Pieces[i].سائيز_2();
  Pieces[i].پڇاڙي_Offs = Pieces[i].شروعات_Offs + Intra_Piece_وٿي;
  سائيز += Pieces[i].سائيز_2();

  أرجع وجهو(وٿي, پڇ);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ڳنڍ *Rope_Piece_BTree_پن::وجهو(بنانشان وٿي,
                                               ثابت RopePiece &R) {
  // If this node is not full, insert the piece.
  إذا (!is_ڀريل()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    بنانشان i = 0, e = get_انگ_Pieces();
    إذا (وٿي == سائيز_2()) {
      // Fastpath for a common case.
      i = e;
    } وإلا {
      بنانشان جڳهه_Offs = 0;
      لكل (; وٿي > جڳهه_Offs; ++i)
        جڳهه_Offs += getPiece(i).سائيز_2();
      أكد(جڳهه_Offs == وٿي && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    لكل (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++انگ_Pieces;
    سائيز += R.سائيز_2();
    أرجع خالي_اشارو;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  Rope_Piece_BTree_پن *New_ڳنڍ = جديد Rope_Piece_BTree_پن();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[ويڪر_ضرب_ڪندڙ], &Pieces[2 * ويڪر_ضرب_ڪندڙ],
            &New_ڳنڍ->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[ويڪر_ضرب_ڪندڙ], &Pieces[2 * ويڪر_ضرب_ڪندڙ], RopePiece());

  // Decrease the number of values in the two nodes.
  New_ڳنڍ->انگ_Pieces = انگ_Pieces = ويڪر_ضرب_ڪندڙ;

  // Recompute the two nodes' size.
  New_ڳنڍ->ڀريل_Recompute_سائيز_Locally();
  ڀريل_Recompute_سائيز_Locally();

  // Update the list of leaves.
  New_ڳنڍ->وجهو_After_پن_اندريون_ترتيب(هذا);

  // These insertions can't fail.
  إذا (هذا->سائيز_2() >= وٿي)
    هذا->وجهو(وٿي, R);
  وإلا
    New_ڳنڍ->وجهو(وٿي - هذا->سائيز_2(), R);
  أرجع New_ڳنڍ;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
فراغ Rope_Piece_BTree_پن::erase(بنانشان وٿي, بنانشان انگ_بائيٽون) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  بنانشان PieceOffs = 0;
  بنانشان i = 0;
  لكل (; وٿي > PieceOffs; ++i)
    PieceOffs += getPiece(i).سائيز_2();
  أكد(PieceOffs == وٿي && "Split didn't occur before erase!");

  بنانشان شروعات_Piece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  لكل (; وٿي + انگ_بائيٽون > PieceOffs + getPiece(i).سائيز_2(); ++i)
    PieceOffs += getPiece(i).سائيز_2();

  // If we exactly include the last one, include it in the region to delete.
  إذا (وٿي + انگ_بائيٽون == PieceOffs + getPiece(i).سائيز_2()) {
    PieceOffs += getPiece(i).سائيز_2();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  إذا (i != شروعات_Piece) {
    بنانشان انگ_Deleted = i - شروعات_Piece;
    لكل (; i != get_انگ_Pieces(); ++i)
      Pieces[i - انگ_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_انگ_Pieces() - انگ_Deleted], &Pieces[get_انگ_Pieces()],
              RopePiece());
    انگ_Pieces -= انگ_Deleted;

    بنانشان Cover_بائيٽون = PieceOffs - وٿي;
    انگ_بائيٽون -= Cover_بائيٽون;
    سائيز -= Cover_بائيٽون;
  }

  // If we completely removed some stuff, we could be done.
  إذا (انگ_بائيٽون == 0)
    أرجع;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  أكد(getPiece(شروعات_Piece).سائيز_2() > انگ_بائيٽون);
  Pieces[شروعات_Piece].شروعات_Offs += انگ_بائيٽون;

  // The size of this node just shrunk by NumBytes.
  سائيز -= انگ_بائيٽون;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

نطاق {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
صنف RopePieceBTreeInterior : عام Rope_Piece_BTree_ڳنڍ {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  بنانشان حرف انگ_Children = 0;

  Rope_Piece_BTree_ڳنڍ *Children[2 * ويڪر_ضرب_ڪندڙ];

عام:
  RopePieceBTreeInterior() : Rope_Piece_BTree_ڳنڍ(خطأ) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_ڳنڍ *LHS, Rope_Piece_BTree_ڳنڍ *RHS)
      : Rope_Piece_BTree_ڳنڍ(خطأ) {
    Children[0] = LHS;
    Children[1] = RHS;
    انگ_Children = 2;
    سائيز = LHS->سائيز_2() + RHS->سائيز_2();
  }

  ~RopePieceBTreeInterior() {
    لكل (بنانشان i = 0, e = get_انگ_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  منطقي is_ڀريل() ثابت { أرجع انگ_Children == 2 * ويڪر_ضرب_ڪندڙ; }

  بنانشان get_انگ_Children() ثابت { أرجع انگ_Children; }

  ثابت Rope_Piece_BTree_ڳنڍ *get_ٻار(بنانشان i) ثابت {
    أكد(i < انگ_Children && "invalid child #");
    أرجع Children[i];
  }

  Rope_Piece_BTree_ڳنڍ *get_ٻار(بنانشان i) {
    أكد(i < انگ_Children && "invalid child #");
    أرجع Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  فراغ ڀريل_Recompute_سائيز_Locally() {
    سائيز = 0;
    لكل (بنانشان i = 0, e = get_انگ_Children(); i != e; ++i)
      سائيز += get_ٻار(i)->سائيز_2();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ڳنڍ *ورهايو(بنانشان وٿي);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ڳنڍ *وجهو(بنانشان وٿي, ثابت RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_ڳنڍ *Handle_ٻار_Piece(بنانشان i, Rope_Piece_BTree_ڳنڍ *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  فراغ erase(بنانشان وٿي, بنانشان انگ_بائيٽون);

  ساكن منطقي classof(ثابت Rope_Piece_BTree_ڳنڍ *N) { أرجع !N->is_پن(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ڳنڍ *RopePieceBTreeInterior::ورهايو(بنانشان وٿي) {
  // Figure out which child to split.
  إذا (وٿي == 0 || وٿي == سائيز_2())
    أرجع خالي_اشارو; // If we have an exact offset, we're already split.

  بنانشان ٻار_وٿي = 0;
  بنانشان i = 0;
  لكل (; وٿي >= ٻار_وٿي + get_ٻار(i)->سائيز_2(); ++i)
    ٻار_وٿي += get_ٻار(i)->سائيز_2();

  // If already split there, we're done.
  إذا (ٻار_وٿي == وٿي)
    أرجع خالي_اشارو;

  // Otherwise, recursively split the child.
  إذا (Rope_Piece_BTree_ڳنڍ *RHS = get_ٻار(i)->ورهايو(وٿي - ٻار_وٿي))
    أرجع Handle_ٻار_Piece(i, RHS);
  أرجع خالي_اشارو; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ڳنڍ *RopePieceBTreeInterior::وجهو(بنانشان وٿي,
                                                   ثابت RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  بنانشان i = 0, e = get_انگ_Children();

  بنانشان ٻار_Offs = 0;
  إذا (وٿي == سائيز_2()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    ٻار_Offs = سائيز_2() - get_ٻار(i)->سائيز_2();
  } وإلا {
    لكل (; وٿي > ٻار_Offs + get_ٻار(i)->سائيز_2(); ++i)
      ٻار_Offs += get_ٻار(i)->سائيز_2();
  }

  سائيز += R.سائيز_2();

  // Insert at the end of this child.
  إذا (Rope_Piece_BTree_ڳنڍ *RHS = get_ٻار(i)->وجهو(وٿي - ٻار_Offs, R))
    أرجع Handle_ٻار_Piece(i, RHS);

  أرجع خالي_اشارو;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_ڳنڍ *
RopePieceBTreeInterior::Handle_ٻار_Piece(بنانشان i, Rope_Piece_BTree_ڳنڍ *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  إذا (!is_ڀريل()) {
    // Insert RHS after child 'i'.
    إذا (i + 1 != get_انگ_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_انگ_Children() - i - 1) * حجم(Children[0]));
    Children[i + 1] = RHS;
    ++انگ_Children;
    أرجع خالي_اشارو;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_ڳنڍ = جديد RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  يادگيري_نقل(&New_ڳنڍ->Children[0], &Children[ويڪر_ضرب_ڪندڙ],
         ويڪر_ضرب_ڪندڙ * حجم(Children[0]));

  // Decrease the number of values in the two nodes.
  New_ڳنڍ->انگ_Children = انگ_Children = ويڪر_ضرب_ڪندڙ;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  إذا (i < ويڪر_ضرب_ڪندڙ)
    هذا->Handle_ٻار_Piece(i, RHS);
  وإلا
    New_ڳنڍ->Handle_ٻار_Piece(i - ويڪر_ضرب_ڪندڙ, RHS);

  // Recompute the two nodes' size.
  New_ڳنڍ->ڀريل_Recompute_سائيز_Locally();
  ڀريل_Recompute_سائيز_Locally();
  أرجع New_ڳنڍ;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
فراغ RopePieceBTreeInterior::erase(بنانشان وٿي, بنانشان انگ_بائيٽون) {
  // This will shrink this node by NumBytes.
  سائيز -= انگ_بائيٽون;

  // Find the first child that overlaps with Offset.
  بنانشان i = 0;
  لكل (; وٿي >= get_ٻار(i)->سائيز_2(); ++i)
    وٿي -= get_ٻار(i)->سائيز_2();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  طالما (انگ_بائيٽون) {
    Rope_Piece_BTree_ڳنڍ *Cur_ٻار = get_ٻار(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    إذا (وٿي + انگ_بائيٽون < Cur_ٻار->سائيز_2()) {
      Cur_ٻار->erase(وٿي, انگ_بائيٽون);
      أرجع;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    إذا (وٿي) {
      بنانشان بائيٽون_From_ٻار = Cur_ٻار->سائيز_2() - وٿي;
      Cur_ٻار->erase(وٿي, بائيٽون_From_ٻار);
      انگ_بائيٽون -= بائيٽون_From_ٻار;
      // Start at the beginning of the next child.
      وٿي = 0;
      ++i;
      استمر;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    انگ_بائيٽون -= Cur_ٻار->سائيز_2();
    Cur_ٻار->Destroy();
    --انگ_Children;
    إذا (i != get_انگ_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_انگ_Children() - i) * حجم(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

فراغ Rope_Piece_BTree_ڳنڍ::Destroy() {
  إذا (خودڪار *پن = dyn_cast<Rope_Piece_BTree_پن>(هذا))
    ميٽيو پن;
  وإلا
    ميٽيو cast<RopePieceBTreeInterior>(هذا);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ڳنڍ *Rope_Piece_BTree_ڳنڍ::ورهايو(بنانشان وٿي) {
  أكد(وٿي <= سائيز_2() && "Invalid offset to split!");
  إذا (خودڪار *پن = dyn_cast<Rope_Piece_BTree_پن>(هذا))
    أرجع پن->ورهايو(وٿي);
  أرجع cast<RopePieceBTreeInterior>(هذا)->ورهايو(وٿي);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ڳنڍ *Rope_Piece_BTree_ڳنڍ::وجهو(بنانشان وٿي,
                                               ثابت RopePiece &R) {
  أكد(وٿي <= سائيز_2() && "Invalid offset to insert!");
  إذا (خودڪار *پن = dyn_cast<Rope_Piece_BTree_پن>(هذا))
    أرجع پن->وجهو(وٿي, R);
  أرجع cast<RopePieceBTreeInterior>(هذا)->وجهو(وٿي, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
فراغ Rope_Piece_BTree_ڳنڍ::erase(بنانشان وٿي, بنانشان انگ_بائيٽون) {
  أكد(وٿي + انگ_بائيٽون <= سائيز_2() && "Invalid offset to erase!");
  إذا (خودڪار *پن = dyn_cast<Rope_Piece_BTree_پن>(هذا))
    أرجع پن->erase(وٿي, انگ_بائيٽون);
  أرجع cast<RopePieceBTreeInterior>(هذا)->erase(وٿي, انگ_بائيٽون);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

ساكن ثابت Rope_Piece_BTree_پن *getCN(ثابت فراغ *P) {
  أرجع static_cast<ثابت Rope_Piece_BTree_پن *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(ثابت فراغ *n) {
  ثابت خودڪار *N = static_cast<ثابت Rope_Piece_BTree_ڳنڍ *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  طالما (ثابت خودڪار *اندريون = dyn_cast<RopePieceBTreeInterior>(N))
    N = اندريون->get_ٻار(0);

  // We must have at least one leaf.
  Cur_ڳنڍ = cast<Rope_Piece_BTree_پن>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  طالما (Cur_ڳنڍ && getCN(Cur_ڳنڍ)->get_انگ_Pieces() == 0)
    Cur_ڳنڍ = getCN(Cur_ڳنڍ)->get_ايندڙ_پن_اندريون_ترتيب();

  إذا (Cur_ڳنڍ)
    CurPiece = &getCN(Cur_ڳنڍ)->getPiece(0);
  وإلا // Empty tree, this is an end() iterator.
    CurPiece = خالي_اشارو;
  CurChar = 0;
}

فراغ RopePieceBTreeIterator::Move_منزل_ايندڙ_Piece() {
  إذا (CurPiece !=
      &getCN(Cur_ڳنڍ)->getPiece(getCN(Cur_ڳنڍ)->get_انگ_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    أرجع;
  }

  // Find the next non-empty leaf node.
  افعل
    Cur_ڳنڍ = getCN(Cur_ڳنڍ)->get_ايندڙ_پن_اندريون_ترتيب();
  طالما (Cur_ڳنڍ && getCN(Cur_ڳنڍ)->get_انگ_Pieces() == 0);

  إذا (Cur_ڳنڍ)
    CurPiece = &getCN(Cur_ڳنڍ)->getPiece(0);
  وإلا // Hit end().
    CurPiece = خالي_اشارو;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

ساكن Rope_Piece_BTree_ڳنڍ *get_پاڙ(فراغ *P) {
  أرجع static_cast<Rope_Piece_BTree_ڳنڍ *>(P);
}

RopePieceBTree::RopePieceBTree() { پاڙ_2 = جديد Rope_Piece_BTree_پن(); }

RopePieceBTree::RopePieceBTree(ثابت RopePieceBTree &RHS) {
  أكد(RHS.خالي_جايون() && "Can't copy non-empty tree yet");
  پاڙ_2 = جديد Rope_Piece_BTree_پن();
}

RopePieceBTree::~RopePieceBTree() { get_پاڙ(پاڙ_2)->Destroy(); }

بنانشان RopePieceBTree::سائيز_2() ثابت { أرجع get_پاڙ(پاڙ_2)->سائيز_2(); }

فراغ RopePieceBTree::clear() {
  إذا (خودڪار *پن = dyn_cast<Rope_Piece_BTree_پن>(get_پاڙ(پاڙ_2)))
    پن->clear();
  وإلا {
    get_پاڙ(پاڙ_2)->Destroy();
    پاڙ_2 = جديد Rope_Piece_BTree_پن();
  }
}

فراغ RopePieceBTree::وجهو(بنانشان وٿي, ثابت RopePiece &R) {
  // #1. Split at Offset.
  إذا (Rope_Piece_BTree_ڳنڍ *RHS = get_پاڙ(پاڙ_2)->ورهايو(وٿي))
    پاڙ_2 = جديد RopePieceBTreeInterior(get_پاڙ(پاڙ_2), RHS);

  // #2. Do the insertion.
  إذا (Rope_Piece_BTree_ڳنڍ *RHS = get_پاڙ(پاڙ_2)->وجهو(وٿي, R))
    پاڙ_2 = جديد RopePieceBTreeInterior(get_پاڙ(پاڙ_2), RHS);
}

فراغ RopePieceBTree::erase(بنانشان وٿي, بنانشان انگ_بائيٽون) {
  // #1. Split at Offset.
  إذا (Rope_Piece_BTree_ڳنڍ *RHS = get_پاڙ(پاڙ_2)->ورهايو(وٿي))
    پاڙ_2 = جديد RopePieceBTreeInterior(get_پاڙ(پاڙ_2), RHS);

  // #2. Do the erasing.
  get_پاڙ(پاڙ_2)->erase(وٿي, انگ_بائيٽون);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(ثابت حرف *شروعات, ثابت حرف *پڇاڙي_2) {
  بنانشان Len = پڇاڙي_2 - شروعات;
  أكد(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  إذا (AllocOffs + Len <= Alloc_Chunk_سائيز) {
    يادگيري_نقل(Alloc_بفر->معلومات_2 + AllocOffs, شروعات, Len);
    AllocOffs += Len;
    أرجع RopePiece(Alloc_بفر, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  إذا (Len > Alloc_Chunk_سائيز) {
    بنانشان سائيز = پڇاڙي_2 - شروعات + حجم(Rope_Ref_ڳڻپ_String) - 1;
    خودڪار *Res = reinterpret_cast<Rope_Ref_ڳڻپ_String *>(جديد حرف[سائيز]);
    Res->Ref_ڳڻپ = 0;
    يادگيري_نقل(Res->معلومات_2, شروعات, پڇاڙي_2 - شروعات);
    أرجع RopePiece(Res, 0, پڇاڙي_2 - شروعات);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  بنانشان Alloc_سائيز = offsetof(Rope_Ref_ڳڻپ_String, معلومات_2) + Alloc_Chunk_سائيز;
  خودڪار *Res = reinterpret_cast<Rope_Ref_ڳڻپ_String *>(جديد حرف[Alloc_سائيز]);
  Res->Ref_ڳڻپ = 0;
  يادگيري_نقل(Res->معلومات_2, شروعات, Len);
  Alloc_بفر = Res;
  AllocOffs = Len;

  أرجع RopePiece(Alloc_بفر, 0, Len);
}
