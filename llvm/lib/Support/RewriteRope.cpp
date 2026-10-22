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

#અન્તર્ભાવ "llvm/ADT/RewriteRope.h"
#અન્તર્ભાવ "llvm/Support/Casting.h"
#અન્તર્ભાવ <algorithm>
#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <cstring>

using નામાકાશ llvm;

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

નામાકાશ {

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Class
//===----------------------------------------------------------------------===//

/// RopePieceBTreeNode - Common base class of RopePieceBTreeLeaf and
/// RopePieceBTreeInterior.  This provides some 'virtual' dispatching methods
/// and a flag that determines which subclass the instance is.  Also
/// important, this node knows the full extend of the node, including any
/// children that it has.  This allows efficient skipping over entire subtrees
/// when looking for an offset in the BTree.
વર્ગ Rope_Piece_BTree_ગાંઠ {
સુરક્ષિત:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  ગણના { WidthFactor = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  unsigned Size = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  તાર્કિક IsLeaf;

  Rope_Piece_BTree_ગાંઠ(તાર્કિક isLeaf) : IsLeaf(isLeaf) {}
  ~Rope_Piece_BTree_ગાંઠ() = મૂળભૂત;

જાહેર:
  તાર્કિક isLeaf() અચલ { પાછા IsLeaf; }
  unsigned size() અચલ { પાછા Size; }

  શૂન્ય Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ગાંઠ *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ગાંઠ *ઉમેરો(unsigned Offset, અચલ RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  શૂન્ય erase(unsigned Offset, unsigned અંક_Bytes);
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
વર્ગ RopePieceBTreeLeaf : જાહેર Rope_Piece_BTree_ગાંઠ {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  unsigned અક્ષર અંક_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * WidthFactor];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  RopePieceBTreeLeaf **PrevLeaf = nullptr;
  RopePieceBTreeLeaf *NextLeaf = nullptr;

જાહેર:
  RopePieceBTreeLeaf() : Rope_Piece_BTree_ગાંઠ(સાચું) {}

  ~RopePieceBTreeLeaf() {
    જો (PrevLeaf || NextLeaf)
      removeFromLeafInOrder();
    clear();
  }

  તાર્કિક isFull() અચલ { પાછા અંક_Pieces == 2 * WidthFactor; }

  /// clear - Remove all rope pieces from this leaf.
  શૂન્ય clear() {
    જ્યારે (અંક_Pieces)
      Pieces[--અંક_Pieces] = RopePiece();
    Size = 0;
  }

  unsigned get_અંક_Pieces() અચલ { પાછા અંક_Pieces; }

  અચલ RopePiece &getPiece(unsigned i) અચલ {
    પ્રતિજ્ઞા(i < get_અંક_Pieces() && "Invalid piece ID");
    પાછા Pieces[i];
  }

  અચલ RopePieceBTreeLeaf *getNextLeafInOrder() અચલ { પાછા NextLeaf; }

  શૂન્ય ઉમેરો_After_Leaf_In_Order(RopePieceBTreeLeaf *ગાંઠ) {
    પ્રતિજ્ઞા(!PrevLeaf && !NextLeaf && "Already in ordering");

    NextLeaf = ગાંઠ->NextLeaf;
    જો (NextLeaf)
      NextLeaf->PrevLeaf = &NextLeaf;
    PrevLeaf = &ગાંઠ->NextLeaf;
    ગાંઠ->NextLeaf = આ;
  }

  શૂન્ય removeFromLeafInOrder() {
    જો (PrevLeaf) {
      *PrevLeaf = NextLeaf;
      જો (NextLeaf)
        NextLeaf->PrevLeaf = PrevLeaf;
    } નહીંતર જો (NextLeaf) {
      NextLeaf->PrevLeaf = nullptr;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  શૂન્ય FullRecomputeSizeLocally() {
    Size = 0;
    માટે (unsigned i = 0, e = get_અંક_Pieces(); i != e; ++i)
      Size += getPiece(i).size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ગાંઠ *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ગાંઠ *ઉમેરો(unsigned Offset, અચલ RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  શૂન્ય erase(unsigned Offset, unsigned અંક_Bytes);

  સ્થિર તાર્કિક classof(અચલ Rope_Piece_BTree_ગાંઠ *N) { પાછા N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ગાંઠ *RopePieceBTreeLeaf::split(unsigned Offset) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  જો (Offset == 0 || Offset == size()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    પાછા nullptr;
  }

  // Find the piece that this offset lands in.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  જ્યારે (Offset >= PieceOffs + Pieces[i].size()) {
    PieceOffs += Pieces[i].size();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  જો (PieceOffs == Offset)
    પાછા nullptr;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  unsigned IntraPieceOffset = Offset - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece Tail(Pieces[i].StrData, Pieces[i].StartOffs + IntraPieceOffset,
                 Pieces[i].EndOffs);
  Size -= Pieces[i].size();
  Pieces[i].EndOffs = Pieces[i].StartOffs + IntraPieceOffset;
  Size += Pieces[i].size();

  પાછા ઉમેરો(Offset, Tail);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ગાંઠ *RopePieceBTreeLeaf::ઉમેરો(unsigned Offset,
                                               અચલ RopePiece &R) {
  // If this node is not full, insert the piece.
  જો (!isFull()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    unsigned i = 0, e = get_અંક_Pieces();
    જો (Offset == size()) {
      // Fastpath for a common case.
      i = e;
    } નહીંતર {
      unsigned SlotOffs = 0;
      માટે (; Offset > SlotOffs; ++i)
        SlotOffs += getPiece(i).size();
      પ્રતિજ્ઞા(SlotOffs == Offset && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    માટે (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++અંક_Pieces;
    Size += R.size();
    પાછા nullptr;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  RopePieceBTreeLeaf *New_ગાંઠ = નવું RopePieceBTreeLeaf();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[WidthFactor], &Pieces[2 * WidthFactor],
            &New_ગાંઠ->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[WidthFactor], &Pieces[2 * WidthFactor], RopePiece());

  // Decrease the number of values in the two nodes.
  New_ગાંઠ->અંક_Pieces = અંક_Pieces = WidthFactor;

  // Recompute the two nodes' size.
  New_ગાંઠ->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();

  // Update the list of leaves.
  New_ગાંઠ->ઉમેરો_After_Leaf_In_Order(આ);

  // These insertions can't fail.
  જો (આ->size() >= Offset)
    આ->ઉમેરો(Offset, R);
  નહીંતર
    New_ગાંઠ->ઉમેરો(Offset - આ->size(), R);
  પાછા New_ગાંઠ;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
શૂન્ય RopePieceBTreeLeaf::erase(unsigned Offset, unsigned અંક_Bytes) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  માટે (; Offset > PieceOffs; ++i)
    PieceOffs += getPiece(i).size();
  પ્રતિજ્ઞા(PieceOffs == Offset && "Split didn't occur before erase!");

  unsigned StartPiece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  માટે (; Offset + અંક_Bytes > PieceOffs + getPiece(i).size(); ++i)
    PieceOffs += getPiece(i).size();

  // If we exactly include the last one, include it in the region to delete.
  જો (Offset + અંક_Bytes == PieceOffs + getPiece(i).size()) {
    PieceOffs += getPiece(i).size();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  જો (i != StartPiece) {
    unsigned અંક_Deleted = i - StartPiece;
    માટે (; i != get_અંક_Pieces(); ++i)
      Pieces[i - અંક_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_અંક_Pieces() - અંક_Deleted], &Pieces[get_અંક_Pieces()],
              RopePiece());
    અંક_Pieces -= અંક_Deleted;

    unsigned CoverBytes = PieceOffs - Offset;
    અંક_Bytes -= CoverBytes;
    Size -= CoverBytes;
  }

  // If we completely removed some stuff, we could be done.
  જો (અંક_Bytes == 0)
    પાછા;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  પ્રતિજ્ઞા(getPiece(StartPiece).size() > અંક_Bytes);
  Pieces[StartPiece].StartOffs += અંક_Bytes;

  // The size of this node just shrunk by NumBytes.
  Size -= અંક_Bytes;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

નામાકાશ {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
વર્ગ RopePieceBTreeInterior : જાહેર Rope_Piece_BTree_ગાંઠ {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  unsigned અક્ષર અંક_Children = 0;

  Rope_Piece_BTree_ગાંઠ *Children[2 * WidthFactor];

જાહેર:
  RopePieceBTreeInterior() : Rope_Piece_BTree_ગાંઠ(ખોટું) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_ગાંઠ *LHS, Rope_Piece_BTree_ગાંઠ *RHS)
      : Rope_Piece_BTree_ગાંઠ(ખોટું) {
    Children[0] = LHS;
    Children[1] = RHS;
    અંક_Children = 2;
    Size = LHS->size() + RHS->size();
  }

  ~RopePieceBTreeInterior() {
    માટે (unsigned i = 0, e = get_અંક_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  તાર્કિક isFull() અચલ { પાછા અંક_Children == 2 * WidthFactor; }

  unsigned get_અંક_Children() અચલ { પાછા અંક_Children; }

  અચલ Rope_Piece_BTree_ગાંઠ *getChild(unsigned i) અચલ {
    પ્રતિજ્ઞા(i < અંક_Children && "invalid child #");
    પાછા Children[i];
  }

  Rope_Piece_BTree_ગાંઠ *getChild(unsigned i) {
    પ્રતિજ્ઞા(i < અંક_Children && "invalid child #");
    પાછા Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  શૂન્ય FullRecomputeSizeLocally() {
    Size = 0;
    માટે (unsigned i = 0, e = get_અંક_Children(); i != e; ++i)
      Size += getChild(i)->size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ગાંઠ *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ગાંઠ *ઉમેરો(unsigned Offset, અચલ RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_ગાંઠ *HandleChildPiece(unsigned i, Rope_Piece_BTree_ગાંઠ *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  શૂન્ય erase(unsigned Offset, unsigned અંક_Bytes);

  સ્થિર તાર્કિક classof(અચલ Rope_Piece_BTree_ગાંઠ *N) { પાછા !N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ગાંઠ *RopePieceBTreeInterior::split(unsigned Offset) {
  // Figure out which child to split.
  જો (Offset == 0 || Offset == size())
    પાછા nullptr; // If we have an exact offset, we're already split.

  unsigned ChildOffset = 0;
  unsigned i = 0;
  માટે (; Offset >= ChildOffset + getChild(i)->size(); ++i)
    ChildOffset += getChild(i)->size();

  // If already split there, we're done.
  જો (ChildOffset == Offset)
    પાછા nullptr;

  // Otherwise, recursively split the child.
  જો (Rope_Piece_BTree_ગાંઠ *RHS = getChild(i)->split(Offset - ChildOffset))
    પાછા HandleChildPiece(i, RHS);
  પાછા nullptr; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ગાંઠ *RopePieceBTreeInterior::ઉમેરો(unsigned Offset,
                                                   અચલ RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  unsigned i = 0, e = get_અંક_Children();

  unsigned ChildOffs = 0;
  જો (Offset == size()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    ChildOffs = size() - getChild(i)->size();
  } નહીંતર {
    માટે (; Offset > ChildOffs + getChild(i)->size(); ++i)
      ChildOffs += getChild(i)->size();
  }

  Size += R.size();

  // Insert at the end of this child.
  જો (Rope_Piece_BTree_ગાંઠ *RHS = getChild(i)->ઉમેરો(Offset - ChildOffs, R))
    પાછા HandleChildPiece(i, RHS);

  પાછા nullptr;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_ગાંઠ *
RopePieceBTreeInterior::HandleChildPiece(unsigned i, Rope_Piece_BTree_ગાંઠ *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  જો (!isFull()) {
    // Insert RHS after child 'i'.
    જો (i + 1 != get_અંક_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_અંક_Children() - i - 1) * કદ(Children[0]));
    Children[i + 1] = RHS;
    ++અંક_Children;
    પાછા nullptr;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_ગાંઠ = નવું RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  memcpy(&New_ગાંઠ->Children[0], &Children[WidthFactor],
         WidthFactor * કદ(Children[0]));

  // Decrease the number of values in the two nodes.
  New_ગાંઠ->અંક_Children = અંક_Children = WidthFactor;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  જો (i < WidthFactor)
    આ->HandleChildPiece(i, RHS);
  નહીંતર
    New_ગાંઠ->HandleChildPiece(i - WidthFactor, RHS);

  // Recompute the two nodes' size.
  New_ગાંઠ->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();
  પાછા New_ગાંઠ;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
શૂન્ય RopePieceBTreeInterior::erase(unsigned Offset, unsigned અંક_Bytes) {
  // This will shrink this node by NumBytes.
  Size -= અંક_Bytes;

  // Find the first child that overlaps with Offset.
  unsigned i = 0;
  માટે (; Offset >= getChild(i)->size(); ++i)
    Offset -= getChild(i)->size();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  જ્યારે (અંક_Bytes) {
    Rope_Piece_BTree_ગાંઠ *CurChild = getChild(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    જો (Offset + અંક_Bytes < CurChild->size()) {
      CurChild->erase(Offset, અંક_Bytes);
      પાછા;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    જો (Offset) {
      unsigned BytesFromChild = CurChild->size() - Offset;
      CurChild->erase(Offset, BytesFromChild);
      અંક_Bytes -= BytesFromChild;
      // Start at the beginning of the next child.
      Offset = 0;
      ++i;
      ચાલુ;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    અંક_Bytes -= CurChild->size();
    CurChild->Destroy();
    --અંક_Children;
    જો (i != get_અંક_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_અંક_Children() - i) * કદ(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

શૂન્ય Rope_Piece_BTree_ગાંઠ::Destroy() {
  જો (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(આ))
    delete Leaf;
  નહીંતર
    delete cast<RopePieceBTreeInterior>(આ);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ગાંઠ *Rope_Piece_BTree_ગાંઠ::split(unsigned Offset) {
  પ્રતિજ્ઞા(Offset <= size() && "Invalid offset to split!");
  જો (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(આ))
    પાછા Leaf->split(Offset);
  પાછા cast<RopePieceBTreeInterior>(આ)->split(Offset);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ગાંઠ *Rope_Piece_BTree_ગાંઠ::ઉમેરો(unsigned Offset,
                                               અચલ RopePiece &R) {
  પ્રતિજ્ઞા(Offset <= size() && "Invalid offset to insert!");
  જો (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(આ))
    પાછા Leaf->ઉમેરો(Offset, R);
  પાછા cast<RopePieceBTreeInterior>(આ)->ઉમેરો(Offset, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
શૂન્ય Rope_Piece_BTree_ગાંઠ::erase(unsigned Offset, unsigned અંક_Bytes) {
  પ્રતિજ્ઞા(Offset + અંક_Bytes <= size() && "Invalid offset to erase!");
  જો (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(આ))
    પાછા Leaf->erase(Offset, અંક_Bytes);
  પાછા cast<RopePieceBTreeInterior>(આ)->erase(Offset, અંક_Bytes);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

સ્થિર અચલ RopePieceBTreeLeaf *getCN(અચલ શૂન્ય *P) {
  પાછા static_cast<અચલ RopePieceBTreeLeaf *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(અચલ શૂન્ય *n) {
  અચલ auto *N = static_cast<અચલ Rope_Piece_BTree_ગાંઠ *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  જ્યારે (અચલ auto *IN = dyn_cast<RopePieceBTreeInterior>(N))
    N = IN->getChild(0);

  // We must have at least one leaf.
  Cur_ગાંઠ = cast<RopePieceBTreeLeaf>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  જ્યારે (Cur_ગાંઠ && getCN(Cur_ગાંઠ)->get_અંક_Pieces() == 0)
    Cur_ગાંઠ = getCN(Cur_ગાંઠ)->getNextLeafInOrder();

  જો (Cur_ગાંઠ)
    CurPiece = &getCN(Cur_ગાંઠ)->getPiece(0);
  નહીંતર // Empty tree, this is an end() iterator.
    CurPiece = nullptr;
  CurChar = 0;
}

શૂન્ય RopePieceBTreeIterator::MoveToNextPiece() {
  જો (CurPiece !=
      &getCN(Cur_ગાંઠ)->getPiece(getCN(Cur_ગાંઠ)->get_અંક_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    પાછા;
  }

  // Find the next non-empty leaf node.
  કરો
    Cur_ગાંઠ = getCN(Cur_ગાંઠ)->getNextLeafInOrder();
  જ્યારે (Cur_ગાંઠ && getCN(Cur_ગાંઠ)->get_અંક_Pieces() == 0);

  જો (Cur_ગાંઠ)
    CurPiece = &getCN(Cur_ગાંઠ)->getPiece(0);
  નહીંતર // Hit end().
    CurPiece = nullptr;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

સ્થિર Rope_Piece_BTree_ગાંઠ *get_મૂળ(શૂન્ય *P) {
  પાછા static_cast<Rope_Piece_BTree_ગાંઠ *>(P);
}

RopePieceBTree::RopePieceBTree() { મૂળ_2 = નવું RopePieceBTreeLeaf(); }

RopePieceBTree::RopePieceBTree(અચલ RopePieceBTree &RHS) {
  પ્રતિજ્ઞા(RHS.empty() && "Can't copy non-empty tree yet");
  મૂળ_2 = નવું RopePieceBTreeLeaf();
}

RopePieceBTree::~RopePieceBTree() { get_મૂળ(મૂળ_2)->Destroy(); }

unsigned RopePieceBTree::size() અચલ { પાછા get_મૂળ(મૂળ_2)->size(); }

શૂન્ય RopePieceBTree::clear() {
  જો (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(get_મૂળ(મૂળ_2)))
    Leaf->clear();
  નહીંતર {
    get_મૂળ(મૂળ_2)->Destroy();
    મૂળ_2 = નવું RopePieceBTreeLeaf();
  }
}

શૂન્ય RopePieceBTree::ઉમેરો(unsigned Offset, અચલ RopePiece &R) {
  // #1. Split at Offset.
  જો (Rope_Piece_BTree_ગાંઠ *RHS = get_મૂળ(મૂળ_2)->split(Offset))
    મૂળ_2 = નવું RopePieceBTreeInterior(get_મૂળ(મૂળ_2), RHS);

  // #2. Do the insertion.
  જો (Rope_Piece_BTree_ગાંઠ *RHS = get_મૂળ(મૂળ_2)->ઉમેરો(Offset, R))
    મૂળ_2 = નવું RopePieceBTreeInterior(get_મૂળ(મૂળ_2), RHS);
}

શૂન્ય RopePieceBTree::erase(unsigned Offset, unsigned અંક_Bytes) {
  // #1. Split at Offset.
  જો (Rope_Piece_BTree_ગાંઠ *RHS = get_મૂળ(મૂળ_2)->split(Offset))
    મૂળ_2 = નવું RopePieceBTreeInterior(get_મૂળ(મૂળ_2), RHS);

  // #2. Do the erasing.
  get_મૂળ(મૂળ_2)->erase(Offset, અંક_Bytes);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(અચલ અક્ષર *Start, અચલ અક્ષર *End) {
  unsigned Len = End - Start;
  પ્રતિજ્ઞા(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  જો (AllocOffs + Len <= AllocChunkSize) {
    memcpy(AllocBuffer->Data + AllocOffs, Start, Len);
    AllocOffs += Len;
    પાછા RopePiece(AllocBuffer, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  જો (Len > AllocChunkSize) {
    unsigned Size = End - Start + કદ(Rope_Ref_ગણતરી_String) - 1;
    auto *Res = reinterpret_cast<Rope_Ref_ગણતરી_String *>(નવું અક્ષર[Size]);
    Res->Ref_ગણતરી = 0;
    memcpy(Res->Data, Start, End - Start);
    પાછા RopePiece(Res, 0, End - Start);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  unsigned AllocSize = offsetof(Rope_Ref_ગણતરી_String, Data) + AllocChunkSize;
  auto *Res = reinterpret_cast<Rope_Ref_ગણતરી_String *>(નવું અક્ષર[AllocSize]);
  Res->Ref_ગણતરી = 0;
  memcpy(Res->Data, Start, Len);
  AllocBuffer = Res;
  AllocOffs = Len;

  પાછા RopePiece(AllocBuffer, 0, Len);
}
