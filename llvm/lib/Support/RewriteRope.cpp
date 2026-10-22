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

#অন্তর্ভাব "llvm/ADT/RewriteRope.h"
#অন্তর্ভাব "llvm/Support/Casting.h"
#অন্তর্ভাব <algorithm>
#অন্তর্ভাব <cassert>
#অন্তর্ভাব <cstring>

using নামাকাশ llvm;

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

নামাকাশ {

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Class
//===----------------------------------------------------------------------===//

/// RopePieceBTreeNode - Common base class of RopePieceBTreeLeaf and
/// RopePieceBTreeInterior.  This provides some 'virtual' dispatching methods
/// and a flag that determines which subclass the instance is.  Also
/// important, this node knows the full extend of the node, including any
/// children that it has.  This allows efficient skipping over entire subtrees
/// when looking for an offset in the BTree.
শ্রেণী Rope_Piece_BTree_গ্রন্থি {
সুরক্ষিত:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  গণনা { WidthFactor = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  unsigned আকার_2 = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  যৌক্তিক IsLeaf;

  Rope_Piece_BTree_গ্রন্থি(যৌক্তিক isLeaf) : IsLeaf(isLeaf) {}
  ~Rope_Piece_BTree_গ্রন্থি() = পূর্বনির্ধারিত;

সর্বজনীন:
  যৌক্তিক isLeaf() ধ্রুবক { ফেরত IsLeaf; }
  unsigned আকার_3() ধ্রুবক { ফেরত আকার_2; }

  শূন্য Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_গ্রন্থি *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_গ্রন্থি *ঢোকাও(unsigned Offset, ধ্রুবক RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  শূন্য erase(unsigned Offset, unsigned অঙ্ক_Bytes);
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
শ্রেণী RopePieceBTreeLeaf : সর্বজনীন Rope_Piece_BTree_গ্রন্থি {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  unsigned অক্ষর অঙ্ক_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * WidthFactor];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  RopePieceBTreeLeaf **পূর্ব_Leaf = nullptr;
  RopePieceBTreeLeaf *পরবর্তী_Leaf = nullptr;

সর্বজনীন:
  RopePieceBTreeLeaf() : Rope_Piece_BTree_গ্রন্থি(সত্য) {}

  ~RopePieceBTreeLeaf() {
    যদি (পূর্ব_Leaf || পরবর্তী_Leaf)
      removeFromLeafInOrder();
    clear();
  }

  যৌক্তিক isFull() ধ্রুবক { ফেরত অঙ্ক_Pieces == 2 * WidthFactor; }

  /// clear - Remove all rope pieces from this leaf.
  শূন্য clear() {
    যখন (অঙ্ক_Pieces)
      Pieces[--অঙ্ক_Pieces] = RopePiece();
    আকার_2 = 0;
  }

  unsigned get_অঙ্ক_Pieces() ধ্রুবক { ফেরত অঙ্ক_Pieces; }

  ধ্রুবক RopePiece &getPiece(unsigned i) ধ্রুবক {
    প্রতিজ্ঞা(i < get_অঙ্ক_Pieces() && "Invalid piece ID");
    ফেরত Pieces[i];
  }

  ধ্রুবক RopePieceBTreeLeaf *get_পরবর্তী_Leaf_In_Order() ধ্রুবক { ফেরত পরবর্তী_Leaf; }

  শূন্য ঢোকাও_After_Leaf_In_Order(RopePieceBTreeLeaf *গ্রন্থি) {
    প্রতিজ্ঞা(!পূর্ব_Leaf && !পরবর্তী_Leaf && "Already in ordering");

    পরবর্তী_Leaf = গ্রন্থি->পরবর্তী_Leaf;
    যদি (পরবর্তী_Leaf)
      পরবর্তী_Leaf->পূর্ব_Leaf = &পরবর্তী_Leaf;
    পূর্ব_Leaf = &গ্রন্থি->পরবর্তী_Leaf;
    গ্রন্থি->পরবর্তী_Leaf = এই;
  }

  শূন্য removeFromLeafInOrder() {
    যদি (পূর্ব_Leaf) {
      *পূর্ব_Leaf = পরবর্তী_Leaf;
      যদি (পরবর্তী_Leaf)
        পরবর্তী_Leaf->পূর্ব_Leaf = পূর্ব_Leaf;
    } অন্যথা যদি (পরবর্তী_Leaf) {
      পরবর্তী_Leaf->পূর্ব_Leaf = nullptr;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  শূন্য Full_Recompute_আকার_Locally() {
    আকার_2 = 0;
    জন্য (unsigned i = 0, e = get_অঙ্ক_Pieces(); i != e; ++i)
      আকার_2 += getPiece(i).আকার_3();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_গ্রন্থি *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_গ্রন্থি *ঢোকাও(unsigned Offset, ধ্রুবক RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  শূন্য erase(unsigned Offset, unsigned অঙ্ক_Bytes);

  স্থির যৌক্তিক classof(ধ্রুবক Rope_Piece_BTree_গ্রন্থি *N) { ফেরত N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_গ্রন্থি *RopePieceBTreeLeaf::split(unsigned Offset) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  যদি (Offset == 0 || Offset == আকার_3()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    ফেরত nullptr;
  }

  // Find the piece that this offset lands in.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  যখন (Offset >= PieceOffs + Pieces[i].আকার_3()) {
    PieceOffs += Pieces[i].আকার_3();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  যদি (PieceOffs == Offset)
    ফেরত nullptr;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  unsigned IntraPieceOffset = Offset - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece লেজ(Pieces[i].Str_তথ্য, Pieces[i].StartOffs + IntraPieceOffset,
                 Pieces[i].EndOffs);
  আকার_2 -= Pieces[i].আকার_3();
  Pieces[i].EndOffs = Pieces[i].StartOffs + IntraPieceOffset;
  আকার_2 += Pieces[i].আকার_3();

  ফেরত ঢোকাও(Offset, লেজ);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_গ্রন্থি *RopePieceBTreeLeaf::ঢোকাও(unsigned Offset,
                                               ধ্রুবক RopePiece &R) {
  // If this node is not full, insert the piece.
  যদি (!isFull()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    unsigned i = 0, e = get_অঙ্ক_Pieces();
    যদি (Offset == আকার_3()) {
      // Fastpath for a common case.
      i = e;
    } অন্যথা {
      unsigned SlotOffs = 0;
      জন্য (; Offset > SlotOffs; ++i)
        SlotOffs += getPiece(i).আকার_3();
      প্রতিজ্ঞা(SlotOffs == Offset && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    জন্য (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++অঙ্ক_Pieces;
    আকার_2 += R.আকার_3();
    ফেরত nullptr;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  RopePieceBTreeLeaf *New_গ্রন্থি = নতুন RopePieceBTreeLeaf();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[WidthFactor], &Pieces[2 * WidthFactor],
            &New_গ্রন্থি->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[WidthFactor], &Pieces[2 * WidthFactor], RopePiece());

  // Decrease the number of values in the two nodes.
  New_গ্রন্থি->অঙ্ক_Pieces = অঙ্ক_Pieces = WidthFactor;

  // Recompute the two nodes' size.
  New_গ্রন্থি->Full_Recompute_আকার_Locally();
  Full_Recompute_আকার_Locally();

  // Update the list of leaves.
  New_গ্রন্থি->ঢোকাও_After_Leaf_In_Order(এই);

  // These insertions can't fail.
  যদি (এই->আকার_3() >= Offset)
    এই->ঢোকাও(Offset, R);
  অন্যথা
    New_গ্রন্থি->ঢোকাও(Offset - এই->আকার_3(), R);
  ফেরত New_গ্রন্থি;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
শূন্য RopePieceBTreeLeaf::erase(unsigned Offset, unsigned অঙ্ক_Bytes) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  জন্য (; Offset > PieceOffs; ++i)
    PieceOffs += getPiece(i).আকার_3();
  প্রতিজ্ঞা(PieceOffs == Offset && "Split didn't occur before erase!");

  unsigned StartPiece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  জন্য (; Offset + অঙ্ক_Bytes > PieceOffs + getPiece(i).আকার_3(); ++i)
    PieceOffs += getPiece(i).আকার_3();

  // If we exactly include the last one, include it in the region to delete.
  যদি (Offset + অঙ্ক_Bytes == PieceOffs + getPiece(i).আকার_3()) {
    PieceOffs += getPiece(i).আকার_3();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  যদি (i != StartPiece) {
    unsigned অঙ্ক_Deleted = i - StartPiece;
    জন্য (; i != get_অঙ্ক_Pieces(); ++i)
      Pieces[i - অঙ্ক_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_অঙ্ক_Pieces() - অঙ্ক_Deleted], &Pieces[get_অঙ্ক_Pieces()],
              RopePiece());
    অঙ্ক_Pieces -= অঙ্ক_Deleted;

    unsigned CoverBytes = PieceOffs - Offset;
    অঙ্ক_Bytes -= CoverBytes;
    আকার_2 -= CoverBytes;
  }

  // If we completely removed some stuff, we could be done.
  যদি (অঙ্ক_Bytes == 0)
    ফেরত;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  প্রতিজ্ঞা(getPiece(StartPiece).আকার_3() > অঙ্ক_Bytes);
  Pieces[StartPiece].StartOffs += অঙ্ক_Bytes;

  // The size of this node just shrunk by NumBytes.
  আকার_2 -= অঙ্ক_Bytes;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

নামাকাশ {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
শ্রেণী RopePieceBTreeInterior : সর্বজনীন Rope_Piece_BTree_গ্রন্থি {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  unsigned অক্ষর অঙ্ক_Children = 0;

  Rope_Piece_BTree_গ্রন্থি *Children[2 * WidthFactor];

সর্বজনীন:
  RopePieceBTreeInterior() : Rope_Piece_BTree_গ্রন্থি(মিথ্যা) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_গ্রন্থি *LHS, Rope_Piece_BTree_গ্রন্থি *RHS)
      : Rope_Piece_BTree_গ্রন্থি(মিথ্যা) {
    Children[0] = LHS;
    Children[1] = RHS;
    অঙ্ক_Children = 2;
    আকার_2 = LHS->আকার_3() + RHS->আকার_3();
  }

  ~RopePieceBTreeInterior() {
    জন্য (unsigned i = 0, e = get_অঙ্ক_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  যৌক্তিক isFull() ধ্রুবক { ফেরত অঙ্ক_Children == 2 * WidthFactor; }

  unsigned get_অঙ্ক_Children() ধ্রুবক { ফেরত অঙ্ক_Children; }

  ধ্রুবক Rope_Piece_BTree_গ্রন্থি *getChild(unsigned i) ধ্রুবক {
    প্রতিজ্ঞা(i < অঙ্ক_Children && "invalid child #");
    ফেরত Children[i];
  }

  Rope_Piece_BTree_গ্রন্থি *getChild(unsigned i) {
    প্রতিজ্ঞা(i < অঙ্ক_Children && "invalid child #");
    ফেরত Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  শূন্য Full_Recompute_আকার_Locally() {
    আকার_2 = 0;
    জন্য (unsigned i = 0, e = get_অঙ্ক_Children(); i != e; ++i)
      আকার_2 += getChild(i)->আকার_3();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_গ্রন্থি *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_গ্রন্থি *ঢোকাও(unsigned Offset, ধ্রুবক RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_গ্রন্থি *HandleChildPiece(unsigned i, Rope_Piece_BTree_গ্রন্থি *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  শূন্য erase(unsigned Offset, unsigned অঙ্ক_Bytes);

  স্থির যৌক্তিক classof(ধ্রুবক Rope_Piece_BTree_গ্রন্থি *N) { ফেরত !N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_গ্রন্থি *RopePieceBTreeInterior::split(unsigned Offset) {
  // Figure out which child to split.
  যদি (Offset == 0 || Offset == আকার_3())
    ফেরত nullptr; // If we have an exact offset, we're already split.

  unsigned ChildOffset = 0;
  unsigned i = 0;
  জন্য (; Offset >= ChildOffset + getChild(i)->আকার_3(); ++i)
    ChildOffset += getChild(i)->আকার_3();

  // If already split there, we're done.
  যদি (ChildOffset == Offset)
    ফেরত nullptr;

  // Otherwise, recursively split the child.
  যদি (Rope_Piece_BTree_গ্রন্থি *RHS = getChild(i)->split(Offset - ChildOffset))
    ফেরত HandleChildPiece(i, RHS);
  ফেরত nullptr; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_গ্রন্থি *RopePieceBTreeInterior::ঢোকাও(unsigned Offset,
                                                   ধ্রুবক RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  unsigned i = 0, e = get_অঙ্ক_Children();

  unsigned ChildOffs = 0;
  যদি (Offset == আকার_3()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    ChildOffs = আকার_3() - getChild(i)->আকার_3();
  } অন্যথা {
    জন্য (; Offset > ChildOffs + getChild(i)->আকার_3(); ++i)
      ChildOffs += getChild(i)->আকার_3();
  }

  আকার_2 += R.আকার_3();

  // Insert at the end of this child.
  যদি (Rope_Piece_BTree_গ্রন্থি *RHS = getChild(i)->ঢোকাও(Offset - ChildOffs, R))
    ফেরত HandleChildPiece(i, RHS);

  ফেরত nullptr;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_গ্রন্থি *
RopePieceBTreeInterior::HandleChildPiece(unsigned i, Rope_Piece_BTree_গ্রন্থি *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  যদি (!isFull()) {
    // Insert RHS after child 'i'.
    যদি (i + 1 != get_অঙ্ক_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_অঙ্ক_Children() - i - 1) * আকার(Children[0]));
    Children[i + 1] = RHS;
    ++অঙ্ক_Children;
    ফেরত nullptr;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_গ্রন্থি = নতুন RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  memcpy(&New_গ্রন্থি->Children[0], &Children[WidthFactor],
         WidthFactor * আকার(Children[0]));

  // Decrease the number of values in the two nodes.
  New_গ্রন্থি->অঙ্ক_Children = অঙ্ক_Children = WidthFactor;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  যদি (i < WidthFactor)
    এই->HandleChildPiece(i, RHS);
  অন্যথা
    New_গ্রন্থি->HandleChildPiece(i - WidthFactor, RHS);

  // Recompute the two nodes' size.
  New_গ্রন্থি->Full_Recompute_আকার_Locally();
  Full_Recompute_আকার_Locally();
  ফেরত New_গ্রন্থি;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
শূন্য RopePieceBTreeInterior::erase(unsigned Offset, unsigned অঙ্ক_Bytes) {
  // This will shrink this node by NumBytes.
  আকার_2 -= অঙ্ক_Bytes;

  // Find the first child that overlaps with Offset.
  unsigned i = 0;
  জন্য (; Offset >= getChild(i)->আকার_3(); ++i)
    Offset -= getChild(i)->আকার_3();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  যখন (অঙ্ক_Bytes) {
    Rope_Piece_BTree_গ্রন্থি *CurChild = getChild(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    যদি (Offset + অঙ্ক_Bytes < CurChild->আকার_3()) {
      CurChild->erase(Offset, অঙ্ক_Bytes);
      ফেরত;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    যদি (Offset) {
      unsigned BytesFromChild = CurChild->আকার_3() - Offset;
      CurChild->erase(Offset, BytesFromChild);
      অঙ্ক_Bytes -= BytesFromChild;
      // Start at the beginning of the next child.
      Offset = 0;
      ++i;
      চালু;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    অঙ্ক_Bytes -= CurChild->আকার_3();
    CurChild->Destroy();
    --অঙ্ক_Children;
    যদি (i != get_অঙ্ক_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_অঙ্ক_Children() - i) * আকার(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

শূন্য Rope_Piece_BTree_গ্রন্থি::Destroy() {
  যদি (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(এই))
    delete Leaf;
  অন্যথা
    delete cast<RopePieceBTreeInterior>(এই);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_গ্রন্থি *Rope_Piece_BTree_গ্রন্থি::split(unsigned Offset) {
  প্রতিজ্ঞা(Offset <= আকার_3() && "Invalid offset to split!");
  যদি (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(এই))
    ফেরত Leaf->split(Offset);
  ফেরত cast<RopePieceBTreeInterior>(এই)->split(Offset);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_গ্রন্থি *Rope_Piece_BTree_গ্রন্থি::ঢোকাও(unsigned Offset,
                                               ধ্রুবক RopePiece &R) {
  প্রতিজ্ঞা(Offset <= আকার_3() && "Invalid offset to insert!");
  যদি (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(এই))
    ফেরত Leaf->ঢোকাও(Offset, R);
  ফেরত cast<RopePieceBTreeInterior>(এই)->ঢোকাও(Offset, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
শূন্য Rope_Piece_BTree_গ্রন্থি::erase(unsigned Offset, unsigned অঙ্ক_Bytes) {
  প্রতিজ্ঞা(Offset + অঙ্ক_Bytes <= আকার_3() && "Invalid offset to erase!");
  যদি (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(এই))
    ফেরত Leaf->erase(Offset, অঙ্ক_Bytes);
  ফেরত cast<RopePieceBTreeInterior>(এই)->erase(Offset, অঙ্ক_Bytes);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

স্থির ধ্রুবক RopePieceBTreeLeaf *getCN(ধ্রুবক শূন্য *P) {
  ফেরত static_cast<ধ্রুবক RopePieceBTreeLeaf *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(ধ্রুবক শূন্য *n) {
  ধ্রুবক auto *N = static_cast<ধ্রুবক Rope_Piece_BTree_গ্রন্থি *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  যখন (ধ্রুবক auto *IN = dyn_cast<RopePieceBTreeInterior>(N))
    N = IN->getChild(0);

  // We must have at least one leaf.
  Cur_গ্রন্থি = cast<RopePieceBTreeLeaf>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  যখন (Cur_গ্রন্থি && getCN(Cur_গ্রন্থি)->get_অঙ্ক_Pieces() == 0)
    Cur_গ্রন্থি = getCN(Cur_গ্রন্থি)->get_পরবর্তী_Leaf_In_Order();

  যদি (Cur_গ্রন্থি)
    CurPiece = &getCN(Cur_গ্রন্থি)->getPiece(0);
  অন্যথা // Empty tree, this is an end() iterator.
    CurPiece = nullptr;
  CurChar = 0;
}

শূন্য RopePieceBTreeIterator::Move_To_পরবর্তী_Piece() {
  যদি (CurPiece !=
      &getCN(Cur_গ্রন্থি)->getPiece(getCN(Cur_গ্রন্থি)->get_অঙ্ক_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    ফেরত;
  }

  // Find the next non-empty leaf node.
  করো
    Cur_গ্রন্থি = getCN(Cur_গ্রন্থি)->get_পরবর্তী_Leaf_In_Order();
  যখন (Cur_গ্রন্থি && getCN(Cur_গ্রন্থি)->get_অঙ্ক_Pieces() == 0);

  যদি (Cur_গ্রন্থি)
    CurPiece = &getCN(Cur_গ্রন্থি)->getPiece(0);
  অন্যথা // Hit end().
    CurPiece = nullptr;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

স্থির Rope_Piece_BTree_গ্রন্থি *get_মূল(শূন্য *P) {
  ফেরত static_cast<Rope_Piece_BTree_গ্রন্থি *>(P);
}

RopePieceBTree::RopePieceBTree() { মূল_2 = নতুন RopePieceBTreeLeaf(); }

RopePieceBTree::RopePieceBTree(ধ্রুবক RopePieceBTree &RHS) {
  প্রতিজ্ঞা(RHS.empty() && "Can't copy non-empty tree yet");
  মূল_2 = নতুন RopePieceBTreeLeaf();
}

RopePieceBTree::~RopePieceBTree() { get_মূল(মূল_2)->Destroy(); }

unsigned RopePieceBTree::আকার_3() ধ্রুবক { ফেরত get_মূল(মূল_2)->আকার_3(); }

শূন্য RopePieceBTree::clear() {
  যদি (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(get_মূল(মূল_2)))
    Leaf->clear();
  অন্যথা {
    get_মূল(মূল_2)->Destroy();
    মূল_2 = নতুন RopePieceBTreeLeaf();
  }
}

শূন্য RopePieceBTree::ঢোকাও(unsigned Offset, ধ্রুবক RopePiece &R) {
  // #1. Split at Offset.
  যদি (Rope_Piece_BTree_গ্রন্থি *RHS = get_মূল(মূল_2)->split(Offset))
    মূল_2 = নতুন RopePieceBTreeInterior(get_মূল(মূল_2), RHS);

  // #2. Do the insertion.
  যদি (Rope_Piece_BTree_গ্রন্থি *RHS = get_মূল(মূল_2)->ঢোকাও(Offset, R))
    মূল_2 = নতুন RopePieceBTreeInterior(get_মূল(মূল_2), RHS);
}

শূন্য RopePieceBTree::erase(unsigned Offset, unsigned অঙ্ক_Bytes) {
  // #1. Split at Offset.
  যদি (Rope_Piece_BTree_গ্রন্থি *RHS = get_মূল(মূল_2)->split(Offset))
    মূল_2 = নতুন RopePieceBTreeInterior(get_মূল(মূল_2), RHS);

  // #2. Do the erasing.
  get_মূল(মূল_2)->erase(Offset, অঙ্ক_Bytes);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(ধ্রুবক অক্ষর *Start, ধ্রুবক অক্ষর *End) {
  unsigned Len = End - Start;
  প্রতিজ্ঞা(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  যদি (AllocOffs + Len <= Alloc_Chunk_আকার) {
    memcpy(Alloc_বাফার->তথ্য_2 + AllocOffs, Start, Len);
    AllocOffs += Len;
    ফেরত RopePiece(Alloc_বাফার, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  যদি (Len > Alloc_Chunk_আকার) {
    unsigned আকার_2 = End - Start + আকার(Rope_Ref_গুনতি_String) - 1;
    auto *Res = reinterpret_cast<Rope_Ref_গুনতি_String *>(নতুন অক্ষর[আকার_2]);
    Res->Ref_গুনতি = 0;
    memcpy(Res->তথ্য_2, Start, End - Start);
    ফেরত RopePiece(Res, 0, End - Start);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  unsigned Alloc_আকার = offsetof(Rope_Ref_গুনতি_String, তথ্য_2) + Alloc_Chunk_আকার;
  auto *Res = reinterpret_cast<Rope_Ref_গুনতি_String *>(নতুন অক্ষর[Alloc_আকার]);
  Res->Ref_গুনতি = 0;
  memcpy(Res->তথ্য_2, Start, Len);
  Alloc_বাফার = Res;
  AllocOffs = Len;

  ফেরত RopePiece(Alloc_বাফার, 0, Len);
}
