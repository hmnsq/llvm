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

استفاده نطاق llvm;

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
صنف Rope_Piece_BTree_گره {
محمي:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  تعداد { عرض_ضریب = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  بیعلامت اندازه = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  منطقي Is_برگ;

  Rope_Piece_BTree_گره(منطقي is_برگ) : Is_برگ(is_برگ) {}
  ~Rope_Piece_BTree_گره() = افتراضي;

عام:
  منطقي is_برگ() ثابت { أرجع Is_برگ; }
  بیعلامت اندازه_2() ثابت { أرجع اندازه; }

  فراغ Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گره *تقسیم(بیعلامت جابه_جایی);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گره *درج(بیعلامت جابه_جایی, ثابت RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  فراغ erase(بیعلامت جابه_جایی, بیعلامت رقم_بایتها);
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
صنف Rope_Piece_BTree_برگ : عام Rope_Piece_BTree_گره {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  بیعلامت حرف رقم_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * عرض_ضریب];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  Rope_Piece_BTree_برگ **Prev_برگ = اشاره_تهی;
  Rope_Piece_BTree_برگ *بعدی_برگ = اشاره_تهی;

عام:
  Rope_Piece_BTree_برگ() : Rope_Piece_BTree_گره(درست) {}

  ~Rope_Piece_BTree_برگ() {
    إذا (Prev_برگ || بعدی_برگ)
      remove_From_برگ_درون_ترتیب();
    clear();
  }

  منطقي is_پر() ثابت { أرجع رقم_Pieces == 2 * عرض_ضریب; }

  /// clear - Remove all rope pieces from this leaf.
  فراغ clear() {
    طالما (رقم_Pieces)
      Pieces[--رقم_Pieces] = RopePiece();
    اندازه = 0;
  }

  بیعلامت get_رقم_Pieces() ثابت { أرجع رقم_Pieces; }

  ثابت RopePiece &getPiece(بیعلامت i) ثابت {
    أكد(i < get_رقم_Pieces() && "Invalid piece ID");
    أرجع Pieces[i];
  }

  ثابت Rope_Piece_BTree_برگ *get_بعدی_برگ_درون_ترتیب() ثابت { أرجع بعدی_برگ; }

  فراغ درج_After_برگ_درون_ترتیب(Rope_Piece_BTree_برگ *گره) {
    أكد(!Prev_برگ && !بعدی_برگ && "Already in ordering");

    بعدی_برگ = گره->بعدی_برگ;
    إذا (بعدی_برگ)
      بعدی_برگ->Prev_برگ = &بعدی_برگ;
    Prev_برگ = &گره->بعدی_برگ;
    گره->بعدی_برگ = هذا;
  }

  فراغ remove_From_برگ_درون_ترتیب() {
    إذا (Prev_برگ) {
      *Prev_برگ = بعدی_برگ;
      إذا (بعدی_برگ)
        بعدی_برگ->Prev_برگ = Prev_برگ;
    } وإلا إذا (بعدی_برگ) {
      بعدی_برگ->Prev_برگ = اشاره_تهی;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  فراغ پر_Recompute_اندازه_Locally() {
    اندازه = 0;
    لكل (بیعلامت i = 0, e = get_رقم_Pieces(); i != e; ++i)
      اندازه += getPiece(i).اندازه_2();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گره *تقسیم(بیعلامت جابه_جایی);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گره *درج(بیعلامت جابه_جایی, ثابت RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  فراغ erase(بیعلامت جابه_جایی, بیعلامت رقم_بایتها);

  ساكن منطقي classof(ثابت Rope_Piece_BTree_گره *N) { أرجع N->is_برگ(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گره *Rope_Piece_BTree_برگ::تقسیم(بیعلامت جابه_جایی) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  إذا (جابه_جایی == 0 || جابه_جایی == اندازه_2()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    أرجع اشاره_تهی;
  }

  // Find the piece that this offset lands in.
  بیعلامت PieceOffs = 0;
  بیعلامت i = 0;
  طالما (جابه_جایی >= PieceOffs + Pieces[i].اندازه_2()) {
    PieceOffs += Pieces[i].اندازه_2();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  إذا (PieceOffs == جابه_جایی)
    أرجع اشاره_تهی;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  بیعلامت Intra_Piece_جابه_جایی = جابه_جایی - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece دم(Pieces[i].Str_داده, Pieces[i].شروع_Offs + Intra_Piece_جابه_جایی,
                 Pieces[i].انتها_Offs);
  اندازه -= Pieces[i].اندازه_2();
  Pieces[i].انتها_Offs = Pieces[i].شروع_Offs + Intra_Piece_جابه_جایی;
  اندازه += Pieces[i].اندازه_2();

  أرجع درج(جابه_جایی, دم);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گره *Rope_Piece_BTree_برگ::درج(بیعلامت جابه_جایی,
                                               ثابت RopePiece &R) {
  // If this node is not full, insert the piece.
  إذا (!is_پر()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    بیعلامت i = 0, e = get_رقم_Pieces();
    إذا (جابه_جایی == اندازه_2()) {
      // Fastpath for a common case.
      i = e;
    } وإلا {
      بیعلامت جایگاه_Offs = 0;
      لكل (; جابه_جایی > جایگاه_Offs; ++i)
        جایگاه_Offs += getPiece(i).اندازه_2();
      أكد(جایگاه_Offs == جابه_جایی && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    لكل (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++رقم_Pieces;
    اندازه += R.اندازه_2();
    أرجع اشاره_تهی;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  Rope_Piece_BTree_برگ *New_گره = جديد Rope_Piece_BTree_برگ();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[عرض_ضریب], &Pieces[2 * عرض_ضریب],
            &New_گره->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[عرض_ضریب], &Pieces[2 * عرض_ضریب], RopePiece());

  // Decrease the number of values in the two nodes.
  New_گره->رقم_Pieces = رقم_Pieces = عرض_ضریب;

  // Recompute the two nodes' size.
  New_گره->پر_Recompute_اندازه_Locally();
  پر_Recompute_اندازه_Locally();

  // Update the list of leaves.
  New_گره->درج_After_برگ_درون_ترتیب(هذا);

  // These insertions can't fail.
  إذا (هذا->اندازه_2() >= جابه_جایی)
    هذا->درج(جابه_جایی, R);
  وإلا
    New_گره->درج(جابه_جایی - هذا->اندازه_2(), R);
  أرجع New_گره;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
فراغ Rope_Piece_BTree_برگ::erase(بیعلامت جابه_جایی, بیعلامت رقم_بایتها) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  بیعلامت PieceOffs = 0;
  بیعلامت i = 0;
  لكل (; جابه_جایی > PieceOffs; ++i)
    PieceOffs += getPiece(i).اندازه_2();
  أكد(PieceOffs == جابه_جایی && "Split didn't occur before erase!");

  بیعلامت شروع_Piece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  لكل (; جابه_جایی + رقم_بایتها > PieceOffs + getPiece(i).اندازه_2(); ++i)
    PieceOffs += getPiece(i).اندازه_2();

  // If we exactly include the last one, include it in the region to delete.
  إذا (جابه_جایی + رقم_بایتها == PieceOffs + getPiece(i).اندازه_2()) {
    PieceOffs += getPiece(i).اندازه_2();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  إذا (i != شروع_Piece) {
    بیعلامت رقم_Deleted = i - شروع_Piece;
    لكل (; i != get_رقم_Pieces(); ++i)
      Pieces[i - رقم_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_رقم_Pieces() - رقم_Deleted], &Pieces[get_رقم_Pieces()],
              RopePiece());
    رقم_Pieces -= رقم_Deleted;

    بیعلامت Cover_بایتها = PieceOffs - جابه_جایی;
    رقم_بایتها -= Cover_بایتها;
    اندازه -= Cover_بایتها;
  }

  // If we completely removed some stuff, we could be done.
  إذا (رقم_بایتها == 0)
    أرجع;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  أكد(getPiece(شروع_Piece).اندازه_2() > رقم_بایتها);
  Pieces[شروع_Piece].شروع_Offs += رقم_بایتها;

  // The size of this node just shrunk by NumBytes.
  اندازه -= رقم_بایتها;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

نطاق {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
صنف RopePieceBTreeInterior : عام Rope_Piece_BTree_گره {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  بیعلامت حرف رقم_Children = 0;

  Rope_Piece_BTree_گره *Children[2 * عرض_ضریب];

عام:
  RopePieceBTreeInterior() : Rope_Piece_BTree_گره(خطأ) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_گره *LHS, Rope_Piece_BTree_گره *RHS)
      : Rope_Piece_BTree_گره(خطأ) {
    Children[0] = LHS;
    Children[1] = RHS;
    رقم_Children = 2;
    اندازه = LHS->اندازه_2() + RHS->اندازه_2();
  }

  ~RopePieceBTreeInterior() {
    لكل (بیعلامت i = 0, e = get_رقم_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  منطقي is_پر() ثابت { أرجع رقم_Children == 2 * عرض_ضریب; }

  بیعلامت get_رقم_Children() ثابت { أرجع رقم_Children; }

  ثابت Rope_Piece_BTree_گره *get_فرزند(بیعلامت i) ثابت {
    أكد(i < رقم_Children && "invalid child #");
    أرجع Children[i];
  }

  Rope_Piece_BTree_گره *get_فرزند(بیعلامت i) {
    أكد(i < رقم_Children && "invalid child #");
    أرجع Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  فراغ پر_Recompute_اندازه_Locally() {
    اندازه = 0;
    لكل (بیعلامت i = 0, e = get_رقم_Children(); i != e; ++i)
      اندازه += get_فرزند(i)->اندازه_2();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گره *تقسیم(بیعلامت جابه_جایی);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گره *درج(بیعلامت جابه_جایی, ثابت RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_گره *Handle_فرزند_Piece(بیعلامت i, Rope_Piece_BTree_گره *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  فراغ erase(بیعلامت جابه_جایی, بیعلامت رقم_بایتها);

  ساكن منطقي classof(ثابت Rope_Piece_BTree_گره *N) { أرجع !N->is_برگ(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گره *RopePieceBTreeInterior::تقسیم(بیعلامت جابه_جایی) {
  // Figure out which child to split.
  إذا (جابه_جایی == 0 || جابه_جایی == اندازه_2())
    أرجع اشاره_تهی; // If we have an exact offset, we're already split.

  بیعلامت فرزند_جابه_جایی = 0;
  بیعلامت i = 0;
  لكل (; جابه_جایی >= فرزند_جابه_جایی + get_فرزند(i)->اندازه_2(); ++i)
    فرزند_جابه_جایی += get_فرزند(i)->اندازه_2();

  // If already split there, we're done.
  إذا (فرزند_جابه_جایی == جابه_جایی)
    أرجع اشاره_تهی;

  // Otherwise, recursively split the child.
  إذا (Rope_Piece_BTree_گره *RHS = get_فرزند(i)->تقسیم(جابه_جایی - فرزند_جابه_جایی))
    أرجع Handle_فرزند_Piece(i, RHS);
  أرجع اشاره_تهی; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گره *RopePieceBTreeInterior::درج(بیعلامت جابه_جایی,
                                                   ثابت RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  بیعلامت i = 0, e = get_رقم_Children();

  بیعلامت فرزند_Offs = 0;
  إذا (جابه_جایی == اندازه_2()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    فرزند_Offs = اندازه_2() - get_فرزند(i)->اندازه_2();
  } وإلا {
    لكل (; جابه_جایی > فرزند_Offs + get_فرزند(i)->اندازه_2(); ++i)
      فرزند_Offs += get_فرزند(i)->اندازه_2();
  }

  اندازه += R.اندازه_2();

  // Insert at the end of this child.
  إذا (Rope_Piece_BTree_گره *RHS = get_فرزند(i)->درج(جابه_جایی - فرزند_Offs, R))
    أرجع Handle_فرزند_Piece(i, RHS);

  أرجع اشاره_تهی;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_گره *
RopePieceBTreeInterior::Handle_فرزند_Piece(بیعلامت i, Rope_Piece_BTree_گره *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  إذا (!is_پر()) {
    // Insert RHS after child 'i'.
    إذا (i + 1 != get_رقم_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_رقم_Children() - i - 1) * حجم(Children[0]));
    Children[i + 1] = RHS;
    ++رقم_Children;
    أرجع اشاره_تهی;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_گره = جديد RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  کپی_حافظه(&New_گره->Children[0], &Children[عرض_ضریب],
         عرض_ضریب * حجم(Children[0]));

  // Decrease the number of values in the two nodes.
  New_گره->رقم_Children = رقم_Children = عرض_ضریب;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  إذا (i < عرض_ضریب)
    هذا->Handle_فرزند_Piece(i, RHS);
  وإلا
    New_گره->Handle_فرزند_Piece(i - عرض_ضریب, RHS);

  // Recompute the two nodes' size.
  New_گره->پر_Recompute_اندازه_Locally();
  پر_Recompute_اندازه_Locally();
  أرجع New_گره;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
فراغ RopePieceBTreeInterior::erase(بیعلامت جابه_جایی, بیعلامت رقم_بایتها) {
  // This will shrink this node by NumBytes.
  اندازه -= رقم_بایتها;

  // Find the first child that overlaps with Offset.
  بیعلامت i = 0;
  لكل (; جابه_جایی >= get_فرزند(i)->اندازه_2(); ++i)
    جابه_جایی -= get_فرزند(i)->اندازه_2();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  طالما (رقم_بایتها) {
    Rope_Piece_BTree_گره *Cur_فرزند = get_فرزند(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    إذا (جابه_جایی + رقم_بایتها < Cur_فرزند->اندازه_2()) {
      Cur_فرزند->erase(جابه_جایی, رقم_بایتها);
      أرجع;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    إذا (جابه_جایی) {
      بیعلامت بایتها_From_فرزند = Cur_فرزند->اندازه_2() - جابه_جایی;
      Cur_فرزند->erase(جابه_جایی, بایتها_From_فرزند);
      رقم_بایتها -= بایتها_From_فرزند;
      // Start at the beginning of the next child.
      جابه_جایی = 0;
      ++i;
      استمر;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    رقم_بایتها -= Cur_فرزند->اندازه_2();
    Cur_فرزند->Destroy();
    --رقم_Children;
    إذا (i != get_رقم_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_رقم_Children() - i) * حجم(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

فراغ Rope_Piece_BTree_گره::Destroy() {
  إذا (خودکار *برگ = dyn_cast<Rope_Piece_BTree_برگ>(هذا))
    حذف برگ;
  وإلا
    حذف cast<RopePieceBTreeInterior>(هذا);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گره *Rope_Piece_BTree_گره::تقسیم(بیعلامت جابه_جایی) {
  أكد(جابه_جایی <= اندازه_2() && "Invalid offset to split!");
  إذا (خودکار *برگ = dyn_cast<Rope_Piece_BTree_برگ>(هذا))
    أرجع برگ->تقسیم(جابه_جایی);
  أرجع cast<RopePieceBTreeInterior>(هذا)->تقسیم(جابه_جایی);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گره *Rope_Piece_BTree_گره::درج(بیعلامت جابه_جایی,
                                               ثابت RopePiece &R) {
  أكد(جابه_جایی <= اندازه_2() && "Invalid offset to insert!");
  إذا (خودکار *برگ = dyn_cast<Rope_Piece_BTree_برگ>(هذا))
    أرجع برگ->درج(جابه_جایی, R);
  أرجع cast<RopePieceBTreeInterior>(هذا)->درج(جابه_جایی, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
فراغ Rope_Piece_BTree_گره::erase(بیعلامت جابه_جایی, بیعلامت رقم_بایتها) {
  أكد(جابه_جایی + رقم_بایتها <= اندازه_2() && "Invalid offset to erase!");
  إذا (خودکار *برگ = dyn_cast<Rope_Piece_BTree_برگ>(هذا))
    أرجع برگ->erase(جابه_جایی, رقم_بایتها);
  أرجع cast<RopePieceBTreeInterior>(هذا)->erase(جابه_جایی, رقم_بایتها);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

ساكن ثابت Rope_Piece_BTree_برگ *getCN(ثابت فراغ *P) {
  أرجع static_cast<ثابت Rope_Piece_BTree_برگ *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(ثابت فراغ *n) {
  ثابت خودکار *N = static_cast<ثابت Rope_Piece_BTree_گره *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  طالما (ثابت خودکار *درون = dyn_cast<RopePieceBTreeInterior>(N))
    N = درون->get_فرزند(0);

  // We must have at least one leaf.
  Cur_گره = cast<Rope_Piece_BTree_برگ>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  طالما (Cur_گره && getCN(Cur_گره)->get_رقم_Pieces() == 0)
    Cur_گره = getCN(Cur_گره)->get_بعدی_برگ_درون_ترتیب();

  إذا (Cur_گره)
    CurPiece = &getCN(Cur_گره)->getPiece(0);
  وإلا // Empty tree, this is an end() iterator.
    CurPiece = اشاره_تهی;
  CurChar = 0;
}

فراغ RopePieceBTreeIterator::Move_مقصد_بعدی_Piece() {
  إذا (CurPiece !=
      &getCN(Cur_گره)->getPiece(getCN(Cur_گره)->get_رقم_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    أرجع;
  }

  // Find the next non-empty leaf node.
  افعل
    Cur_گره = getCN(Cur_گره)->get_بعدی_برگ_درون_ترتیب();
  طالما (Cur_گره && getCN(Cur_گره)->get_رقم_Pieces() == 0);

  إذا (Cur_گره)
    CurPiece = &getCN(Cur_گره)->getPiece(0);
  وإلا // Hit end().
    CurPiece = اشاره_تهی;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

ساكن Rope_Piece_BTree_گره *get_ریشه(فراغ *P) {
  أرجع static_cast<Rope_Piece_BTree_گره *>(P);
}

RopePieceBTree::RopePieceBTree() { ریشه_2 = جديد Rope_Piece_BTree_برگ(); }

RopePieceBTree::RopePieceBTree(ثابت RopePieceBTree &RHS) {
  أكد(RHS.خالی() && "Can't copy non-empty tree yet");
  ریشه_2 = جديد Rope_Piece_BTree_برگ();
}

RopePieceBTree::~RopePieceBTree() { get_ریشه(ریشه_2)->Destroy(); }

بیعلامت RopePieceBTree::اندازه_2() ثابت { أرجع get_ریشه(ریشه_2)->اندازه_2(); }

فراغ RopePieceBTree::clear() {
  إذا (خودکار *برگ = dyn_cast<Rope_Piece_BTree_برگ>(get_ریشه(ریشه_2)))
    برگ->clear();
  وإلا {
    get_ریشه(ریشه_2)->Destroy();
    ریشه_2 = جديد Rope_Piece_BTree_برگ();
  }
}

فراغ RopePieceBTree::درج(بیعلامت جابه_جایی, ثابت RopePiece &R) {
  // #1. Split at Offset.
  إذا (Rope_Piece_BTree_گره *RHS = get_ریشه(ریشه_2)->تقسیم(جابه_جایی))
    ریشه_2 = جديد RopePieceBTreeInterior(get_ریشه(ریشه_2), RHS);

  // #2. Do the insertion.
  إذا (Rope_Piece_BTree_گره *RHS = get_ریشه(ریشه_2)->درج(جابه_جایی, R))
    ریشه_2 = جديد RopePieceBTreeInterior(get_ریشه(ریشه_2), RHS);
}

فراغ RopePieceBTree::erase(بیعلامت جابه_جایی, بیعلامت رقم_بایتها) {
  // #1. Split at Offset.
  إذا (Rope_Piece_BTree_گره *RHS = get_ریشه(ریشه_2)->تقسیم(جابه_جایی))
    ریشه_2 = جديد RopePieceBTreeInterior(get_ریشه(ریشه_2), RHS);

  // #2. Do the erasing.
  get_ریشه(ریشه_2)->erase(جابه_جایی, رقم_بایتها);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(ثابت حرف *شروع, ثابت حرف *انتها_2) {
  بیعلامت Len = انتها_2 - شروع;
  أكد(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  إذا (AllocOffs + Len <= Alloc_Chunk_اندازه) {
    کپی_حافظه(Alloc_میانگیر->داده_2 + AllocOffs, شروع, Len);
    AllocOffs += Len;
    أرجع RopePiece(Alloc_میانگیر, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  إذا (Len > Alloc_Chunk_اندازه) {
    بیعلامت اندازه = انتها_2 - شروع + حجم(Rope_Ref_شمار_String) - 1;
    خودکار *Res = reinterpret_cast<Rope_Ref_شمار_String *>(جديد حرف[اندازه]);
    Res->Ref_شمار = 0;
    کپی_حافظه(Res->داده_2, شروع, انتها_2 - شروع);
    أرجع RopePiece(Res, 0, انتها_2 - شروع);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  بیعلامت Alloc_اندازه = offsetof(Rope_Ref_شمار_String, داده_2) + Alloc_Chunk_اندازه;
  خودکار *Res = reinterpret_cast<Rope_Ref_شمار_String *>(جديد حرف[Alloc_اندازه]);
  Res->Ref_شمار = 0;
  کپی_حافظه(Res->داده_2, شروع, Len);
  Alloc_میانگیر = Res;
  AllocOffs = Len;

  أرجع RopePiece(Alloc_میانگیر, 0, Len);
}
