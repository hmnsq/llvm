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

#شامل "llvm/ADT/RewriteRope.h"
#شامل "llvm/Support/Casting.h"
#شامل <algorithm>
#شامل <cassert>
#شامل <cstring>

استعمال نامخلا llvm;

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

نامخلا {

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Class
//===----------------------------------------------------------------------===//

/// RopePieceBTreeNode - Common base class of RopePieceBTreeLeaf and
/// RopePieceBTreeInterior.  This provides some 'virtual' dispatching methods
/// and a flag that determines which subclass the instance is.  Also
/// important, this node knows the full extend of the node, including any
/// children that it has.  This allows efficient skipping over entire subtrees
/// when looking for an offset in the BTree.
جماعت Rope_Piece_BTree_گرہ {
محفوظ:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  شمار { چوڑائی_ضربی_عدد = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  غیرعلامتی سائز = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  منطقی Is_برگ;

  Rope_Piece_BTree_گرہ(منطقی is_برگ) : Is_برگ(is_برگ) {}
  ~Rope_Piece_BTree_گرہ() = طےشدہ;

عوامی:
  منطقی is_برگ() ثابت { واپس Is_برگ; }
  غیرعلامتی سائز_2() ثابت { واپس سائز; }

  خالی Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گرہ *بانٹو(غیرعلامتی انحراف);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گرہ *داخل_کرو(غیرعلامتی انحراف, ثابت RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  خالی erase(غیرعلامتی انحراف, غیرعلامتی عدد_بائٹیں);
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
جماعت Rope_Piece_BTree_برگ : عوامی Rope_Piece_BTree_گرہ {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  غیرعلامتی حرف عدد_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * چوڑائی_ضربی_عدد];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  Rope_Piece_BTree_برگ **پچھلا_برگ = خالی_اشاریہ;
  Rope_Piece_BTree_برگ *اگلا_برگ = خالی_اشاریہ;

عوامی:
  Rope_Piece_BTree_برگ() : Rope_Piece_BTree_گرہ(سچ) {}

  ~Rope_Piece_BTree_برگ() {
    اگر (پچھلا_برگ || اگلا_برگ)
      remove_From_برگ_اندر_ترتیب();
    clear();
  }

  منطقی is_بھرا() ثابت { واپس عدد_Pieces == 2 * چوڑائی_ضربی_عدد; }

  /// clear - Remove all rope pieces from this leaf.
  خالی clear() {
    جب (عدد_Pieces)
      Pieces[--عدد_Pieces] = RopePiece();
    سائز = 0;
  }

  غیرعلامتی get_عدد_Pieces() ثابت { واپس عدد_Pieces; }

  ثابت RopePiece &getPiece(غیرعلامتی i) ثابت {
    أكد(i < get_عدد_Pieces() && "Invalid piece ID");
    واپس Pieces[i];
  }

  ثابت Rope_Piece_BTree_برگ *get_اگلا_برگ_اندر_ترتیب() ثابت { واپس اگلا_برگ; }

  خالی داخل_کرو_After_برگ_اندر_ترتیب(Rope_Piece_BTree_برگ *گرہ) {
    أكد(!پچھلا_برگ && !اگلا_برگ && "Already in ordering");

    اگلا_برگ = گرہ->اگلا_برگ;
    اگر (اگلا_برگ)
      اگلا_برگ->پچھلا_برگ = &اگلا_برگ;
    پچھلا_برگ = &گرہ->اگلا_برگ;
    گرہ->اگلا_برگ = یہ;
  }

  خالی remove_From_برگ_اندر_ترتیب() {
    اگر (پچھلا_برگ) {
      *پچھلا_برگ = اگلا_برگ;
      اگر (اگلا_برگ)
        اگلا_برگ->پچھلا_برگ = پچھلا_برگ;
    } ورنہ اگر (اگلا_برگ) {
      اگلا_برگ->پچھلا_برگ = خالی_اشاریہ;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  خالی بھرا_Recompute_سائز_Locally() {
    سائز = 0;
    برائے (غیرعلامتی i = 0, e = get_عدد_Pieces(); i != e; ++i)
      سائز += getPiece(i).سائز_2();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گرہ *بانٹو(غیرعلامتی انحراف);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گرہ *داخل_کرو(غیرعلامتی انحراف, ثابت RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  خالی erase(غیرعلامتی انحراف, غیرعلامتی عدد_بائٹیں);

  ساکن منطقی classof(ثابت Rope_Piece_BTree_گرہ *N) { واپس N->is_برگ(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گرہ *Rope_Piece_BTree_برگ::بانٹو(غیرعلامتی انحراف) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  اگر (انحراف == 0 || انحراف == سائز_2()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    واپس خالی_اشاریہ;
  }

  // Find the piece that this offset lands in.
  غیرعلامتی PieceOffs = 0;
  غیرعلامتی i = 0;
  جب (انحراف >= PieceOffs + Pieces[i].سائز_2()) {
    PieceOffs += Pieces[i].سائز_2();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  اگر (PieceOffs == انحراف)
    واپس خالی_اشاریہ;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  غیرعلامتی Intra_Piece_انحراف = انحراف - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece دم(Pieces[i].Str_ڈیٹا, Pieces[i].آغاز_Offs + Intra_Piece_انحراف,
                 Pieces[i].انتہا_Offs);
  سائز -= Pieces[i].سائز_2();
  Pieces[i].انتہا_Offs = Pieces[i].آغاز_Offs + Intra_Piece_انحراف;
  سائز += Pieces[i].سائز_2();

  واپس داخل_کرو(انحراف, دم);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گرہ *Rope_Piece_BTree_برگ::داخل_کرو(غیرعلامتی انحراف,
                                               ثابت RopePiece &R) {
  // If this node is not full, insert the piece.
  اگر (!is_بھرا()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    غیرعلامتی i = 0, e = get_عدد_Pieces();
    اگر (انحراف == سائز_2()) {
      // Fastpath for a common case.
      i = e;
    } ورنہ {
      غیرعلامتی خانہ_Offs = 0;
      برائے (; انحراف > خانہ_Offs; ++i)
        خانہ_Offs += getPiece(i).سائز_2();
      أكد(خانہ_Offs == انحراف && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    برائے (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++عدد_Pieces;
    سائز += R.سائز_2();
    واپس خالی_اشاریہ;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  Rope_Piece_BTree_برگ *New_گرہ = نیا Rope_Piece_BTree_برگ();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[چوڑائی_ضربی_عدد], &Pieces[2 * چوڑائی_ضربی_عدد],
            &New_گرہ->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[چوڑائی_ضربی_عدد], &Pieces[2 * چوڑائی_ضربی_عدد], RopePiece());

  // Decrease the number of values in the two nodes.
  New_گرہ->عدد_Pieces = عدد_Pieces = چوڑائی_ضربی_عدد;

  // Recompute the two nodes' size.
  New_گرہ->بھرا_Recompute_سائز_Locally();
  بھرا_Recompute_سائز_Locally();

  // Update the list of leaves.
  New_گرہ->داخل_کرو_After_برگ_اندر_ترتیب(یہ);

  // These insertions can't fail.
  اگر (یہ->سائز_2() >= انحراف)
    یہ->داخل_کرو(انحراف, R);
  ورنہ
    New_گرہ->داخل_کرو(انحراف - یہ->سائز_2(), R);
  واپس New_گرہ;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
خالی Rope_Piece_BTree_برگ::erase(غیرعلامتی انحراف, غیرعلامتی عدد_بائٹیں) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  غیرعلامتی PieceOffs = 0;
  غیرعلامتی i = 0;
  برائے (; انحراف > PieceOffs; ++i)
    PieceOffs += getPiece(i).سائز_2();
  أكد(PieceOffs == انحراف && "Split didn't occur before erase!");

  غیرعلامتی آغاز_Piece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  برائے (; انحراف + عدد_بائٹیں > PieceOffs + getPiece(i).سائز_2(); ++i)
    PieceOffs += getPiece(i).سائز_2();

  // If we exactly include the last one, include it in the region to delete.
  اگر (انحراف + عدد_بائٹیں == PieceOffs + getPiece(i).سائز_2()) {
    PieceOffs += getPiece(i).سائز_2();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  اگر (i != آغاز_Piece) {
    غیرعلامتی عدد_Deleted = i - آغاز_Piece;
    برائے (; i != get_عدد_Pieces(); ++i)
      Pieces[i - عدد_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_عدد_Pieces() - عدد_Deleted], &Pieces[get_عدد_Pieces()],
              RopePiece());
    عدد_Pieces -= عدد_Deleted;

    غیرعلامتی Cover_بائٹیں = PieceOffs - انحراف;
    عدد_بائٹیں -= Cover_بائٹیں;
    سائز -= Cover_بائٹیں;
  }

  // If we completely removed some stuff, we could be done.
  اگر (عدد_بائٹیں == 0)
    واپس;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  أكد(getPiece(آغاز_Piece).سائز_2() > عدد_بائٹیں);
  Pieces[آغاز_Piece].آغاز_Offs += عدد_بائٹیں;

  // The size of this node just shrunk by NumBytes.
  سائز -= عدد_بائٹیں;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

نامخلا {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
جماعت RopePieceBTreeInterior : عوامی Rope_Piece_BTree_گرہ {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  غیرعلامتی حرف عدد_Children = 0;

  Rope_Piece_BTree_گرہ *Children[2 * چوڑائی_ضربی_عدد];

عوامی:
  RopePieceBTreeInterior() : Rope_Piece_BTree_گرہ(جھوٹ) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_گرہ *LHS, Rope_Piece_BTree_گرہ *RHS)
      : Rope_Piece_BTree_گرہ(جھوٹ) {
    Children[0] = LHS;
    Children[1] = RHS;
    عدد_Children = 2;
    سائز = LHS->سائز_2() + RHS->سائز_2();
  }

  ~RopePieceBTreeInterior() {
    برائے (غیرعلامتی i = 0, e = get_عدد_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  منطقی is_بھرا() ثابت { واپس عدد_Children == 2 * چوڑائی_ضربی_عدد; }

  غیرعلامتی get_عدد_Children() ثابت { واپس عدد_Children; }

  ثابت Rope_Piece_BTree_گرہ *get_بچہ(غیرعلامتی i) ثابت {
    أكد(i < عدد_Children && "invalid child #");
    واپس Children[i];
  }

  Rope_Piece_BTree_گرہ *get_بچہ(غیرعلامتی i) {
    أكد(i < عدد_Children && "invalid child #");
    واپس Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  خالی بھرا_Recompute_سائز_Locally() {
    سائز = 0;
    برائے (غیرعلامتی i = 0, e = get_عدد_Children(); i != e; ++i)
      سائز += get_بچہ(i)->سائز_2();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گرہ *بانٹو(غیرعلامتی انحراف);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گرہ *داخل_کرو(غیرعلامتی انحراف, ثابت RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_گرہ *Handle_بچہ_Piece(غیرعلامتی i, Rope_Piece_BTree_گرہ *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  خالی erase(غیرعلامتی انحراف, غیرعلامتی عدد_بائٹیں);

  ساکن منطقی classof(ثابت Rope_Piece_BTree_گرہ *N) { واپس !N->is_برگ(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گرہ *RopePieceBTreeInterior::بانٹو(غیرعلامتی انحراف) {
  // Figure out which child to split.
  اگر (انحراف == 0 || انحراف == سائز_2())
    واپس خالی_اشاریہ; // If we have an exact offset, we're already split.

  غیرعلامتی بچہ_انحراف = 0;
  غیرعلامتی i = 0;
  برائے (; انحراف >= بچہ_انحراف + get_بچہ(i)->سائز_2(); ++i)
    بچہ_انحراف += get_بچہ(i)->سائز_2();

  // If already split there, we're done.
  اگر (بچہ_انحراف == انحراف)
    واپس خالی_اشاریہ;

  // Otherwise, recursively split the child.
  اگر (Rope_Piece_BTree_گرہ *RHS = get_بچہ(i)->بانٹو(انحراف - بچہ_انحراف))
    واپس Handle_بچہ_Piece(i, RHS);
  واپس خالی_اشاریہ; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گرہ *RopePieceBTreeInterior::داخل_کرو(غیرعلامتی انحراف,
                                                   ثابت RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  غیرعلامتی i = 0, e = get_عدد_Children();

  غیرعلامتی بچہ_Offs = 0;
  اگر (انحراف == سائز_2()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    بچہ_Offs = سائز_2() - get_بچہ(i)->سائز_2();
  } ورنہ {
    برائے (; انحراف > بچہ_Offs + get_بچہ(i)->سائز_2(); ++i)
      بچہ_Offs += get_بچہ(i)->سائز_2();
  }

  سائز += R.سائز_2();

  // Insert at the end of this child.
  اگر (Rope_Piece_BTree_گرہ *RHS = get_بچہ(i)->داخل_کرو(انحراف - بچہ_Offs, R))
    واپس Handle_بچہ_Piece(i, RHS);

  واپس خالی_اشاریہ;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_گرہ *
RopePieceBTreeInterior::Handle_بچہ_Piece(غیرعلامتی i, Rope_Piece_BTree_گرہ *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  اگر (!is_بھرا()) {
    // Insert RHS after child 'i'.
    اگر (i + 1 != get_عدد_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_عدد_Children() - i - 1) * حجم(Children[0]));
    Children[i + 1] = RHS;
    ++عدد_Children;
    واپس خالی_اشاریہ;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_گرہ = نیا RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  حافظہ_نقل(&New_گرہ->Children[0], &Children[چوڑائی_ضربی_عدد],
         چوڑائی_ضربی_عدد * حجم(Children[0]));

  // Decrease the number of values in the two nodes.
  New_گرہ->عدد_Children = عدد_Children = چوڑائی_ضربی_عدد;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  اگر (i < چوڑائی_ضربی_عدد)
    یہ->Handle_بچہ_Piece(i, RHS);
  ورنہ
    New_گرہ->Handle_بچہ_Piece(i - چوڑائی_ضربی_عدد, RHS);

  // Recompute the two nodes' size.
  New_گرہ->بھرا_Recompute_سائز_Locally();
  بھرا_Recompute_سائز_Locally();
  واپس New_گرہ;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
خالی RopePieceBTreeInterior::erase(غیرعلامتی انحراف, غیرعلامتی عدد_بائٹیں) {
  // This will shrink this node by NumBytes.
  سائز -= عدد_بائٹیں;

  // Find the first child that overlaps with Offset.
  غیرعلامتی i = 0;
  برائے (; انحراف >= get_بچہ(i)->سائز_2(); ++i)
    انحراف -= get_بچہ(i)->سائز_2();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  جب (عدد_بائٹیں) {
    Rope_Piece_BTree_گرہ *Cur_بچہ = get_بچہ(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    اگر (انحراف + عدد_بائٹیں < Cur_بچہ->سائز_2()) {
      Cur_بچہ->erase(انحراف, عدد_بائٹیں);
      واپس;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    اگر (انحراف) {
      غیرعلامتی بائٹیں_From_بچہ = Cur_بچہ->سائز_2() - انحراف;
      Cur_بچہ->erase(انحراف, بائٹیں_From_بچہ);
      عدد_بائٹیں -= بائٹیں_From_بچہ;
      // Start at the beginning of the next child.
      انحراف = 0;
      ++i;
      جاری;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    عدد_بائٹیں -= Cur_بچہ->سائز_2();
    Cur_بچہ->Destroy();
    --عدد_Children;
    اگر (i != get_عدد_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_عدد_Children() - i) * حجم(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

خالی Rope_Piece_BTree_گرہ::Destroy() {
  اگر (خودکار *برگ = dyn_cast<Rope_Piece_BTree_برگ>(یہ))
    حذف برگ;
  ورنہ
    حذف cast<RopePieceBTreeInterior>(یہ);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گرہ *Rope_Piece_BTree_گرہ::بانٹو(غیرعلامتی انحراف) {
  أكد(انحراف <= سائز_2() && "Invalid offset to split!");
  اگر (خودکار *برگ = dyn_cast<Rope_Piece_BTree_برگ>(یہ))
    واپس برگ->بانٹو(انحراف);
  واپس cast<RopePieceBTreeInterior>(یہ)->بانٹو(انحراف);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گرہ *Rope_Piece_BTree_گرہ::داخل_کرو(غیرعلامتی انحراف,
                                               ثابت RopePiece &R) {
  أكد(انحراف <= سائز_2() && "Invalid offset to insert!");
  اگر (خودکار *برگ = dyn_cast<Rope_Piece_BTree_برگ>(یہ))
    واپس برگ->داخل_کرو(انحراف, R);
  واپس cast<RopePieceBTreeInterior>(یہ)->داخل_کرو(انحراف, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
خالی Rope_Piece_BTree_گرہ::erase(غیرعلامتی انحراف, غیرعلامتی عدد_بائٹیں) {
  أكد(انحراف + عدد_بائٹیں <= سائز_2() && "Invalid offset to erase!");
  اگر (خودکار *برگ = dyn_cast<Rope_Piece_BTree_برگ>(یہ))
    واپس برگ->erase(انحراف, عدد_بائٹیں);
  واپس cast<RopePieceBTreeInterior>(یہ)->erase(انحراف, عدد_بائٹیں);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

ساکن ثابت Rope_Piece_BTree_برگ *getCN(ثابت خالی *P) {
  واپس static_cast<ثابت Rope_Piece_BTree_برگ *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(ثابت خالی *n) {
  ثابت خودکار *N = static_cast<ثابت Rope_Piece_BTree_گرہ *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  جب (ثابت خودکار *اندر = dyn_cast<RopePieceBTreeInterior>(N))
    N = اندر->get_بچہ(0);

  // We must have at least one leaf.
  Cur_گرہ = cast<Rope_Piece_BTree_برگ>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  جب (Cur_گرہ && getCN(Cur_گرہ)->get_عدد_Pieces() == 0)
    Cur_گرہ = getCN(Cur_گرہ)->get_اگلا_برگ_اندر_ترتیب();

  اگر (Cur_گرہ)
    CurPiece = &getCN(Cur_گرہ)->getPiece(0);
  ورنہ // Empty tree, this is an end() iterator.
    CurPiece = خالی_اشاریہ;
  CurChar = 0;
}

خالی RopePieceBTreeIterator::Move_مقصد_اگلا_Piece() {
  اگر (CurPiece !=
      &getCN(Cur_گرہ)->getPiece(getCN(Cur_گرہ)->get_عدد_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    واپس;
  }

  // Find the next non-empty leaf node.
  کرو
    Cur_گرہ = getCN(Cur_گرہ)->get_اگلا_برگ_اندر_ترتیب();
  جب (Cur_گرہ && getCN(Cur_گرہ)->get_عدد_Pieces() == 0);

  اگر (Cur_گرہ)
    CurPiece = &getCN(Cur_گرہ)->getPiece(0);
  ورنہ // Hit end().
    CurPiece = خالی_اشاریہ;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

ساکن Rope_Piece_BTree_گرہ *get_جڑ(خالی *P) {
  واپس static_cast<Rope_Piece_BTree_گرہ *>(P);
}

RopePieceBTree::RopePieceBTree() { جڑ_2 = نیا Rope_Piece_BTree_برگ(); }

RopePieceBTree::RopePieceBTree(ثابت RopePieceBTree &RHS) {
  أكد(RHS.خالی_خانے() && "Can't copy non-empty tree yet");
  جڑ_2 = نیا Rope_Piece_BTree_برگ();
}

RopePieceBTree::~RopePieceBTree() { get_جڑ(جڑ_2)->Destroy(); }

غیرعلامتی RopePieceBTree::سائز_2() ثابت { واپس get_جڑ(جڑ_2)->سائز_2(); }

خالی RopePieceBTree::clear() {
  اگر (خودکار *برگ = dyn_cast<Rope_Piece_BTree_برگ>(get_جڑ(جڑ_2)))
    برگ->clear();
  ورنہ {
    get_جڑ(جڑ_2)->Destroy();
    جڑ_2 = نیا Rope_Piece_BTree_برگ();
  }
}

خالی RopePieceBTree::داخل_کرو(غیرعلامتی انحراف, ثابت RopePiece &R) {
  // #1. Split at Offset.
  اگر (Rope_Piece_BTree_گرہ *RHS = get_جڑ(جڑ_2)->بانٹو(انحراف))
    جڑ_2 = نیا RopePieceBTreeInterior(get_جڑ(جڑ_2), RHS);

  // #2. Do the insertion.
  اگر (Rope_Piece_BTree_گرہ *RHS = get_جڑ(جڑ_2)->داخل_کرو(انحراف, R))
    جڑ_2 = نیا RopePieceBTreeInterior(get_جڑ(جڑ_2), RHS);
}

خالی RopePieceBTree::erase(غیرعلامتی انحراف, غیرعلامتی عدد_بائٹیں) {
  // #1. Split at Offset.
  اگر (Rope_Piece_BTree_گرہ *RHS = get_جڑ(جڑ_2)->بانٹو(انحراف))
    جڑ_2 = نیا RopePieceBTreeInterior(get_جڑ(جڑ_2), RHS);

  // #2. Do the erasing.
  get_جڑ(جڑ_2)->erase(انحراف, عدد_بائٹیں);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(ثابت حرف *آغاز, ثابت حرف *انتہا_2) {
  غیرعلامتی Len = انتہا_2 - آغاز;
  أكد(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  اگر (AllocOffs + Len <= Alloc_Chunk_سائز) {
    حافظہ_نقل(Alloc_بفر->ڈیٹا_2 + AllocOffs, آغاز, Len);
    AllocOffs += Len;
    واپس RopePiece(Alloc_بفر, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  اگر (Len > Alloc_Chunk_سائز) {
    غیرعلامتی سائز = انتہا_2 - آغاز + حجم(Rope_Ref_گنتی_String) - 1;
    خودکار *Res = reinterpret_cast<Rope_Ref_گنتی_String *>(نیا حرف[سائز]);
    Res->Ref_گنتی = 0;
    حافظہ_نقل(Res->ڈیٹا_2, آغاز, انتہا_2 - آغاز);
    واپس RopePiece(Res, 0, انتہا_2 - آغاز);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  غیرعلامتی Alloc_سائز = offsetof(Rope_Ref_گنتی_String, ڈیٹا_2) + Alloc_Chunk_سائز;
  خودکار *Res = reinterpret_cast<Rope_Ref_گنتی_String *>(نیا حرف[Alloc_سائز]);
  Res->Ref_گنتی = 0;
  حافظہ_نقل(Res->ڈیٹا_2, آغاز, Len);
  Alloc_بفر = Res;
  AllocOffs = Len;

  واپس RopePiece(Alloc_بفر, 0, Len);
}
