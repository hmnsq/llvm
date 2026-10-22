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

#अन्तर्भाव "llvm/ADT/RewriteRope.h"
#अन्तर्भाव "llvm/Support/Casting.h"
#अन्तर्भाव <algorithm>
#अन्तर्भाव <cassert>
#अन्तर्भाव <cstring>

using नामाकाश llvm;

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

नामाकाश {

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Class
//===----------------------------------------------------------------------===//

/// RopePieceBTreeNode - Common base class of RopePieceBTreeLeaf and
/// RopePieceBTreeInterior.  This provides some 'virtual' dispatching methods
/// and a flag that determines which subclass the instance is.  Also
/// important, this node knows the full extend of the node, including any
/// children that it has.  This allows efficient skipping over entire subtrees
/// when looking for an offset in the BTree.
वग्ग Rope_Piece_BTree_ग्रन्थिः {
रक्षित:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  गणना { WidthFactor = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  unsigned Size = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  तार्किक IsLeaf;

  Rope_Piece_BTree_ग्रन्थिः(तार्किक isLeaf) : IsLeaf(isLeaf) {}
  ~Rope_Piece_BTree_ग्रन्थिः() = स्वभाव;

सार्वजनिक:
  तार्किक isLeaf() ध्रुव { पटिनिवत्त IsLeaf; }
  unsigned size() ध्रुव { पटिनिवत्त Size; }

  शून्य Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ग्रन्थिः *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ग्रन्थिः *निवेशय(unsigned Offset, ध्रुव RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  शून्य erase(unsigned Offset, unsigned अङ्कः_Bytes);
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
वग्ग RopePieceBTreeLeaf : सार्वजनिक Rope_Piece_BTree_ग्रन्थिः {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  unsigned वर्ण अङ्कः_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * WidthFactor];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  RopePieceBTreeLeaf **PrevLeaf = nullptr;
  RopePieceBTreeLeaf *NextLeaf = nullptr;

सार्वजनिक:
  RopePieceBTreeLeaf() : Rope_Piece_BTree_ग्रन्थिः(सच्च) {}

  ~RopePieceBTreeLeaf() {
    सचे (PrevLeaf || NextLeaf)
      removeFromLeafInOrder();
    clear();
  }

  तार्किक isFull() ध्रुव { पटिनिवत्त अङ्कः_Pieces == 2 * WidthFactor; }

  /// clear - Remove all rope pieces from this leaf.
  शून्य clear() {
    याव (अङ्कः_Pieces)
      Pieces[--अङ्कः_Pieces] = RopePiece();
    Size = 0;
  }

  unsigned get_अङ्कः_Pieces() ध्रुव { पटिनिवत्त अङ्कः_Pieces; }

  ध्रुव RopePiece &getPiece(unsigned i) ध्रुव {
    प्रतिज्ञा(i < get_अङ्कः_Pieces() && "Invalid piece ID");
    पटिनिवत्त Pieces[i];
  }

  ध्रुव RopePieceBTreeLeaf *getNextLeafInOrder() ध्रुव { पटिनिवत्त NextLeaf; }

  शून्य निवेशय_After_Leaf_In_Order(RopePieceBTreeLeaf *ग्रन्थिः) {
    प्रतिज्ञा(!PrevLeaf && !NextLeaf && "Already in ordering");

    NextLeaf = ग्रन्थिः->NextLeaf;
    सचे (NextLeaf)
      NextLeaf->PrevLeaf = &NextLeaf;
    PrevLeaf = &ग्रन्थिः->NextLeaf;
    ग्रन्थिः->NextLeaf = एतत्;
  }

  शून्य removeFromLeafInOrder() {
    सचे (PrevLeaf) {
      *PrevLeaf = NextLeaf;
      सचे (NextLeaf)
        NextLeaf->PrevLeaf = PrevLeaf;
    } अञ्ञथा सचे (NextLeaf) {
      NextLeaf->PrevLeaf = nullptr;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  शून्य FullRecomputeSizeLocally() {
    Size = 0;
    अत्थाय (unsigned i = 0, e = get_अङ्कः_Pieces(); i != e; ++i)
      Size += getPiece(i).size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ग्रन्थिः *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ग्रन्थिः *निवेशय(unsigned Offset, ध्रुव RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  शून्य erase(unsigned Offset, unsigned अङ्कः_Bytes);

  स्थिर तार्किक classof(ध्रुव Rope_Piece_BTree_ग्रन्थिः *N) { पटिनिवत्त N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ग्रन्थिः *RopePieceBTreeLeaf::split(unsigned Offset) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  सचे (Offset == 0 || Offset == size()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    पटिनिवत्त nullptr;
  }

  // Find the piece that this offset lands in.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  याव (Offset >= PieceOffs + Pieces[i].size()) {
    PieceOffs += Pieces[i].size();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  सचे (PieceOffs == Offset)
    पटिनिवत्त nullptr;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  unsigned IntraPieceOffset = Offset - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece Tail(Pieces[i].StrData, Pieces[i].StartOffs + IntraPieceOffset,
                 Pieces[i].EndOffs);
  Size -= Pieces[i].size();
  Pieces[i].EndOffs = Pieces[i].StartOffs + IntraPieceOffset;
  Size += Pieces[i].size();

  पटिनिवत्त निवेशय(Offset, Tail);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ग्रन्थिः *RopePieceBTreeLeaf::निवेशय(unsigned Offset,
                                               ध्रुव RopePiece &R) {
  // If this node is not full, insert the piece.
  सचे (!isFull()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    unsigned i = 0, e = get_अङ्कः_Pieces();
    सचे (Offset == size()) {
      // Fastpath for a common case.
      i = e;
    } अञ्ञथा {
      unsigned SlotOffs = 0;
      अत्थाय (; Offset > SlotOffs; ++i)
        SlotOffs += getPiece(i).size();
      प्रतिज्ञा(SlotOffs == Offset && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    अत्थाय (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++अङ्कः_Pieces;
    Size += R.size();
    पटिनिवत्त nullptr;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  RopePieceBTreeLeaf *New_ग्रन्थिः = नूतन RopePieceBTreeLeaf();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[WidthFactor], &Pieces[2 * WidthFactor],
            &New_ग्रन्थिः->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[WidthFactor], &Pieces[2 * WidthFactor], RopePiece());

  // Decrease the number of values in the two nodes.
  New_ग्रन्थिः->अङ्कः_Pieces = अङ्कः_Pieces = WidthFactor;

  // Recompute the two nodes' size.
  New_ग्रन्थिः->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();

  // Update the list of leaves.
  New_ग्रन्थिः->निवेशय_After_Leaf_In_Order(एतत्);

  // These insertions can't fail.
  सचे (एतत्->size() >= Offset)
    एतत्->निवेशय(Offset, R);
  अञ्ञथा
    New_ग्रन्थिः->निवेशय(Offset - एतत्->size(), R);
  पटिनिवत्त New_ग्रन्थिः;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
शून्य RopePieceBTreeLeaf::erase(unsigned Offset, unsigned अङ्कः_Bytes) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  अत्थाय (; Offset > PieceOffs; ++i)
    PieceOffs += getPiece(i).size();
  प्रतिज्ञा(PieceOffs == Offset && "Split didn't occur before erase!");

  unsigned StartPiece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  अत्थाय (; Offset + अङ्कः_Bytes > PieceOffs + getPiece(i).size(); ++i)
    PieceOffs += getPiece(i).size();

  // If we exactly include the last one, include it in the region to delete.
  सचे (Offset + अङ्कः_Bytes == PieceOffs + getPiece(i).size()) {
    PieceOffs += getPiece(i).size();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  सचे (i != StartPiece) {
    unsigned अङ्कः_Deleted = i - StartPiece;
    अत्थाय (; i != get_अङ्कः_Pieces(); ++i)
      Pieces[i - अङ्कः_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_अङ्कः_Pieces() - अङ्कः_Deleted], &Pieces[get_अङ्कः_Pieces()],
              RopePiece());
    अङ्कः_Pieces -= अङ्कः_Deleted;

    unsigned CoverBytes = PieceOffs - Offset;
    अङ्कः_Bytes -= CoverBytes;
    Size -= CoverBytes;
  }

  // If we completely removed some stuff, we could be done.
  सचे (अङ्कः_Bytes == 0)
    पटिनिवत्त;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  प्रतिज्ञा(getPiece(StartPiece).size() > अङ्कः_Bytes);
  Pieces[StartPiece].StartOffs += अङ्कः_Bytes;

  // The size of this node just shrunk by NumBytes.
  Size -= अङ्कः_Bytes;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

नामाकाश {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
वग्ग RopePieceBTreeInterior : सार्वजनिक Rope_Piece_BTree_ग्रन्थिः {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  unsigned वर्ण अङ्कः_Children = 0;

  Rope_Piece_BTree_ग्रन्थिः *Children[2 * WidthFactor];

सार्वजनिक:
  RopePieceBTreeInterior() : Rope_Piece_BTree_ग्रन्थिः(मिच्छा) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_ग्रन्थिः *LHS, Rope_Piece_BTree_ग्रन्थिः *RHS)
      : Rope_Piece_BTree_ग्रन्थिः(मिच्छा) {
    Children[0] = LHS;
    Children[1] = RHS;
    अङ्कः_Children = 2;
    Size = LHS->size() + RHS->size();
  }

  ~RopePieceBTreeInterior() {
    अत्थाय (unsigned i = 0, e = get_अङ्कः_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  तार्किक isFull() ध्रुव { पटिनिवत्त अङ्कः_Children == 2 * WidthFactor; }

  unsigned get_अङ्कः_Children() ध्रुव { पटिनिवत्त अङ्कः_Children; }

  ध्रुव Rope_Piece_BTree_ग्रन्थिः *getChild(unsigned i) ध्रुव {
    प्रतिज्ञा(i < अङ्कः_Children && "invalid child #");
    पटिनिवत्त Children[i];
  }

  Rope_Piece_BTree_ग्रन्थिः *getChild(unsigned i) {
    प्रतिज्ञा(i < अङ्कः_Children && "invalid child #");
    पटिनिवत्त Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  शून्य FullRecomputeSizeLocally() {
    Size = 0;
    अत्थाय (unsigned i = 0, e = get_अङ्कः_Children(); i != e; ++i)
      Size += getChild(i)->size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ग्रन्थिः *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ग्रन्थिः *निवेशय(unsigned Offset, ध्रुव RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_ग्रन्थिः *HandleChildPiece(unsigned i, Rope_Piece_BTree_ग्रन्थिः *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  शून्य erase(unsigned Offset, unsigned अङ्कः_Bytes);

  स्थिर तार्किक classof(ध्रुव Rope_Piece_BTree_ग्रन्थिः *N) { पटिनिवत्त !N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ग्रन्थिः *RopePieceBTreeInterior::split(unsigned Offset) {
  // Figure out which child to split.
  सचे (Offset == 0 || Offset == size())
    पटिनिवत्त nullptr; // If we have an exact offset, we're already split.

  unsigned ChildOffset = 0;
  unsigned i = 0;
  अत्थाय (; Offset >= ChildOffset + getChild(i)->size(); ++i)
    ChildOffset += getChild(i)->size();

  // If already split there, we're done.
  सचे (ChildOffset == Offset)
    पटिनिवत्त nullptr;

  // Otherwise, recursively split the child.
  सचे (Rope_Piece_BTree_ग्रन्थिः *RHS = getChild(i)->split(Offset - ChildOffset))
    पटिनिवत्त HandleChildPiece(i, RHS);
  पटिनिवत्त nullptr; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ग्रन्थिः *RopePieceBTreeInterior::निवेशय(unsigned Offset,
                                                   ध्रुव RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  unsigned i = 0, e = get_अङ्कः_Children();

  unsigned ChildOffs = 0;
  सचे (Offset == size()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    ChildOffs = size() - getChild(i)->size();
  } अञ्ञथा {
    अत्थाय (; Offset > ChildOffs + getChild(i)->size(); ++i)
      ChildOffs += getChild(i)->size();
  }

  Size += R.size();

  // Insert at the end of this child.
  सचे (Rope_Piece_BTree_ग्रन्थिः *RHS = getChild(i)->निवेशय(Offset - ChildOffs, R))
    पटिनिवत्त HandleChildPiece(i, RHS);

  पटिनिवत्त nullptr;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_ग्रन्थिः *
RopePieceBTreeInterior::HandleChildPiece(unsigned i, Rope_Piece_BTree_ग्रन्थिः *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  सचे (!isFull()) {
    // Insert RHS after child 'i'.
    सचे (i + 1 != get_अङ्कः_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_अङ्कः_Children() - i - 1) * परिमाण(Children[0]));
    Children[i + 1] = RHS;
    ++अङ्कः_Children;
    पटिनिवत्त nullptr;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_ग्रन्थिः = नूतन RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  memcpy(&New_ग्रन्थिः->Children[0], &Children[WidthFactor],
         WidthFactor * परिमाण(Children[0]));

  // Decrease the number of values in the two nodes.
  New_ग्रन्थिः->अङ्कः_Children = अङ्कः_Children = WidthFactor;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  सचे (i < WidthFactor)
    एतत्->HandleChildPiece(i, RHS);
  अञ्ञथा
    New_ग्रन्थिः->HandleChildPiece(i - WidthFactor, RHS);

  // Recompute the two nodes' size.
  New_ग्रन्थिः->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();
  पटिनिवत्त New_ग्रन्थिः;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
शून्य RopePieceBTreeInterior::erase(unsigned Offset, unsigned अङ्कः_Bytes) {
  // This will shrink this node by NumBytes.
  Size -= अङ्कः_Bytes;

  // Find the first child that overlaps with Offset.
  unsigned i = 0;
  अत्थाय (; Offset >= getChild(i)->size(); ++i)
    Offset -= getChild(i)->size();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  याव (अङ्कः_Bytes) {
    Rope_Piece_BTree_ग्रन्थिः *CurChild = getChild(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    सचे (Offset + अङ्कः_Bytes < CurChild->size()) {
      CurChild->erase(Offset, अङ्कः_Bytes);
      पटिनिवत्त;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    सचे (Offset) {
      unsigned BytesFromChild = CurChild->size() - Offset;
      CurChild->erase(Offset, BytesFromChild);
      अङ्कः_Bytes -= BytesFromChild;
      // Start at the beginning of the next child.
      Offset = 0;
      ++i;
      अनुवर्त;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    अङ्कः_Bytes -= CurChild->size();
    CurChild->Destroy();
    --अङ्कः_Children;
    सचे (i != get_अङ्कः_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_अङ्कः_Children() - i) * परिमाण(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

शून्य Rope_Piece_BTree_ग्रन्थिः::Destroy() {
  सचे (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(एतत्))
    delete Leaf;
  अञ्ञथा
    delete cast<RopePieceBTreeInterior>(एतत्);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ग्रन्थिः *Rope_Piece_BTree_ग्रन्थिः::split(unsigned Offset) {
  प्रतिज्ञा(Offset <= size() && "Invalid offset to split!");
  सचे (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(एतत्))
    पटिनिवत्त Leaf->split(Offset);
  पटिनिवत्त cast<RopePieceBTreeInterior>(एतत्)->split(Offset);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ग्रन्थिः *Rope_Piece_BTree_ग्रन्थिः::निवेशय(unsigned Offset,
                                               ध्रुव RopePiece &R) {
  प्रतिज्ञा(Offset <= size() && "Invalid offset to insert!");
  सचे (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(एतत्))
    पटिनिवत्त Leaf->निवेशय(Offset, R);
  पटिनिवत्त cast<RopePieceBTreeInterior>(एतत्)->निवेशय(Offset, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
शून्य Rope_Piece_BTree_ग्रन्थिः::erase(unsigned Offset, unsigned अङ्कः_Bytes) {
  प्रतिज्ञा(Offset + अङ्कः_Bytes <= size() && "Invalid offset to erase!");
  सचे (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(एतत्))
    पटिनिवत्त Leaf->erase(Offset, अङ्कः_Bytes);
  पटिनिवत्त cast<RopePieceBTreeInterior>(एतत्)->erase(Offset, अङ्कः_Bytes);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

स्थिर ध्रुव RopePieceBTreeLeaf *getCN(ध्रुव शून्य *P) {
  पटिनिवत्त static_cast<ध्रुव RopePieceBTreeLeaf *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(ध्रुव शून्य *n) {
  ध्रुव auto *N = static_cast<ध्रुव Rope_Piece_BTree_ग्रन्थिः *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  याव (ध्रुव auto *IN = dyn_cast<RopePieceBTreeInterior>(N))
    N = IN->getChild(0);

  // We must have at least one leaf.
  Cur_ग्रन्थिः = cast<RopePieceBTreeLeaf>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  याव (Cur_ग्रन्थिः && getCN(Cur_ग्रन्थिः)->get_अङ्कः_Pieces() == 0)
    Cur_ग्रन्थिः = getCN(Cur_ग्रन्थिः)->getNextLeafInOrder();

  सचे (Cur_ग्रन्थिः)
    CurPiece = &getCN(Cur_ग्रन्थिः)->getPiece(0);
  अञ्ञथा // Empty tree, this is an end() iterator.
    CurPiece = nullptr;
  CurChar = 0;
}

शून्य RopePieceBTreeIterator::MoveToNextPiece() {
  सचे (CurPiece !=
      &getCN(Cur_ग्रन्थिः)->getPiece(getCN(Cur_ग्रन्थिः)->get_अङ्कः_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    पटिनिवत्त;
  }

  // Find the next non-empty leaf node.
  करोहि
    Cur_ग्रन्थिः = getCN(Cur_ग्रन्थिः)->getNextLeafInOrder();
  याव (Cur_ग्रन्थिः && getCN(Cur_ग्रन्थिः)->get_अङ्कः_Pieces() == 0);

  सचे (Cur_ग्रन्थिः)
    CurPiece = &getCN(Cur_ग्रन्थिः)->getPiece(0);
  अञ्ञथा // Hit end().
    CurPiece = nullptr;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

स्थिर Rope_Piece_BTree_ग्रन्थिः *get_मूलम्(शून्य *P) {
  पटिनिवत्त static_cast<Rope_Piece_BTree_ग्रन्थिः *>(P);
}

RopePieceBTree::RopePieceBTree() { मूलम्_2 = नूतन RopePieceBTreeLeaf(); }

RopePieceBTree::RopePieceBTree(ध्रुव RopePieceBTree &RHS) {
  प्रतिज्ञा(RHS.empty() && "Can't copy non-empty tree yet");
  मूलम्_2 = नूतन RopePieceBTreeLeaf();
}

RopePieceBTree::~RopePieceBTree() { get_मूलम्(मूलम्_2)->Destroy(); }

unsigned RopePieceBTree::size() ध्रुव { पटिनिवत्त get_मूलम्(मूलम्_2)->size(); }

शून्य RopePieceBTree::clear() {
  सचे (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(get_मूलम्(मूलम्_2)))
    Leaf->clear();
  अञ्ञथा {
    get_मूलम्(मूलम्_2)->Destroy();
    मूलम्_2 = नूतन RopePieceBTreeLeaf();
  }
}

शून्य RopePieceBTree::निवेशय(unsigned Offset, ध्रुव RopePiece &R) {
  // #1. Split at Offset.
  सचे (Rope_Piece_BTree_ग्रन्थिः *RHS = get_मूलम्(मूलम्_2)->split(Offset))
    मूलम्_2 = नूतन RopePieceBTreeInterior(get_मूलम्(मूलम्_2), RHS);

  // #2. Do the insertion.
  सचे (Rope_Piece_BTree_ग्रन्थिः *RHS = get_मूलम्(मूलम्_2)->निवेशय(Offset, R))
    मूलम्_2 = नूतन RopePieceBTreeInterior(get_मूलम्(मूलम्_2), RHS);
}

शून्य RopePieceBTree::erase(unsigned Offset, unsigned अङ्कः_Bytes) {
  // #1. Split at Offset.
  सचे (Rope_Piece_BTree_ग्रन्थिः *RHS = get_मूलम्(मूलम्_2)->split(Offset))
    मूलम्_2 = नूतन RopePieceBTreeInterior(get_मूलम्(मूलम्_2), RHS);

  // #2. Do the erasing.
  get_मूलम्(मूलम्_2)->erase(Offset, अङ्कः_Bytes);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(ध्रुव वर्ण *Start, ध्रुव वर्ण *End) {
  unsigned Len = End - Start;
  प्रतिज्ञा(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  सचे (AllocOffs + Len <= AllocChunkSize) {
    memcpy(AllocBuffer->Data + AllocOffs, Start, Len);
    AllocOffs += Len;
    पटिनिवत्त RopePiece(AllocBuffer, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  सचे (Len > AllocChunkSize) {
    unsigned Size = End - Start + परिमाण(Rope_Ref_गणनम्_String) - 1;
    auto *Res = reinterpret_cast<Rope_Ref_गणनम्_String *>(नूतन वर्ण[Size]);
    Res->Ref_गणनम् = 0;
    memcpy(Res->Data, Start, End - Start);
    पटिनिवत्त RopePiece(Res, 0, End - Start);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  unsigned AllocSize = offsetof(Rope_Ref_गणनम्_String, Data) + AllocChunkSize;
  auto *Res = reinterpret_cast<Rope_Ref_गणनम्_String *>(नूतन वर्ण[AllocSize]);
  Res->Ref_गणनम् = 0;
  memcpy(Res->Data, Start, Len);
  AllocBuffer = Res;
  AllocOffs = Len;

  पटिनिवत्त RopePiece(AllocBuffer, 0, Len);
}
