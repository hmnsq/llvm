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
वर्ग Rope_Piece_BTree_गाँठो {
रक्षित:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  गणना { WidthFactor = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  अचिन्हित Size = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  तार्किक IsLeaf;

  Rope_Piece_BTree_गाँठो(तार्किक isLeaf) : IsLeaf(isLeaf) {}
  ~Rope_Piece_BTree_गाँठो() = स्वभाव;

सार्वजनिक:
  तार्किक isLeaf() अचर { फर्काऊ IsLeaf; }
  अचिन्हित size() अचर { फर्काऊ Size; }

  शून्य Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_गाँठो *split(अचिन्हित Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_गाँठो *घुसाउ(अचिन्हित Offset, अचर RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  शून्य erase(अचिन्हित Offset, अचिन्हित अङ्क_Bytes);
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
वर्ग RopePieceBTreeLeaf : सार्वजनिक Rope_Piece_BTree_गाँठो {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  अचिन्हित वर्ण अङ्क_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * WidthFactor];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  RopePieceBTreeLeaf **PrevLeaf = nullptr;
  RopePieceBTreeLeaf *NextLeaf = nullptr;

सार्वजनिक:
  RopePieceBTreeLeaf() : Rope_Piece_BTree_गाँठो(सत्य) {}

  ~RopePieceBTreeLeaf() {
    यदि (PrevLeaf || NextLeaf)
      removeFromLeafInOrder();
    clear();
  }

  तार्किक isFull() अचर { फर्काऊ अङ्क_Pieces == 2 * WidthFactor; }

  /// clear - Remove all rope pieces from this leaf.
  शून्य clear() {
    जबसम्म (अङ्क_Pieces)
      Pieces[--अङ्क_Pieces] = RopePiece();
    Size = 0;
  }

  अचिन्हित get_अङ्क_Pieces() अचर { फर्काऊ अङ्क_Pieces; }

  अचर RopePiece &getPiece(अचिन्हित i) अचर {
    प्रतिज्ञा(i < get_अङ्क_Pieces() && "Invalid piece ID");
    फर्काऊ Pieces[i];
  }

  अचर RopePieceBTreeLeaf *getNextLeafInOrder() अचर { फर्काऊ NextLeaf; }

  शून्य घुसाउ_After_Leaf_In_Order(RopePieceBTreeLeaf *गाँठो) {
    प्रतिज्ञा(!PrevLeaf && !NextLeaf && "Already in ordering");

    NextLeaf = गाँठो->NextLeaf;
    यदि (NextLeaf)
      NextLeaf->PrevLeaf = &NextLeaf;
    PrevLeaf = &गाँठो->NextLeaf;
    गाँठो->NextLeaf = एतत्;
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
    लागि (अचिन्हित i = 0, e = get_अङ्क_Pieces(); i != e; ++i)
      Size += getPiece(i).size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_गाँठो *split(अचिन्हित Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_गाँठो *घुसाउ(अचिन्हित Offset, अचर RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  शून्य erase(अचिन्हित Offset, अचिन्हित अङ्क_Bytes);

  स्थिर तार्किक classof(अचर Rope_Piece_BTree_गाँठो *N) { फर्काऊ N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_गाँठो *RopePieceBTreeLeaf::split(अचिन्हित Offset) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  यदि (Offset == 0 || Offset == size()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    फर्काऊ nullptr;
  }

  // Find the piece that this offset lands in.
  अचिन्हित PieceOffs = 0;
  अचिन्हित i = 0;
  जबसम्म (Offset >= PieceOffs + Pieces[i].size()) {
    PieceOffs += Pieces[i].size();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  यदि (PieceOffs == Offset)
    फर्काऊ nullptr;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  अचिन्हित IntraPieceOffset = Offset - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece Tail(Pieces[i].StrData, Pieces[i].StartOffs + IntraPieceOffset,
                 Pieces[i].EndOffs);
  Size -= Pieces[i].size();
  Pieces[i].EndOffs = Pieces[i].StartOffs + IntraPieceOffset;
  Size += Pieces[i].size();

  फर्काऊ घुसाउ(Offset, Tail);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_गाँठो *RopePieceBTreeLeaf::घुसाउ(अचिन्हित Offset,
                                               अचर RopePiece &R) {
  // If this node is not full, insert the piece.
  यदि (!isFull()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    अचिन्हित i = 0, e = get_अङ्क_Pieces();
    यदि (Offset == size()) {
      // Fastpath for a common case.
      i = e;
    } अन्यथा {
      अचिन्हित SlotOffs = 0;
      लागि (; Offset > SlotOffs; ++i)
        SlotOffs += getPiece(i).size();
      प्रतिज्ञा(SlotOffs == Offset && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    लागि (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++अङ्क_Pieces;
    Size += R.size();
    फर्काऊ nullptr;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  RopePieceBTreeLeaf *New_गाँठो = नूतन RopePieceBTreeLeaf();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[WidthFactor], &Pieces[2 * WidthFactor],
            &New_गाँठो->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[WidthFactor], &Pieces[2 * WidthFactor], RopePiece());

  // Decrease the number of values in the two nodes.
  New_गाँठो->अङ्क_Pieces = अङ्क_Pieces = WidthFactor;

  // Recompute the two nodes' size.
  New_गाँठो->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();

  // Update the list of leaves.
  New_गाँठो->घुसाउ_After_Leaf_In_Order(एतत्);

  // These insertions can't fail.
  यदि (एतत्->size() >= Offset)
    एतत्->घुसाउ(Offset, R);
  अन्यथा
    New_गाँठो->घुसाउ(Offset - एतत्->size(), R);
  फर्काऊ New_गाँठो;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
शून्य RopePieceBTreeLeaf::erase(अचिन्हित Offset, अचिन्हित अङ्क_Bytes) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  अचिन्हित PieceOffs = 0;
  अचिन्हित i = 0;
  लागि (; Offset > PieceOffs; ++i)
    PieceOffs += getPiece(i).size();
  प्रतिज्ञा(PieceOffs == Offset && "Split didn't occur before erase!");

  अचिन्हित StartPiece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  लागि (; Offset + अङ्क_Bytes > PieceOffs + getPiece(i).size(); ++i)
    PieceOffs += getPiece(i).size();

  // If we exactly include the last one, include it in the region to delete.
  यदि (Offset + अङ्क_Bytes == PieceOffs + getPiece(i).size()) {
    PieceOffs += getPiece(i).size();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  यदि (i != StartPiece) {
    अचिन्हित अङ्क_Deleted = i - StartPiece;
    लागि (; i != get_अङ्क_Pieces(); ++i)
      Pieces[i - अङ्क_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_अङ्क_Pieces() - अङ्क_Deleted], &Pieces[get_अङ्क_Pieces()],
              RopePiece());
    अङ्क_Pieces -= अङ्क_Deleted;

    अचिन्हित CoverBytes = PieceOffs - Offset;
    अङ्क_Bytes -= CoverBytes;
    Size -= CoverBytes;
  }

  // If we completely removed some stuff, we could be done.
  यदि (अङ्क_Bytes == 0)
    फर्काऊ;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  प्रतिज्ञा(getPiece(StartPiece).size() > अङ्क_Bytes);
  Pieces[StartPiece].StartOffs += अङ्क_Bytes;

  // The size of this node just shrunk by NumBytes.
  Size -= अङ्क_Bytes;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

नामाकाश {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
वर्ग RopePieceBTreeInterior : सार्वजनिक Rope_Piece_BTree_गाँठो {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  अचिन्हित वर्ण अङ्क_Children = 0;

  Rope_Piece_BTree_गाँठो *Children[2 * WidthFactor];

सार्वजनिक:
  RopePieceBTreeInterior() : Rope_Piece_BTree_गाँठो(असत्य) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_गाँठो *LHS, Rope_Piece_BTree_गाँठो *RHS)
      : Rope_Piece_BTree_गाँठो(असत्य) {
    Children[0] = LHS;
    Children[1] = RHS;
    अङ्क_Children = 2;
    Size = LHS->size() + RHS->size();
  }

  ~RopePieceBTreeInterior() {
    लागि (अचिन्हित i = 0, e = get_अङ्क_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  तार्किक isFull() अचर { फर्काऊ अङ्क_Children == 2 * WidthFactor; }

  अचिन्हित get_अङ्क_Children() अचर { फर्काऊ अङ्क_Children; }

  अचर Rope_Piece_BTree_गाँठो *getChild(अचिन्हित i) अचर {
    प्रतिज्ञा(i < अङ्क_Children && "invalid child #");
    फर्काऊ Children[i];
  }

  Rope_Piece_BTree_गाँठो *getChild(अचिन्हित i) {
    प्रतिज्ञा(i < अङ्क_Children && "invalid child #");
    फर्काऊ Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  शून्य FullRecomputeSizeLocally() {
    Size = 0;
    लागि (अचिन्हित i = 0, e = get_अङ्क_Children(); i != e; ++i)
      Size += getChild(i)->size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_गाँठो *split(अचिन्हित Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_गाँठो *घुसाउ(अचिन्हित Offset, अचर RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_गाँठो *HandleChildPiece(अचिन्हित i, Rope_Piece_BTree_गाँठो *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  शून्य erase(अचिन्हित Offset, अचिन्हित अङ्क_Bytes);

  स्थिर तार्किक classof(अचर Rope_Piece_BTree_गाँठो *N) { फर्काऊ !N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_गाँठो *RopePieceBTreeInterior::split(अचिन्हित Offset) {
  // Figure out which child to split.
  यदि (Offset == 0 || Offset == size())
    फर्काऊ nullptr; // If we have an exact offset, we're already split.

  अचिन्हित ChildOffset = 0;
  अचिन्हित i = 0;
  लागि (; Offset >= ChildOffset + getChild(i)->size(); ++i)
    ChildOffset += getChild(i)->size();

  // If already split there, we're done.
  यदि (ChildOffset == Offset)
    फर्काऊ nullptr;

  // Otherwise, recursively split the child.
  यदि (Rope_Piece_BTree_गाँठो *RHS = getChild(i)->split(Offset - ChildOffset))
    फर्काऊ HandleChildPiece(i, RHS);
  फर्काऊ nullptr; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_गाँठो *RopePieceBTreeInterior::घुसाउ(अचिन्हित Offset,
                                                   अचर RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  अचिन्हित i = 0, e = get_अङ्क_Children();

  अचिन्हित ChildOffs = 0;
  यदि (Offset == size()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    ChildOffs = size() - getChild(i)->size();
  } अन्यथा {
    लागि (; Offset > ChildOffs + getChild(i)->size(); ++i)
      ChildOffs += getChild(i)->size();
  }

  Size += R.size();

  // Insert at the end of this child.
  यदि (Rope_Piece_BTree_गाँठो *RHS = getChild(i)->घुसाउ(Offset - ChildOffs, R))
    फर्काऊ HandleChildPiece(i, RHS);

  फर्काऊ nullptr;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_गाँठो *
RopePieceBTreeInterior::HandleChildPiece(अचिन्हित i, Rope_Piece_BTree_गाँठो *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  यदि (!isFull()) {
    // Insert RHS after child 'i'.
    यदि (i + 1 != get_अङ्क_Children())
      स_हटाओ(&Children[i + 2], &Children[i + 1],
              (get_अङ्क_Children() - i - 1) * परिमाण(Children[0]));
    Children[i + 1] = RHS;
    ++अङ्क_Children;
    फर्काऊ nullptr;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_गाँठो = नूतन RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  स_नकल(&New_गाँठो->Children[0], &Children[WidthFactor],
         WidthFactor * परिमाण(Children[0]));

  // Decrease the number of values in the two nodes.
  New_गाँठो->अङ्क_Children = अङ्क_Children = WidthFactor;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  यदि (i < WidthFactor)
    एतत्->HandleChildPiece(i, RHS);
  अन्यथा
    New_गाँठो->HandleChildPiece(i - WidthFactor, RHS);

  // Recompute the two nodes' size.
  New_गाँठो->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();
  फर्काऊ New_गाँठो;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
शून्य RopePieceBTreeInterior::erase(अचिन्हित Offset, अचिन्हित अङ्क_Bytes) {
  // This will shrink this node by NumBytes.
  Size -= अङ्क_Bytes;

  // Find the first child that overlaps with Offset.
  अचिन्हित i = 0;
  लागि (; Offset >= getChild(i)->size(); ++i)
    Offset -= getChild(i)->size();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  जबसम्म (अङ्क_Bytes) {
    Rope_Piece_BTree_गाँठो *CurChild = getChild(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    यदि (Offset + अङ्क_Bytes < CurChild->size()) {
      CurChild->erase(Offset, अङ्क_Bytes);
      फर्काऊ;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    यदि (Offset) {
      अचिन्हित BytesFromChild = CurChild->size() - Offset;
      CurChild->erase(Offset, BytesFromChild);
      अङ्क_Bytes -= BytesFromChild;
      // Start at the beginning of the next child.
      Offset = 0;
      ++i;
      जारी;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    अङ्क_Bytes -= CurChild->size();
    CurChild->Destroy();
    --अङ्क_Children;
    यदि (i != get_अङ्क_Children())
      स_हटाओ(&Children[i], &Children[i + 1],
              (get_अङ्क_Children() - i) * परिमाण(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

शून्य Rope_Piece_BTree_गाँठो::Destroy() {
  यदि (स्वतः *Leaf = dyn_cast<RopePieceBTreeLeaf>(एतत्))
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
Rope_Piece_BTree_गाँठो *Rope_Piece_BTree_गाँठो::split(अचिन्हित Offset) {
  प्रतिज्ञा(Offset <= size() && "Invalid offset to split!");
  यदि (स्वतः *Leaf = dyn_cast<RopePieceBTreeLeaf>(एतत्))
    फर्काऊ Leaf->split(Offset);
  फर्काऊ cast<RopePieceBTreeInterior>(एतत्)->split(Offset);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_गाँठो *Rope_Piece_BTree_गाँठो::घुसाउ(अचिन्हित Offset,
                                               अचर RopePiece &R) {
  प्रतिज्ञा(Offset <= size() && "Invalid offset to insert!");
  यदि (स्वतः *Leaf = dyn_cast<RopePieceBTreeLeaf>(एतत्))
    फर्काऊ Leaf->घुसाउ(Offset, R);
  फर्काऊ cast<RopePieceBTreeInterior>(एतत्)->घुसाउ(Offset, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
शून्य Rope_Piece_BTree_गाँठो::erase(अचिन्हित Offset, अचिन्हित अङ्क_Bytes) {
  प्रतिज्ञा(Offset + अङ्क_Bytes <= size() && "Invalid offset to erase!");
  यदि (स्वतः *Leaf = dyn_cast<RopePieceBTreeLeaf>(एतत्))
    फर्काऊ Leaf->erase(Offset, अङ्क_Bytes);
  फर्काऊ cast<RopePieceBTreeInterior>(एतत्)->erase(Offset, अङ्क_Bytes);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

स्थिर अचर RopePieceBTreeLeaf *getCN(अचर शून्य *P) {
  फर्काऊ static_cast<अचर RopePieceBTreeLeaf *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(अचर शून्य *n) {
  अचर स्वतः *N = static_cast<अचर Rope_Piece_BTree_गाँठो *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  जबसम्म (अचर स्वतः *IN = dyn_cast<RopePieceBTreeInterior>(N))
    N = IN->getChild(0);

  // We must have at least one leaf.
  Cur_गाँठो = cast<RopePieceBTreeLeaf>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  जबसम्म (Cur_गाँठो && getCN(Cur_गाँठो)->get_अङ्क_Pieces() == 0)
    Cur_गाँठो = getCN(Cur_गाँठो)->getNextLeafInOrder();

  यदि (Cur_गाँठो)
    CurPiece = &getCN(Cur_गाँठो)->getPiece(0);
  अन्यथा // Empty tree, this is an end() iterator.
    CurPiece = nullptr;
  CurChar = 0;
}

शून्य RopePieceBTreeIterator::MoveToNextPiece() {
  यदि (CurPiece !=
      &getCN(Cur_गाँठो)->getPiece(getCN(Cur_गाँठो)->get_अङ्क_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    फर्काऊ;
  }

  // Find the next non-empty leaf node.
  गर
    Cur_गाँठो = getCN(Cur_गाँठो)->getNextLeafInOrder();
  जबसम्म (Cur_गाँठो && getCN(Cur_गाँठो)->get_अङ्क_Pieces() == 0);

  यदि (Cur_गाँठो)
    CurPiece = &getCN(Cur_गाँठो)->getPiece(0);
  अन्यथा // Hit end().
    CurPiece = nullptr;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

स्थिर Rope_Piece_BTree_गाँठो *get_जरा(शून्य *P) {
  फर्काऊ static_cast<Rope_Piece_BTree_गाँठो *>(P);
}

RopePieceBTree::RopePieceBTree() { जरा_2 = नूतन RopePieceBTreeLeaf(); }

RopePieceBTree::RopePieceBTree(अचर RopePieceBTree &RHS) {
  प्रतिज्ञा(RHS.empty() && "Can't copy non-empty tree yet");
  जरा_2 = नूतन RopePieceBTreeLeaf();
}

RopePieceBTree::~RopePieceBTree() { get_जरा(जरा_2)->Destroy(); }

अचिन्हित RopePieceBTree::size() अचर { फर्काऊ get_जरा(जरा_2)->size(); }

शून्य RopePieceBTree::clear() {
  यदि (स्वतः *Leaf = dyn_cast<RopePieceBTreeLeaf>(get_जरा(जरा_2)))
    Leaf->clear();
  अन्यथा {
    get_जरा(जरा_2)->Destroy();
    जरा_2 = नूतन RopePieceBTreeLeaf();
  }
}

शून्य RopePieceBTree::घुसाउ(अचिन्हित Offset, अचर RopePiece &R) {
  // #1. Split at Offset.
  यदि (Rope_Piece_BTree_गाँठो *RHS = get_जरा(जरा_2)->split(Offset))
    जरा_2 = नूतन RopePieceBTreeInterior(get_जरा(जरा_2), RHS);

  // #2. Do the insertion.
  यदि (Rope_Piece_BTree_गाँठो *RHS = get_जरा(जरा_2)->घुसाउ(Offset, R))
    जरा_2 = नूतन RopePieceBTreeInterior(get_जरा(जरा_2), RHS);
}

शून्य RopePieceBTree::erase(अचिन्हित Offset, अचिन्हित अङ्क_Bytes) {
  // #1. Split at Offset.
  यदि (Rope_Piece_BTree_गाँठो *RHS = get_जरा(जरा_2)->split(Offset))
    जरा_2 = नूतन RopePieceBTreeInterior(get_जरा(जरा_2), RHS);

  // #2. Do the erasing.
  get_जरा(जरा_2)->erase(Offset, अङ्क_Bytes);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(अचर वर्ण *Start, अचर वर्ण *End) {
  अचिन्हित Len = End - Start;
  प्रतिज्ञा(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  यदि (AllocOffs + Len <= AllocChunkSize) {
    स_नकल(AllocBuffer->Data + AllocOffs, Start, Len);
    AllocOffs += Len;
    फर्काऊ RopePiece(AllocBuffer, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  यदि (Len > AllocChunkSize) {
    अचिन्हित Size = End - Start + परिमाण(Rope_Ref_गन्ती_String) - 1;
    स्वतः *Res = reinterpret_cast<Rope_Ref_गन्ती_String *>(नूतन वर्ण[Size]);
    Res->Ref_गन्ती = 0;
    स_नकल(Res->Data, Start, End - Start);
    फर्काऊ RopePiece(Res, 0, End - Start);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  अचिन्हित AllocSize = दुरत्व(Rope_Ref_गन्ती_String, Data) + AllocChunkSize;
  स्वतः *Res = reinterpret_cast<Rope_Ref_गन्ती_String *>(नूतन वर्ण[AllocSize]);
  Res->Ref_गन्ती = 0;
  स_नकल(Res->Data, Start, Len);
  AllocBuffer = Res;
  AllocOffs = Len;

  फर्काऊ RopePiece(AllocBuffer, 0, Len);
}
