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
वर्ग Rope_Piece_BTree_गाठ {
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

  Rope_Piece_BTree_गाठ(तार्किक isLeaf) : IsLeaf(isLeaf) {}
  ~Rope_Piece_BTree_गाठ() = स्वभाव;

सार्वजनिक:
  तार्किक isLeaf() ध्रुव { प्रत्यागम IsLeaf; }
  unsigned size() ध्रुव { प्रत्यागम Size; }

  शून्य Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_गाठ *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_गाठ *घाला(unsigned Offset, ध्रुव RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  शून्य erase(unsigned Offset, unsigned अंक_Bytes);
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
वर्ग RopePieceBTreeLeaf : सार्वजनिक Rope_Piece_BTree_गाठ {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  unsigned वर्ण अंक_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * WidthFactor];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  RopePieceBTreeLeaf **PrevLeaf = nullptr;
  RopePieceBTreeLeaf *NextLeaf = nullptr;

सार्वजनिक:
  RopePieceBTreeLeaf() : Rope_Piece_BTree_गाठ(सत्य) {}

  ~RopePieceBTreeLeaf() {
    यदि (PrevLeaf || NextLeaf)
      removeFromLeafInOrder();
    clear();
  }

  तार्किक isFull() ध्रुव { प्रत्यागम अंक_Pieces == 2 * WidthFactor; }

  /// clear - Remove all rope pieces from this leaf.
  शून्य clear() {
    यावत् (अंक_Pieces)
      Pieces[--अंक_Pieces] = RopePiece();
    Size = 0;
  }

  unsigned get_अंक_Pieces() ध्रुव { प्रत्यागम अंक_Pieces; }

  ध्रुव RopePiece &getPiece(unsigned i) ध्रुव {
    प्रतिज्ञा(i < get_अंक_Pieces() && "Invalid piece ID");
    प्रत्यागम Pieces[i];
  }

  ध्रुव RopePieceBTreeLeaf *getNextLeafInOrder() ध्रुव { प्रत्यागम NextLeaf; }

  शून्य घाला_After_Leaf_In_Order(RopePieceBTreeLeaf *गाठ) {
    प्रतिज्ञा(!PrevLeaf && !NextLeaf && "Already in ordering");

    NextLeaf = गाठ->NextLeaf;
    यदि (NextLeaf)
      NextLeaf->PrevLeaf = &NextLeaf;
    PrevLeaf = &गाठ->NextLeaf;
    गाठ->NextLeaf = एतत्;
  }

  शून्य removeFromLeafInOrder() {
    यदि (PrevLeaf) {
      *PrevLeaf = NextLeaf;
      यदि (NextLeaf)
        NextLeaf->PrevLeaf = PrevLeaf;
    } अन्यथा यदि (NextLeaf) {
      NextLeaf->PrevLeaf = nullptr;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  शून्य FullRecomputeSizeLocally() {
    Size = 0;
    प्रति (unsigned i = 0, e = get_अंक_Pieces(); i != e; ++i)
      Size += getPiece(i).size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_गाठ *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_गाठ *घाला(unsigned Offset, ध्रुव RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  शून्य erase(unsigned Offset, unsigned अंक_Bytes);

  स्थिर तार्किक classof(ध्रुव Rope_Piece_BTree_गाठ *N) { प्रत्यागम N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_गाठ *RopePieceBTreeLeaf::split(unsigned Offset) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  यदि (Offset == 0 || Offset == size()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    प्रत्यागम nullptr;
  }

  // Find the piece that this offset lands in.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  यावत् (Offset >= PieceOffs + Pieces[i].size()) {
    PieceOffs += Pieces[i].size();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  यदि (PieceOffs == Offset)
    प्रत्यागम nullptr;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  unsigned IntraPieceOffset = Offset - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece Tail(Pieces[i].StrData, Pieces[i].StartOffs + IntraPieceOffset,
                 Pieces[i].EndOffs);
  Size -= Pieces[i].size();
  Pieces[i].EndOffs = Pieces[i].StartOffs + IntraPieceOffset;
  Size += Pieces[i].size();

  प्रत्यागम घाला(Offset, Tail);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_गाठ *RopePieceBTreeLeaf::घाला(unsigned Offset,
                                               ध्रुव RopePiece &R) {
  // If this node is not full, insert the piece.
  यदि (!isFull()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    unsigned i = 0, e = get_अंक_Pieces();
    यदि (Offset == size()) {
      // Fastpath for a common case.
      i = e;
    } अन्यथा {
      unsigned SlotOffs = 0;
      प्रति (; Offset > SlotOffs; ++i)
        SlotOffs += getPiece(i).size();
      प्रतिज्ञा(SlotOffs == Offset && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    प्रति (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++अंक_Pieces;
    Size += R.size();
    प्रत्यागम nullptr;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  RopePieceBTreeLeaf *New_गाठ = नूतन RopePieceBTreeLeaf();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[WidthFactor], &Pieces[2 * WidthFactor],
            &New_गाठ->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[WidthFactor], &Pieces[2 * WidthFactor], RopePiece());

  // Decrease the number of values in the two nodes.
  New_गाठ->अंक_Pieces = अंक_Pieces = WidthFactor;

  // Recompute the two nodes' size.
  New_गाठ->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();

  // Update the list of leaves.
  New_गाठ->घाला_After_Leaf_In_Order(एतत्);

  // These insertions can't fail.
  यदि (एतत्->size() >= Offset)
    एतत्->घाला(Offset, R);
  अन्यथा
    New_गाठ->घाला(Offset - एतत्->size(), R);
  प्रत्यागम New_गाठ;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
शून्य RopePieceBTreeLeaf::erase(unsigned Offset, unsigned अंक_Bytes) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  प्रति (; Offset > PieceOffs; ++i)
    PieceOffs += getPiece(i).size();
  प्रतिज्ञा(PieceOffs == Offset && "Split didn't occur before erase!");

  unsigned StartPiece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  प्रति (; Offset + अंक_Bytes > PieceOffs + getPiece(i).size(); ++i)
    PieceOffs += getPiece(i).size();

  // If we exactly include the last one, include it in the region to delete.
  यदि (Offset + अंक_Bytes == PieceOffs + getPiece(i).size()) {
    PieceOffs += getPiece(i).size();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  यदि (i != StartPiece) {
    unsigned अंक_Deleted = i - StartPiece;
    प्रति (; i != get_अंक_Pieces(); ++i)
      Pieces[i - अंक_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_अंक_Pieces() - अंक_Deleted], &Pieces[get_अंक_Pieces()],
              RopePiece());
    अंक_Pieces -= अंक_Deleted;

    unsigned CoverBytes = PieceOffs - Offset;
    अंक_Bytes -= CoverBytes;
    Size -= CoverBytes;
  }

  // If we completely removed some stuff, we could be done.
  यदि (अंक_Bytes == 0)
    प्रत्यागम;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  प्रतिज्ञा(getPiece(StartPiece).size() > अंक_Bytes);
  Pieces[StartPiece].StartOffs += अंक_Bytes;

  // The size of this node just shrunk by NumBytes.
  Size -= अंक_Bytes;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

नामाकाश {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
वर्ग RopePieceBTreeInterior : सार्वजनिक Rope_Piece_BTree_गाठ {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  unsigned वर्ण अंक_Children = 0;

  Rope_Piece_BTree_गाठ *Children[2 * WidthFactor];

सार्वजनिक:
  RopePieceBTreeInterior() : Rope_Piece_BTree_गाठ(असत्य) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_गाठ *LHS, Rope_Piece_BTree_गाठ *RHS)
      : Rope_Piece_BTree_गाठ(असत्य) {
    Children[0] = LHS;
    Children[1] = RHS;
    अंक_Children = 2;
    Size = LHS->size() + RHS->size();
  }

  ~RopePieceBTreeInterior() {
    प्रति (unsigned i = 0, e = get_अंक_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  तार्किक isFull() ध्रुव { प्रत्यागम अंक_Children == 2 * WidthFactor; }

  unsigned get_अंक_Children() ध्रुव { प्रत्यागम अंक_Children; }

  ध्रुव Rope_Piece_BTree_गाठ *getChild(unsigned i) ध्रुव {
    प्रतिज्ञा(i < अंक_Children && "invalid child #");
    प्रत्यागम Children[i];
  }

  Rope_Piece_BTree_गाठ *getChild(unsigned i) {
    प्रतिज्ञा(i < अंक_Children && "invalid child #");
    प्रत्यागम Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  शून्य FullRecomputeSizeLocally() {
    Size = 0;
    प्रति (unsigned i = 0, e = get_अंक_Children(); i != e; ++i)
      Size += getChild(i)->size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_गाठ *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_गाठ *घाला(unsigned Offset, ध्रुव RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_गाठ *HandleChildPiece(unsigned i, Rope_Piece_BTree_गाठ *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  शून्य erase(unsigned Offset, unsigned अंक_Bytes);

  स्थिर तार्किक classof(ध्रुव Rope_Piece_BTree_गाठ *N) { प्रत्यागम !N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_गाठ *RopePieceBTreeInterior::split(unsigned Offset) {
  // Figure out which child to split.
  यदि (Offset == 0 || Offset == size())
    प्रत्यागम nullptr; // If we have an exact offset, we're already split.

  unsigned ChildOffset = 0;
  unsigned i = 0;
  प्रति (; Offset >= ChildOffset + getChild(i)->size(); ++i)
    ChildOffset += getChild(i)->size();

  // If already split there, we're done.
  यदि (ChildOffset == Offset)
    प्रत्यागम nullptr;

  // Otherwise, recursively split the child.
  यदि (Rope_Piece_BTree_गाठ *RHS = getChild(i)->split(Offset - ChildOffset))
    प्रत्यागम HandleChildPiece(i, RHS);
  प्रत्यागम nullptr; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_गाठ *RopePieceBTreeInterior::घाला(unsigned Offset,
                                                   ध्रुव RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  unsigned i = 0, e = get_अंक_Children();

  unsigned ChildOffs = 0;
  यदि (Offset == size()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    ChildOffs = size() - getChild(i)->size();
  } अन्यथा {
    प्रति (; Offset > ChildOffs + getChild(i)->size(); ++i)
      ChildOffs += getChild(i)->size();
  }

  Size += R.size();

  // Insert at the end of this child.
  यदि (Rope_Piece_BTree_गाठ *RHS = getChild(i)->घाला(Offset - ChildOffs, R))
    प्रत्यागम HandleChildPiece(i, RHS);

  प्रत्यागम nullptr;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_गाठ *
RopePieceBTreeInterior::HandleChildPiece(unsigned i, Rope_Piece_BTree_गाठ *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  यदि (!isFull()) {
    // Insert RHS after child 'i'.
    यदि (i + 1 != get_अंक_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_अंक_Children() - i - 1) * परिमाण(Children[0]));
    Children[i + 1] = RHS;
    ++अंक_Children;
    प्रत्यागम nullptr;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_गाठ = नूतन RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  memcpy(&New_गाठ->Children[0], &Children[WidthFactor],
         WidthFactor * परिमाण(Children[0]));

  // Decrease the number of values in the two nodes.
  New_गाठ->अंक_Children = अंक_Children = WidthFactor;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  यदि (i < WidthFactor)
    एतत्->HandleChildPiece(i, RHS);
  अन्यथा
    New_गाठ->HandleChildPiece(i - WidthFactor, RHS);

  // Recompute the two nodes' size.
  New_गाठ->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();
  प्रत्यागम New_गाठ;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
शून्य RopePieceBTreeInterior::erase(unsigned Offset, unsigned अंक_Bytes) {
  // This will shrink this node by NumBytes.
  Size -= अंक_Bytes;

  // Find the first child that overlaps with Offset.
  unsigned i = 0;
  प्रति (; Offset >= getChild(i)->size(); ++i)
    Offset -= getChild(i)->size();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  यावत् (अंक_Bytes) {
    Rope_Piece_BTree_गाठ *CurChild = getChild(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    यदि (Offset + अंक_Bytes < CurChild->size()) {
      CurChild->erase(Offset, अंक_Bytes);
      प्रत्यागम;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    यदि (Offset) {
      unsigned BytesFromChild = CurChild->size() - Offset;
      CurChild->erase(Offset, BytesFromChild);
      अंक_Bytes -= BytesFromChild;
      // Start at the beginning of the next child.
      Offset = 0;
      ++i;
      अनुवर्त;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    अंक_Bytes -= CurChild->size();
    CurChild->Destroy();
    --अंक_Children;
    यदि (i != get_अंक_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_अंक_Children() - i) * परिमाण(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

शून्य Rope_Piece_BTree_गाठ::Destroy() {
  यदि (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(एतत्))
    delete Leaf;
  अन्यथा
    delete cast<RopePieceBTreeInterior>(एतत्);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_गाठ *Rope_Piece_BTree_गाठ::split(unsigned Offset) {
  प्रतिज्ञा(Offset <= size() && "Invalid offset to split!");
  यदि (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(एतत्))
    प्रत्यागम Leaf->split(Offset);
  प्रत्यागम cast<RopePieceBTreeInterior>(एतत्)->split(Offset);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_गाठ *Rope_Piece_BTree_गाठ::घाला(unsigned Offset,
                                               ध्रुव RopePiece &R) {
  प्रतिज्ञा(Offset <= size() && "Invalid offset to insert!");
  यदि (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(एतत्))
    प्रत्यागम Leaf->घाला(Offset, R);
  प्रत्यागम cast<RopePieceBTreeInterior>(एतत्)->घाला(Offset, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
शून्य Rope_Piece_BTree_गाठ::erase(unsigned Offset, unsigned अंक_Bytes) {
  प्रतिज्ञा(Offset + अंक_Bytes <= size() && "Invalid offset to erase!");
  यदि (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(एतत्))
    प्रत्यागम Leaf->erase(Offset, अंक_Bytes);
  प्रत्यागम cast<RopePieceBTreeInterior>(एतत्)->erase(Offset, अंक_Bytes);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

स्थिर ध्रुव RopePieceBTreeLeaf *getCN(ध्रुव शून्य *P) {
  प्रत्यागम static_cast<ध्रुव RopePieceBTreeLeaf *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(ध्रुव शून्य *n) {
  ध्रुव auto *N = static_cast<ध्रुव Rope_Piece_BTree_गाठ *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  यावत् (ध्रुव auto *IN = dyn_cast<RopePieceBTreeInterior>(N))
    N = IN->getChild(0);

  // We must have at least one leaf.
  Cur_गाठ = cast<RopePieceBTreeLeaf>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  यावत् (Cur_गाठ && getCN(Cur_गाठ)->get_अंक_Pieces() == 0)
    Cur_गाठ = getCN(Cur_गाठ)->getNextLeafInOrder();

  यदि (Cur_गाठ)
    CurPiece = &getCN(Cur_गाठ)->getPiece(0);
  अन्यथा // Empty tree, this is an end() iterator.
    CurPiece = nullptr;
  CurChar = 0;
}

शून्य RopePieceBTreeIterator::MoveToNextPiece() {
  यदि (CurPiece !=
      &getCN(Cur_गाठ)->getPiece(getCN(Cur_गाठ)->get_अंक_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    प्रत्यागम;
  }

  // Find the next non-empty leaf node.
  कुरु
    Cur_गाठ = getCN(Cur_गाठ)->getNextLeafInOrder();
  यावत् (Cur_गाठ && getCN(Cur_गाठ)->get_अंक_Pieces() == 0);

  यदि (Cur_गाठ)
    CurPiece = &getCN(Cur_गाठ)->getPiece(0);
  अन्यथा // Hit end().
    CurPiece = nullptr;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

स्थिर Rope_Piece_BTree_गाठ *get_मूळ(शून्य *P) {
  प्रत्यागम static_cast<Rope_Piece_BTree_गाठ *>(P);
}

RopePieceBTree::RopePieceBTree() { मूळ_2 = नूतन RopePieceBTreeLeaf(); }

RopePieceBTree::RopePieceBTree(ध्रुव RopePieceBTree &RHS) {
  प्रतिज्ञा(RHS.empty() && "Can't copy non-empty tree yet");
  मूळ_2 = नूतन RopePieceBTreeLeaf();
}

RopePieceBTree::~RopePieceBTree() { get_मूळ(मूळ_2)->Destroy(); }

unsigned RopePieceBTree::size() ध्रुव { प्रत्यागम get_मूळ(मूळ_2)->size(); }

शून्य RopePieceBTree::clear() {
  यदि (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(get_मूळ(मूळ_2)))
    Leaf->clear();
  अन्यथा {
    get_मूळ(मूळ_2)->Destroy();
    मूळ_2 = नूतन RopePieceBTreeLeaf();
  }
}

शून्य RopePieceBTree::घाला(unsigned Offset, ध्रुव RopePiece &R) {
  // #1. Split at Offset.
  यदि (Rope_Piece_BTree_गाठ *RHS = get_मूळ(मूळ_2)->split(Offset))
    मूळ_2 = नूतन RopePieceBTreeInterior(get_मूळ(मूळ_2), RHS);

  // #2. Do the insertion.
  यदि (Rope_Piece_BTree_गाठ *RHS = get_मूळ(मूळ_2)->घाला(Offset, R))
    मूळ_2 = नूतन RopePieceBTreeInterior(get_मूळ(मूळ_2), RHS);
}

शून्य RopePieceBTree::erase(unsigned Offset, unsigned अंक_Bytes) {
  // #1. Split at Offset.
  यदि (Rope_Piece_BTree_गाठ *RHS = get_मूळ(मूळ_2)->split(Offset))
    मूळ_2 = नूतन RopePieceBTreeInterior(get_मूळ(मूळ_2), RHS);

  // #2. Do the erasing.
  get_मूळ(मूळ_2)->erase(Offset, अंक_Bytes);
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
  यदि (AllocOffs + Len <= AllocChunkSize) {
    memcpy(AllocBuffer->Data + AllocOffs, Start, Len);
    AllocOffs += Len;
    प्रत्यागम RopePiece(AllocBuffer, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  यदि (Len > AllocChunkSize) {
    unsigned Size = End - Start + परिमाण(Rope_Ref_मोजणी_String) - 1;
    auto *Res = reinterpret_cast<Rope_Ref_मोजणी_String *>(नूतन वर्ण[Size]);
    Res->Ref_मोजणी = 0;
    memcpy(Res->Data, Start, End - Start);
    प्रत्यागम RopePiece(Res, 0, End - Start);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  unsigned AllocSize = offsetof(Rope_Ref_मोजणी_String, Data) + AllocChunkSize;
  auto *Res = reinterpret_cast<Rope_Ref_मोजणी_String *>(नूतन वर्ण[AllocSize]);
  Res->Ref_मोजणी = 0;
  memcpy(Res->Data, Start, Len);
  AllocBuffer = Res;
  AllocOffs = Len;

  प्रत्यागम RopePiece(AllocBuffer, 0, Len);
}
