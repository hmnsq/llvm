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

#समावेश "llvm/ADT/RewriteRope.h"
#समावेश "llvm/Support/Casting.h"
#समावेश <algorithm>
#समावेश <cassert>
#समावेश <cstring>

using namespace llvm;

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

namespace {

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Class
//===----------------------------------------------------------------------===//

/// RopePieceBTreeNode - Common base class of RopePieceBTreeLeaf and
/// RopePieceBTreeInterior.  This provides some 'virtual' dispatching methods
/// and a flag that determines which subclass the instance is.  Also
/// important, this node knows the full extend of the node, including any
/// children that it has.  This allows efficient skipping over entire subtrees
/// when looking for an offset in the BTree.
श्रेणी Rope_Piece_BTree_गाँठ {
रक्षित:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  क्रमागत { WidthFactor = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  अचिन्हित आकार = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  bool IsLeaf;

  Rope_Piece_BTree_गाँठ(bool isLeaf) : IsLeaf(isLeaf) {}
  ~Rope_Piece_BTree_गाँठ() = शेष;

खुला:
  bool isLeaf() स्थिर { वापस IsLeaf; }
  अचिन्हित आकार_2() स्थिर { वापस आकार; }

  व्योम Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_गाँठ *split(अचिन्हित Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_गाँठ *जोड़ो(अचिन्हित Offset, स्थिर RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  व्योम erase(अचिन्हित Offset, अचिन्हित अंक_Bytes);
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
श्रेणी RopePieceBTreeLeaf : खुला Rope_Piece_BTree_गाँठ {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  अचिन्हित अक्षर अंक_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * WidthFactor];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  RopePieceBTreeLeaf **पूर्व_Leaf = nullptr;
  RopePieceBTreeLeaf *अग्र_Leaf = nullptr;

खुला:
  RopePieceBTreeLeaf() : Rope_Piece_BTree_गाँठ(true) {}

  ~RopePieceBTreeLeaf() {
    अगर (पूर्व_Leaf || अग्र_Leaf)
      removeFromLeafInOrder();
    सफा();
  }

  bool isFull() स्थिर { वापस अंक_Pieces == 2 * WidthFactor; }

  /// clear - Remove all rope pieces from this leaf.
  व्योम सफा() {
    जबतक (अंक_Pieces)
      Pieces[--अंक_Pieces] = RopePiece();
    आकार = 0;
  }

  अचिन्हित get_अंक_Pieces() स्थिर { वापस अंक_Pieces; }

  स्थिर RopePiece &getPiece(अचिन्हित i) स्थिर {
    निश्चित(i < get_अंक_Pieces() && "Invalid piece ID");
    वापस Pieces[i];
  }

  स्थिर RopePieceBTreeLeaf *get_अग्र_Leaf_In_Order() स्थिर { वापस अग्र_Leaf; }

  व्योम जोड़ो_After_Leaf_In_Order(RopePieceBTreeLeaf *गाँठ) {
    निश्चित(!पूर्व_Leaf && !अग्र_Leaf && "Already in ordering");

    अग्र_Leaf = गाँठ->अग्र_Leaf;
    अगर (अग्र_Leaf)
      अग्र_Leaf->पूर्व_Leaf = &अग्र_Leaf;
    पूर्व_Leaf = &गाँठ->अग्र_Leaf;
    गाँठ->अग्र_Leaf = यह;
  }

  व्योम removeFromLeafInOrder() {
    अगर (पूर्व_Leaf) {
      *पूर्व_Leaf = अग्र_Leaf;
      अगर (अग्र_Leaf)
        अग्र_Leaf->पूर्व_Leaf = पूर्व_Leaf;
    } अन्यथा अगर (अग्र_Leaf) {
      अग्र_Leaf->पूर्व_Leaf = nullptr;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  व्योम Full_Recompute_आकार_Locally() {
    आकार = 0;
    क्रम (अचिन्हित i = 0, e = get_अंक_Pieces(); i != e; ++i)
      आकार += getPiece(i).आकार_2();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_गाँठ *split(अचिन्हित Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_गाँठ *जोड़ो(अचिन्हित Offset, स्थिर RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  व्योम erase(अचिन्हित Offset, अचिन्हित अंक_Bytes);

  अटल bool classof(स्थिर Rope_Piece_BTree_गाँठ *N) { वापस N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_गाँठ *RopePieceBTreeLeaf::split(अचिन्हित Offset) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  अगर (Offset == 0 || Offset == आकार_2()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    वापस nullptr;
  }

  // Find the piece that this offset lands in.
  अचिन्हित PieceOffs = 0;
  अचिन्हित i = 0;
  जबतक (Offset >= PieceOffs + Pieces[i].आकार_2()) {
    PieceOffs += Pieces[i].आकार_2();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  अगर (PieceOffs == Offset)
    वापस nullptr;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  अचिन्हित IntraPieceOffset = Offset - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece पुच्छ(Pieces[i].Str_तथ्य, Pieces[i].StartOffs + IntraPieceOffset,
                 Pieces[i].EndOffs);
  आकार -= Pieces[i].आकार_2();
  Pieces[i].EndOffs = Pieces[i].StartOffs + IntraPieceOffset;
  आकार += Pieces[i].आकार_2();

  वापस जोड़ो(Offset, पुच्छ);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_गाँठ *RopePieceBTreeLeaf::जोड़ो(अचिन्हित Offset,
                                               स्थिर RopePiece &R) {
  // If this node is not full, insert the piece.
  अगर (!isFull()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    अचिन्हित i = 0, e = get_अंक_Pieces();
    अगर (Offset == आकार_2()) {
      // Fastpath for a common case.
      i = e;
    } अन्यथा {
      अचिन्हित SlotOffs = 0;
      क्रम (; Offset > SlotOffs; ++i)
        SlotOffs += getPiece(i).आकार_2();
      निश्चित(SlotOffs == Offset && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    क्रम (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++अंक_Pieces;
    आकार += R.आकार_2();
    वापस nullptr;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  RopePieceBTreeLeaf *New_गाँठ = नया RopePieceBTreeLeaf();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[WidthFactor], &Pieces[2 * WidthFactor],
            &New_गाँठ->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[WidthFactor], &Pieces[2 * WidthFactor], RopePiece());

  // Decrease the number of values in the two nodes.
  New_गाँठ->अंक_Pieces = अंक_Pieces = WidthFactor;

  // Recompute the two nodes' size.
  New_गाँठ->Full_Recompute_आकार_Locally();
  Full_Recompute_आकार_Locally();

  // Update the list of leaves.
  New_गाँठ->जोड़ो_After_Leaf_In_Order(यह);

  // These insertions can't fail.
  अगर (यह->आकार_2() >= Offset)
    यह->जोड़ो(Offset, R);
  अन्यथा
    New_गाँठ->जोड़ो(Offset - यह->आकार_2(), R);
  वापस New_गाँठ;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
व्योम RopePieceBTreeLeaf::erase(अचिन्हित Offset, अचिन्हित अंक_Bytes) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  अचिन्हित PieceOffs = 0;
  अचिन्हित i = 0;
  क्रम (; Offset > PieceOffs; ++i)
    PieceOffs += getPiece(i).आकार_2();
  निश्चित(PieceOffs == Offset && "Split didn't occur before erase!");

  अचिन्हित StartPiece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  क्रम (; Offset + अंक_Bytes > PieceOffs + getPiece(i).आकार_2(); ++i)
    PieceOffs += getPiece(i).आकार_2();

  // If we exactly include the last one, include it in the region to delete.
  अगर (Offset + अंक_Bytes == PieceOffs + getPiece(i).आकार_2()) {
    PieceOffs += getPiece(i).आकार_2();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  अगर (i != StartPiece) {
    अचिन्हित अंक_Deleted = i - StartPiece;
    क्रम (; i != get_अंक_Pieces(); ++i)
      Pieces[i - अंक_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_अंक_Pieces() - अंक_Deleted], &Pieces[get_अंक_Pieces()],
              RopePiece());
    अंक_Pieces -= अंक_Deleted;

    अचिन्हित CoverBytes = PieceOffs - Offset;
    अंक_Bytes -= CoverBytes;
    आकार -= CoverBytes;
  }

  // If we completely removed some stuff, we could be done.
  अगर (अंक_Bytes == 0)
    वापस;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  निश्चित(getPiece(StartPiece).आकार_2() > अंक_Bytes);
  Pieces[StartPiece].StartOffs += अंक_Bytes;

  // The size of this node just shrunk by NumBytes.
  आकार -= अंक_Bytes;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

namespace {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
श्रेणी RopePieceBTreeInterior : खुला Rope_Piece_BTree_गाँठ {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  अचिन्हित अक्षर अंक_Children = 0;

  Rope_Piece_BTree_गाँठ *Children[2 * WidthFactor];

खुला:
  RopePieceBTreeInterior() : Rope_Piece_BTree_गाँठ(false) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_गाँठ *LHS, Rope_Piece_BTree_गाँठ *RHS)
      : Rope_Piece_BTree_गाँठ(false) {
    Children[0] = LHS;
    Children[1] = RHS;
    अंक_Children = 2;
    आकार = LHS->आकार_2() + RHS->आकार_2();
  }

  ~RopePieceBTreeInterior() {
    क्रम (अचिन्हित i = 0, e = get_अंक_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  bool isFull() स्थिर { वापस अंक_Children == 2 * WidthFactor; }

  अचिन्हित get_अंक_Children() स्थिर { वापस अंक_Children; }

  स्थिर Rope_Piece_BTree_गाँठ *getChild(अचिन्हित i) स्थिर {
    निश्चित(i < अंक_Children && "invalid child #");
    वापस Children[i];
  }

  Rope_Piece_BTree_गाँठ *getChild(अचिन्हित i) {
    निश्चित(i < अंक_Children && "invalid child #");
    वापस Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  व्योम Full_Recompute_आकार_Locally() {
    आकार = 0;
    क्रम (अचिन्हित i = 0, e = get_अंक_Children(); i != e; ++i)
      आकार += getChild(i)->आकार_2();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_गाँठ *split(अचिन्हित Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_गाँठ *जोड़ो(अचिन्हित Offset, स्थिर RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_गाँठ *HandleChildPiece(अचिन्हित i, Rope_Piece_BTree_गाँठ *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  व्योम erase(अचिन्हित Offset, अचिन्हित अंक_Bytes);

  अटल bool classof(स्थिर Rope_Piece_BTree_गाँठ *N) { वापस !N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_गाँठ *RopePieceBTreeInterior::split(अचिन्हित Offset) {
  // Figure out which child to split.
  अगर (Offset == 0 || Offset == आकार_2())
    वापस nullptr; // If we have an exact offset, we're already split.

  अचिन्हित ChildOffset = 0;
  अचिन्हित i = 0;
  क्रम (; Offset >= ChildOffset + getChild(i)->आकार_2(); ++i)
    ChildOffset += getChild(i)->आकार_2();

  // If already split there, we're done.
  अगर (ChildOffset == Offset)
    वापस nullptr;

  // Otherwise, recursively split the child.
  अगर (Rope_Piece_BTree_गाँठ *RHS = getChild(i)->split(Offset - ChildOffset))
    वापस HandleChildPiece(i, RHS);
  वापस nullptr; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_गाँठ *RopePieceBTreeInterior::जोड़ो(अचिन्हित Offset,
                                                   स्थिर RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  अचिन्हित i = 0, e = get_अंक_Children();

  अचिन्हित ChildOffs = 0;
  अगर (Offset == आकार_2()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    ChildOffs = आकार_2() - getChild(i)->आकार_2();
  } अन्यथा {
    क्रम (; Offset > ChildOffs + getChild(i)->आकार_2(); ++i)
      ChildOffs += getChild(i)->आकार_2();
  }

  आकार += R.आकार_2();

  // Insert at the end of this child.
  अगर (Rope_Piece_BTree_गाँठ *RHS = getChild(i)->जोड़ो(Offset - ChildOffs, R))
    वापस HandleChildPiece(i, RHS);

  वापस nullptr;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_गाँठ *
RopePieceBTreeInterior::HandleChildPiece(अचिन्हित i, Rope_Piece_BTree_गाँठ *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  अगर (!isFull()) {
    // Insert RHS after child 'i'.
    अगर (i + 1 != get_अंक_Children())
      स_हटाओ(&Children[i + 2], &Children[i + 1],
              (get_अंक_Children() - i - 1) * माप(Children[0]));
    Children[i + 1] = RHS;
    ++अंक_Children;
    वापस nullptr;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_गाँठ = नया RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  स_नकल(&New_गाँठ->Children[0], &Children[WidthFactor],
         WidthFactor * माप(Children[0]));

  // Decrease the number of values in the two nodes.
  New_गाँठ->अंक_Children = अंक_Children = WidthFactor;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  अगर (i < WidthFactor)
    यह->HandleChildPiece(i, RHS);
  अन्यथा
    New_गाँठ->HandleChildPiece(i - WidthFactor, RHS);

  // Recompute the two nodes' size.
  New_गाँठ->Full_Recompute_आकार_Locally();
  Full_Recompute_आकार_Locally();
  वापस New_गाँठ;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
व्योम RopePieceBTreeInterior::erase(अचिन्हित Offset, अचिन्हित अंक_Bytes) {
  // This will shrink this node by NumBytes.
  आकार -= अंक_Bytes;

  // Find the first child that overlaps with Offset.
  अचिन्हित i = 0;
  क्रम (; Offset >= getChild(i)->आकार_2(); ++i)
    Offset -= getChild(i)->आकार_2();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  जबतक (अंक_Bytes) {
    Rope_Piece_BTree_गाँठ *CurChild = getChild(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    अगर (Offset + अंक_Bytes < CurChild->आकार_2()) {
      CurChild->erase(Offset, अंक_Bytes);
      वापस;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    अगर (Offset) {
      अचिन्हित BytesFromChild = CurChild->आकार_2() - Offset;
      CurChild->erase(Offset, BytesFromChild);
      अंक_Bytes -= BytesFromChild;
      // Start at the beginning of the next child.
      Offset = 0;
      ++i;
      जारी;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    अंक_Bytes -= CurChild->आकार_2();
    CurChild->Destroy();
    --अंक_Children;
    अगर (i != get_अंक_Children())
      स_हटाओ(&Children[i], &Children[i + 1],
              (get_अंक_Children() - i) * माप(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

व्योम Rope_Piece_BTree_गाँठ::Destroy() {
  अगर (स्वतः *Leaf = dyn_cast<RopePieceBTreeLeaf>(यह))
    मिटाओ Leaf;
  अन्यथा
    मिटाओ cast<RopePieceBTreeInterior>(यह);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_गाँठ *Rope_Piece_BTree_गाँठ::split(अचिन्हित Offset) {
  निश्चित(Offset <= आकार_2() && "Invalid offset to split!");
  अगर (स्वतः *Leaf = dyn_cast<RopePieceBTreeLeaf>(यह))
    वापस Leaf->split(Offset);
  वापस cast<RopePieceBTreeInterior>(यह)->split(Offset);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_गाँठ *Rope_Piece_BTree_गाँठ::जोड़ो(अचिन्हित Offset,
                                               स्थिर RopePiece &R) {
  निश्चित(Offset <= आकार_2() && "Invalid offset to insert!");
  अगर (स्वतः *Leaf = dyn_cast<RopePieceBTreeLeaf>(यह))
    वापस Leaf->जोड़ो(Offset, R);
  वापस cast<RopePieceBTreeInterior>(यह)->जोड़ो(Offset, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
व्योम Rope_Piece_BTree_गाँठ::erase(अचिन्हित Offset, अचिन्हित अंक_Bytes) {
  निश्चित(Offset + अंक_Bytes <= आकार_2() && "Invalid offset to erase!");
  अगर (स्वतः *Leaf = dyn_cast<RopePieceBTreeLeaf>(यह))
    वापस Leaf->erase(Offset, अंक_Bytes);
  वापस cast<RopePieceBTreeInterior>(यह)->erase(Offset, अंक_Bytes);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

अटल स्थिर RopePieceBTreeLeaf *getCN(स्थिर व्योम *P) {
  वापस static_cast<स्थिर RopePieceBTreeLeaf *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(स्थिर व्योम *n) {
  स्थिर स्वतः *N = static_cast<स्थिर Rope_Piece_BTree_गाँठ *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  जबतक (स्थिर स्वतः *IN = dyn_cast<RopePieceBTreeInterior>(N))
    N = IN->getChild(0);

  // We must have at least one leaf.
  Cur_गाँठ = cast<RopePieceBTreeLeaf>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  जबतक (Cur_गाँठ && getCN(Cur_गाँठ)->get_अंक_Pieces() == 0)
    Cur_गाँठ = getCN(Cur_गाँठ)->get_अग्र_Leaf_In_Order();

  अगर (Cur_गाँठ)
    CurPiece = &getCN(Cur_गाँठ)->getPiece(0);
  अन्यथा // Empty tree, this is an end() iterator.
    CurPiece = nullptr;
  CurChar = 0;
}

व्योम RopePieceBTreeIterator::Move_To_अग्र_Piece() {
  अगर (CurPiece !=
      &getCN(Cur_गाँठ)->getPiece(getCN(Cur_गाँठ)->get_अंक_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    वापस;
  }

  // Find the next non-empty leaf node.
  करो
    Cur_गाँठ = getCN(Cur_गाँठ)->get_अग्र_Leaf_In_Order();
  जबतक (Cur_गाँठ && getCN(Cur_गाँठ)->get_अंक_Pieces() == 0);

  अगर (Cur_गाँठ)
    CurPiece = &getCN(Cur_गाँठ)->getPiece(0);
  अन्यथा // Hit end().
    CurPiece = nullptr;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

अटल Rope_Piece_BTree_गाँठ *get_मूल(व्योम *P) {
  वापस static_cast<Rope_Piece_BTree_गाँठ *>(P);
}

RopePieceBTree::RopePieceBTree() { मूल_2 = नया RopePieceBTreeLeaf(); }

RopePieceBTree::RopePieceBTree(स्थिर RopePieceBTree &RHS) {
  निश्चित(RHS.empty() && "Can't copy non-empty tree yet");
  मूल_2 = नया RopePieceBTreeLeaf();
}

RopePieceBTree::~RopePieceBTree() { get_मूल(मूल_2)->Destroy(); }

अचिन्हित RopePieceBTree::आकार_2() स्थिर { वापस get_मूल(मूल_2)->आकार_2(); }

व्योम RopePieceBTree::सफा() {
  अगर (स्वतः *Leaf = dyn_cast<RopePieceBTreeLeaf>(get_मूल(मूल_2)))
    Leaf->सफा();
  अन्यथा {
    get_मूल(मूल_2)->Destroy();
    मूल_2 = नया RopePieceBTreeLeaf();
  }
}

व्योम RopePieceBTree::जोड़ो(अचिन्हित Offset, स्थिर RopePiece &R) {
  // #1. Split at Offset.
  अगर (Rope_Piece_BTree_गाँठ *RHS = get_मूल(मूल_2)->split(Offset))
    मूल_2 = नया RopePieceBTreeInterior(get_मूल(मूल_2), RHS);

  // #2. Do the insertion.
  अगर (Rope_Piece_BTree_गाँठ *RHS = get_मूल(मूल_2)->जोड़ो(Offset, R))
    मूल_2 = नया RopePieceBTreeInterior(get_मूल(मूल_2), RHS);
}

व्योम RopePieceBTree::erase(अचिन्हित Offset, अचिन्हित अंक_Bytes) {
  // #1. Split at Offset.
  अगर (Rope_Piece_BTree_गाँठ *RHS = get_मूल(मूल_2)->split(Offset))
    मूल_2 = नया RopePieceBTreeInterior(get_मूल(मूल_2), RHS);

  // #2. Do the erasing.
  get_मूल(मूल_2)->erase(Offset, अंक_Bytes);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(स्थिर अक्षर *Start, स्थिर अक्षर *End) {
  अचिन्हित Len = End - Start;
  निश्चित(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  अगर (AllocOffs + Len <= Alloc_Chunk_आकार) {
    स_नकल(Alloc_बफर->तथ्य_2 + AllocOffs, Start, Len);
    AllocOffs += Len;
    वापस RopePiece(Alloc_बफर, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  अगर (Len > Alloc_Chunk_आकार) {
    अचिन्हित आकार = End - Start + माप(Rope_Ref_गिनती_String) - 1;
    स्वतः *Res = reinterpret_cast<Rope_Ref_गिनती_String *>(नया अक्षर[आकार]);
    Res->Ref_गिनती = 0;
    स_नकल(Res->तथ्य_2, Start, End - Start);
    वापस RopePiece(Res, 0, End - Start);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  अचिन्हित Alloc_आकार = दुरत्व(Rope_Ref_गिनती_String, तथ्य_2) + Alloc_Chunk_आकार;
  स्वतः *Res = reinterpret_cast<Rope_Ref_गिनती_String *>(नया अक्षर[Alloc_आकार]);
  Res->Ref_गिनती = 0;
  स_नकल(Res->तथ्य_2, Start, Len);
  Alloc_बफर = Res;
  AllocOffs = Len;

  वापस RopePiece(Alloc_बफर, 0, Len);
}
