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

کارول نطاق llvm;

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
صنف Rope_Piece_BTree_غوټه {
محمي:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  تعداد { پلنوالی_ضریب = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  بینښه اندازه = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  منطقي Is_ونې_پاڼه;

  Rope_Piece_BTree_غوټه(منطقي is_ونې_پاڼه) : Is_ونې_پاڼه(is_ونې_پاڼه) {}
  ~Rope_Piece_BTree_غوټه() = افتراضي;

عام:
  منطقي is_ونې_پاڼه() ثابت { أرجع Is_ونې_پاڼه; }
  بینښه اندازه_2() ثابت { أرجع اندازه; }

  فراغ Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_غوټه *وېشل(بینښه دننه_واټن);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_غوټه *ورننباسه(بینښه دننه_واټن, ثابت RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  فراغ erase(بینښه دننه_واټن, بینښه عدد_بایټونه);
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
صنف Rope_Piece_BTree_ونې_پاڼه : عام Rope_Piece_BTree_غوټه {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  بینښه حرف عدد_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * پلنوالی_ضریب];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  Rope_Piece_BTree_ونې_پاڼه **Prev_ونې_پاڼه = تش_اشاره;
  Rope_Piece_BTree_ونې_پاڼه *بل_ونې_پاڼه = تش_اشاره;

عام:
  Rope_Piece_BTree_ونې_پاڼه() : Rope_Piece_BTree_غوټه(ریښتیا) {}

  ~Rope_Piece_BTree_ونې_پاڼه() {
    إذا (Prev_ونې_پاڼه || بل_ونې_پاڼه)
      remove_From_ونې_پاڼه_دننه_ترتیب();
    clear();
  }

  منطقي is_ډک() ثابت { أرجع عدد_Pieces == 2 * پلنوالی_ضریب; }

  /// clear - Remove all rope pieces from this leaf.
  فراغ clear() {
    طالما (عدد_Pieces)
      Pieces[--عدد_Pieces] = RopePiece();
    اندازه = 0;
  }

  بینښه get_عدد_Pieces() ثابت { أرجع عدد_Pieces; }

  ثابت RopePiece &getPiece(بینښه i) ثابت {
    أكد(i < get_عدد_Pieces() && "Invalid piece ID");
    أرجع Pieces[i];
  }

  ثابت Rope_Piece_BTree_ونې_پاڼه *get_بل_ونې_پاڼه_دننه_ترتیب() ثابت { أرجع بل_ونې_پاڼه; }

  فراغ ورننباسه_After_ونې_پاڼه_دننه_ترتیب(Rope_Piece_BTree_ونې_پاڼه *غوټه) {
    أكد(!Prev_ونې_پاڼه && !بل_ونې_پاڼه && "Already in ordering");

    بل_ونې_پاڼه = غوټه->بل_ونې_پاڼه;
    إذا (بل_ونې_پاڼه)
      بل_ونې_پاڼه->Prev_ونې_پاڼه = &بل_ونې_پاڼه;
    Prev_ونې_پاڼه = &غوټه->بل_ونې_پاڼه;
    غوټه->بل_ونې_پاڼه = هذا;
  }

  فراغ remove_From_ونې_پاڼه_دننه_ترتیب() {
    إذا (Prev_ونې_پاڼه) {
      *Prev_ونې_پاڼه = بل_ونې_پاڼه;
      إذا (بل_ونې_پاڼه)
        بل_ونې_پاڼه->Prev_ونې_پاڼه = Prev_ونې_پاڼه;
    } وإلا إذا (بل_ونې_پاڼه) {
      بل_ونې_پاڼه->Prev_ونې_پاڼه = تش_اشاره;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  فراغ ډک_Recompute_اندازه_Locally() {
    اندازه = 0;
    لكل (بینښه i = 0, e = get_عدد_Pieces(); i != e; ++i)
      اندازه += getPiece(i).اندازه_2();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_غوټه *وېشل(بینښه دننه_واټن);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_غوټه *ورننباسه(بینښه دننه_واټن, ثابت RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  فراغ erase(بینښه دننه_واټن, بینښه عدد_بایټونه);

  ساكن منطقي classof(ثابت Rope_Piece_BTree_غوټه *N) { أرجع N->is_ونې_پاڼه(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_غوټه *Rope_Piece_BTree_ونې_پاڼه::وېشل(بینښه دننه_واټن) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  إذا (دننه_واټن == 0 || دننه_واټن == اندازه_2()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    أرجع تش_اشاره;
  }

  // Find the piece that this offset lands in.
  بینښه PieceOffs = 0;
  بینښه i = 0;
  طالما (دننه_واټن >= PieceOffs + Pieces[i].اندازه_2()) {
    PieceOffs += Pieces[i].اندازه_2();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  إذا (PieceOffs == دننه_واټن)
    أرجع تش_اشاره;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  بینښه Intra_Piece_دننه_واټن = دننه_واټن - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece لکۍ(Pieces[i].Str_معلومات, Pieces[i].پیل_Offs + Intra_Piece_دننه_واټن,
                 Pieces[i].پای_ټکی_Offs);
  اندازه -= Pieces[i].اندازه_2();
  Pieces[i].پای_ټکی_Offs = Pieces[i].پیل_Offs + Intra_Piece_دننه_واټن;
  اندازه += Pieces[i].اندازه_2();

  أرجع ورننباسه(دننه_واټن, لکۍ);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_غوټه *Rope_Piece_BTree_ونې_پاڼه::ورننباسه(بینښه دننه_واټن,
                                               ثابت RopePiece &R) {
  // If this node is not full, insert the piece.
  إذا (!is_ډک()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    بینښه i = 0, e = get_عدد_Pieces();
    إذا (دننه_واټن == اندازه_2()) {
      // Fastpath for a common case.
      i = e;
    } وإلا {
      بینښه خونه_Offs = 0;
      لكل (; دننه_واټن > خونه_Offs; ++i)
        خونه_Offs += getPiece(i).اندازه_2();
      أكد(خونه_Offs == دننه_واټن && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    لكل (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++عدد_Pieces;
    اندازه += R.اندازه_2();
    أرجع تش_اشاره;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  Rope_Piece_BTree_ونې_پاڼه *New_غوټه = جديد Rope_Piece_BTree_ونې_پاڼه();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[پلنوالی_ضریب], &Pieces[2 * پلنوالی_ضریب],
            &New_غوټه->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[پلنوالی_ضریب], &Pieces[2 * پلنوالی_ضریب], RopePiece());

  // Decrease the number of values in the two nodes.
  New_غوټه->عدد_Pieces = عدد_Pieces = پلنوالی_ضریب;

  // Recompute the two nodes' size.
  New_غوټه->ډک_Recompute_اندازه_Locally();
  ډک_Recompute_اندازه_Locally();

  // Update the list of leaves.
  New_غوټه->ورننباسه_After_ونې_پاڼه_دننه_ترتیب(هذا);

  // These insertions can't fail.
  إذا (هذا->اندازه_2() >= دننه_واټن)
    هذا->ورننباسه(دننه_واټن, R);
  وإلا
    New_غوټه->ورننباسه(دننه_واټن - هذا->اندازه_2(), R);
  أرجع New_غوټه;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
فراغ Rope_Piece_BTree_ونې_پاڼه::erase(بینښه دننه_واټن, بینښه عدد_بایټونه) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  بینښه PieceOffs = 0;
  بینښه i = 0;
  لكل (; دننه_واټن > PieceOffs; ++i)
    PieceOffs += getPiece(i).اندازه_2();
  أكد(PieceOffs == دننه_واټن && "Split didn't occur before erase!");

  بینښه پیل_Piece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  لكل (; دننه_واټن + عدد_بایټونه > PieceOffs + getPiece(i).اندازه_2(); ++i)
    PieceOffs += getPiece(i).اندازه_2();

  // If we exactly include the last one, include it in the region to delete.
  إذا (دننه_واټن + عدد_بایټونه == PieceOffs + getPiece(i).اندازه_2()) {
    PieceOffs += getPiece(i).اندازه_2();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  إذا (i != پیل_Piece) {
    بینښه عدد_Deleted = i - پیل_Piece;
    لكل (; i != get_عدد_Pieces(); ++i)
      Pieces[i - عدد_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_عدد_Pieces() - عدد_Deleted], &Pieces[get_عدد_Pieces()],
              RopePiece());
    عدد_Pieces -= عدد_Deleted;

    بینښه Cover_بایټونه = PieceOffs - دننه_واټن;
    عدد_بایټونه -= Cover_بایټونه;
    اندازه -= Cover_بایټونه;
  }

  // If we completely removed some stuff, we could be done.
  إذا (عدد_بایټونه == 0)
    أرجع;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  أكد(getPiece(پیل_Piece).اندازه_2() > عدد_بایټونه);
  Pieces[پیل_Piece].پیل_Offs += عدد_بایټونه;

  // The size of this node just shrunk by NumBytes.
  اندازه -= عدد_بایټونه;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

نطاق {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
صنف RopePieceBTreeInterior : عام Rope_Piece_BTree_غوټه {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  بینښه حرف عدد_Children = 0;

  Rope_Piece_BTree_غوټه *Children[2 * پلنوالی_ضریب];

عام:
  RopePieceBTreeInterior() : Rope_Piece_BTree_غوټه(خطأ) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_غوټه *LHS, Rope_Piece_BTree_غوټه *RHS)
      : Rope_Piece_BTree_غوټه(خطأ) {
    Children[0] = LHS;
    Children[1] = RHS;
    عدد_Children = 2;
    اندازه = LHS->اندازه_2() + RHS->اندازه_2();
  }

  ~RopePieceBTreeInterior() {
    لكل (بینښه i = 0, e = get_عدد_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  منطقي is_ډک() ثابت { أرجع عدد_Children == 2 * پلنوالی_ضریب; }

  بینښه get_عدد_Children() ثابت { أرجع عدد_Children; }

  ثابت Rope_Piece_BTree_غوټه *get_ماشوم(بینښه i) ثابت {
    أكد(i < عدد_Children && "invalid child #");
    أرجع Children[i];
  }

  Rope_Piece_BTree_غوټه *get_ماشوم(بینښه i) {
    أكد(i < عدد_Children && "invalid child #");
    أرجع Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  فراغ ډک_Recompute_اندازه_Locally() {
    اندازه = 0;
    لكل (بینښه i = 0, e = get_عدد_Children(); i != e; ++i)
      اندازه += get_ماشوم(i)->اندازه_2();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_غوټه *وېشل(بینښه دننه_واټن);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_غوټه *ورننباسه(بینښه دننه_واټن, ثابت RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_غوټه *Handle_ماشوم_Piece(بینښه i, Rope_Piece_BTree_غوټه *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  فراغ erase(بینښه دننه_واټن, بینښه عدد_بایټونه);

  ساكن منطقي classof(ثابت Rope_Piece_BTree_غوټه *N) { أرجع !N->is_ونې_پاڼه(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_غوټه *RopePieceBTreeInterior::وېشل(بینښه دننه_واټن) {
  // Figure out which child to split.
  إذا (دننه_واټن == 0 || دننه_واټن == اندازه_2())
    أرجع تش_اشاره; // If we have an exact offset, we're already split.

  بینښه ماشوم_دننه_واټن = 0;
  بینښه i = 0;
  لكل (; دننه_واټن >= ماشوم_دننه_واټن + get_ماشوم(i)->اندازه_2(); ++i)
    ماشوم_دننه_واټن += get_ماشوم(i)->اندازه_2();

  // If already split there, we're done.
  إذا (ماشوم_دننه_واټن == دننه_واټن)
    أرجع تش_اشاره;

  // Otherwise, recursively split the child.
  إذا (Rope_Piece_BTree_غوټه *RHS = get_ماشوم(i)->وېشل(دننه_واټن - ماشوم_دننه_واټن))
    أرجع Handle_ماشوم_Piece(i, RHS);
  أرجع تش_اشاره; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_غوټه *RopePieceBTreeInterior::ورننباسه(بینښه دننه_واټن,
                                                   ثابت RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  بینښه i = 0, e = get_عدد_Children();

  بینښه ماشوم_Offs = 0;
  إذا (دننه_واټن == اندازه_2()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    ماشوم_Offs = اندازه_2() - get_ماشوم(i)->اندازه_2();
  } وإلا {
    لكل (; دننه_واټن > ماشوم_Offs + get_ماشوم(i)->اندازه_2(); ++i)
      ماشوم_Offs += get_ماشوم(i)->اندازه_2();
  }

  اندازه += R.اندازه_2();

  // Insert at the end of this child.
  إذا (Rope_Piece_BTree_غوټه *RHS = get_ماشوم(i)->ورننباسه(دننه_واټن - ماشوم_Offs, R))
    أرجع Handle_ماشوم_Piece(i, RHS);

  أرجع تش_اشاره;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_غوټه *
RopePieceBTreeInterior::Handle_ماشوم_Piece(بینښه i, Rope_Piece_BTree_غوټه *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  إذا (!is_ډک()) {
    // Insert RHS after child 'i'.
    إذا (i + 1 != get_عدد_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_عدد_Children() - i - 1) * حجم(Children[0]));
    Children[i + 1] = RHS;
    ++عدد_Children;
    أرجع تش_اشاره;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_غوټه = جديد RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  حافظه_نقل(&New_غوټه->Children[0], &Children[پلنوالی_ضریب],
         پلنوالی_ضریب * حجم(Children[0]));

  // Decrease the number of values in the two nodes.
  New_غوټه->عدد_Children = عدد_Children = پلنوالی_ضریب;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  إذا (i < پلنوالی_ضریب)
    هذا->Handle_ماشوم_Piece(i, RHS);
  وإلا
    New_غوټه->Handle_ماشوم_Piece(i - پلنوالی_ضریب, RHS);

  // Recompute the two nodes' size.
  New_غوټه->ډک_Recompute_اندازه_Locally();
  ډک_Recompute_اندازه_Locally();
  أرجع New_غوټه;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
فراغ RopePieceBTreeInterior::erase(بینښه دننه_واټن, بینښه عدد_بایټونه) {
  // This will shrink this node by NumBytes.
  اندازه -= عدد_بایټونه;

  // Find the first child that overlaps with Offset.
  بینښه i = 0;
  لكل (; دننه_واټن >= get_ماشوم(i)->اندازه_2(); ++i)
    دننه_واټن -= get_ماشوم(i)->اندازه_2();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  طالما (عدد_بایټونه) {
    Rope_Piece_BTree_غوټه *Cur_ماشوم = get_ماشوم(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    إذا (دننه_واټن + عدد_بایټونه < Cur_ماشوم->اندازه_2()) {
      Cur_ماشوم->erase(دننه_واټن, عدد_بایټونه);
      أرجع;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    إذا (دننه_واټن) {
      بینښه بایټونه_From_ماشوم = Cur_ماشوم->اندازه_2() - دننه_واټن;
      Cur_ماشوم->erase(دننه_واټن, بایټونه_From_ماشوم);
      عدد_بایټونه -= بایټونه_From_ماشوم;
      // Start at the beginning of the next child.
      دننه_واټن = 0;
      ++i;
      استمر;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    عدد_بایټونه -= Cur_ماشوم->اندازه_2();
    Cur_ماشوم->Destroy();
    --عدد_Children;
    إذا (i != get_عدد_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_عدد_Children() - i) * حجم(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

فراغ Rope_Piece_BTree_غوټه::Destroy() {
  إذا (خپلکاری *ونې_پاڼه = dyn_cast<Rope_Piece_BTree_ونې_پاڼه>(هذا))
    ړنګول ونې_پاڼه;
  وإلا
    ړنګول cast<RopePieceBTreeInterior>(هذا);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_غوټه *Rope_Piece_BTree_غوټه::وېشل(بینښه دننه_واټن) {
  أكد(دننه_واټن <= اندازه_2() && "Invalid offset to split!");
  إذا (خپلکاری *ونې_پاڼه = dyn_cast<Rope_Piece_BTree_ونې_پاڼه>(هذا))
    أرجع ونې_پاڼه->وېشل(دننه_واټن);
  أرجع cast<RopePieceBTreeInterior>(هذا)->وېشل(دننه_واټن);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_غوټه *Rope_Piece_BTree_غوټه::ورننباسه(بینښه دننه_واټن,
                                               ثابت RopePiece &R) {
  أكد(دننه_واټن <= اندازه_2() && "Invalid offset to insert!");
  إذا (خپلکاری *ونې_پاڼه = dyn_cast<Rope_Piece_BTree_ونې_پاڼه>(هذا))
    أرجع ونې_پاڼه->ورننباسه(دننه_واټن, R);
  أرجع cast<RopePieceBTreeInterior>(هذا)->ورننباسه(دننه_واټن, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
فراغ Rope_Piece_BTree_غوټه::erase(بینښه دننه_واټن, بینښه عدد_بایټونه) {
  أكد(دننه_واټن + عدد_بایټونه <= اندازه_2() && "Invalid offset to erase!");
  إذا (خپلکاری *ونې_پاڼه = dyn_cast<Rope_Piece_BTree_ونې_پاڼه>(هذا))
    أرجع ونې_پاڼه->erase(دننه_واټن, عدد_بایټونه);
  أرجع cast<RopePieceBTreeInterior>(هذا)->erase(دننه_واټن, عدد_بایټونه);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

ساكن ثابت Rope_Piece_BTree_ونې_پاڼه *getCN(ثابت فراغ *P) {
  أرجع static_cast<ثابت Rope_Piece_BTree_ونې_پاڼه *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(ثابت فراغ *n) {
  ثابت خپلکاری *N = static_cast<ثابت Rope_Piece_BTree_غوټه *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  طالما (ثابت خپلکاری *دننه = dyn_cast<RopePieceBTreeInterior>(N))
    N = دننه->get_ماشوم(0);

  // We must have at least one leaf.
  Cur_غوټه = cast<Rope_Piece_BTree_ونې_پاڼه>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  طالما (Cur_غوټه && getCN(Cur_غوټه)->get_عدد_Pieces() == 0)
    Cur_غوټه = getCN(Cur_غوټه)->get_بل_ونې_پاڼه_دننه_ترتیب();

  إذا (Cur_غوټه)
    CurPiece = &getCN(Cur_غوټه)->getPiece(0);
  وإلا // Empty tree, this is an end() iterator.
    CurPiece = تش_اشاره;
  CurChar = 0;
}

فراغ RopePieceBTreeIterator::Move_منزل_بل_Piece() {
  إذا (CurPiece !=
      &getCN(Cur_غوټه)->getPiece(getCN(Cur_غوټه)->get_عدد_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    أرجع;
  }

  // Find the next non-empty leaf node.
  افعل
    Cur_غوټه = getCN(Cur_غوټه)->get_بل_ونې_پاڼه_دننه_ترتیب();
  طالما (Cur_غوټه && getCN(Cur_غوټه)->get_عدد_Pieces() == 0);

  إذا (Cur_غوټه)
    CurPiece = &getCN(Cur_غوټه)->getPiece(0);
  وإلا // Hit end().
    CurPiece = تش_اشاره;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

ساكن Rope_Piece_BTree_غوټه *get_ریښه(فراغ *P) {
  أرجع static_cast<Rope_Piece_BTree_غوټه *>(P);
}

RopePieceBTree::RopePieceBTree() { ریښه_2 = جديد Rope_Piece_BTree_ونې_پاڼه(); }

RopePieceBTree::RopePieceBTree(ثابت RopePieceBTree &RHS) {
  أكد(RHS.تشه() && "Can't copy non-empty tree yet");
  ریښه_2 = جديد Rope_Piece_BTree_ونې_پاڼه();
}

RopePieceBTree::~RopePieceBTree() { get_ریښه(ریښه_2)->Destroy(); }

بینښه RopePieceBTree::اندازه_2() ثابت { أرجع get_ریښه(ریښه_2)->اندازه_2(); }

فراغ RopePieceBTree::clear() {
  إذا (خپلکاری *ونې_پاڼه = dyn_cast<Rope_Piece_BTree_ونې_پاڼه>(get_ریښه(ریښه_2)))
    ونې_پاڼه->clear();
  وإلا {
    get_ریښه(ریښه_2)->Destroy();
    ریښه_2 = جديد Rope_Piece_BTree_ونې_پاڼه();
  }
}

فراغ RopePieceBTree::ورننباسه(بینښه دننه_واټن, ثابت RopePiece &R) {
  // #1. Split at Offset.
  إذا (Rope_Piece_BTree_غوټه *RHS = get_ریښه(ریښه_2)->وېشل(دننه_واټن))
    ریښه_2 = جديد RopePieceBTreeInterior(get_ریښه(ریښه_2), RHS);

  // #2. Do the insertion.
  إذا (Rope_Piece_BTree_غوټه *RHS = get_ریښه(ریښه_2)->ورننباسه(دننه_واټن, R))
    ریښه_2 = جديد RopePieceBTreeInterior(get_ریښه(ریښه_2), RHS);
}

فراغ RopePieceBTree::erase(بینښه دننه_واټن, بینښه عدد_بایټونه) {
  // #1. Split at Offset.
  إذا (Rope_Piece_BTree_غوټه *RHS = get_ریښه(ریښه_2)->وېشل(دننه_واټن))
    ریښه_2 = جديد RopePieceBTreeInterior(get_ریښه(ریښه_2), RHS);

  // #2. Do the erasing.
  get_ریښه(ریښه_2)->erase(دننه_واټن, عدد_بایټونه);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(ثابت حرف *پیل, ثابت حرف *پای_ټکی_2) {
  بینښه Len = پای_ټکی_2 - پیل;
  أكد(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  إذا (AllocOffs + Len <= Alloc_Chunk_اندازه) {
    حافظه_نقل(Alloc_بفر->معلومات_2 + AllocOffs, پیل, Len);
    AllocOffs += Len;
    أرجع RopePiece(Alloc_بفر, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  إذا (Len > Alloc_Chunk_اندازه) {
    بینښه اندازه = پای_ټکی_2 - پیل + حجم(Rope_Ref_شمېر_String) - 1;
    خپلکاری *Res = reinterpret_cast<Rope_Ref_شمېر_String *>(جديد حرف[اندازه]);
    Res->Ref_شمېر = 0;
    حافظه_نقل(Res->معلومات_2, پیل, پای_ټکی_2 - پیل);
    أرجع RopePiece(Res, 0, پای_ټکی_2 - پیل);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  بینښه Alloc_اندازه = offsetof(Rope_Ref_شمېر_String, معلومات_2) + Alloc_Chunk_اندازه;
  خپلکاری *Res = reinterpret_cast<Rope_Ref_شمېر_String *>(جديد حرف[Alloc_اندازه]);
  Res->Ref_شمېر = 0;
  حافظه_نقل(Res->معلومات_2, پیل, Len);
  Alloc_بفر = Res;
  AllocOffs = Len;

  أرجع RopePiece(Alloc_بفر, 0, Len);
}
