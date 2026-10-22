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

#அந்தர்பாவ "llvm/ADT/RewriteRope.h"
#அந்தர்பாவ "llvm/Support/Casting.h"
#அந்தர்பாவ <algorithm>
#அந்தர்பாவ <cassert>
#அந்தர்பாவ <cstring>

using நாமாகாஶ llvm;

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

நாமாகாஶ {

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Class
//===----------------------------------------------------------------------===//

/// RopePieceBTreeNode - Common base class of RopePieceBTreeLeaf and
/// RopePieceBTreeInterior.  This provides some 'virtual' dispatching methods
/// and a flag that determines which subclass the instance is.  Also
/// important, this node knows the full extend of the node, including any
/// children that it has.  This allows efficient skipping over entire subtrees
/// when looking for an offset in the BTree.
வகுப்பு Rope_Piece_BTree_முனை {
பாதுகாக்கப்பட்ட:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  எண்ணிக்கை { WidthFactor = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  unsigned Size = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  தருக்க IsLeaf;

  Rope_Piece_BTree_முனை(தருக்க isLeaf) : IsLeaf(isLeaf) {}
  ~Rope_Piece_BTree_முனை() = இயல்பு;

பொது:
  தருக்க isLeaf() மாறிலி { திரும்பு IsLeaf; }
  unsigned size() மாறிலி { திரும்பு Size; }

  வெறுமை Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_முனை *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_முனை *செருகு(unsigned Offset, மாறிலி RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  வெறுமை erase(unsigned Offset, unsigned இலக்கம்_Bytes);
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
வகுப்பு RopePieceBTreeLeaf : பொது Rope_Piece_BTree_முனை {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  unsigned எழுத்து இலக்கம்_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * WidthFactor];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  RopePieceBTreeLeaf **PrevLeaf = nullptr;
  RopePieceBTreeLeaf *NextLeaf = nullptr;

பொது:
  RopePieceBTreeLeaf() : Rope_Piece_BTree_முனை(உண்மை) {}

  ~RopePieceBTreeLeaf() {
    எனில் (PrevLeaf || NextLeaf)
      removeFromLeafInOrder();
    clear();
  }

  தருக்க isFull() மாறிலி { திரும்பு இலக்கம்_Pieces == 2 * WidthFactor; }

  /// clear - Remove all rope pieces from this leaf.
  வெறுமை clear() {
    போது (இலக்கம்_Pieces)
      Pieces[--இலக்கம்_Pieces] = RopePiece();
    Size = 0;
  }

  unsigned get_இலக்கம்_Pieces() மாறிலி { திரும்பு இலக்கம்_Pieces; }

  மாறிலி RopePiece &getPiece(unsigned i) மாறிலி {
    ப்ரதிஜ்ஞா(i < get_இலக்கம்_Pieces() && "Invalid piece ID");
    திரும்பு Pieces[i];
  }

  மாறிலி RopePieceBTreeLeaf *getNextLeafInOrder() மாறிலி { திரும்பு NextLeaf; }

  வெறுமை செருகு_After_Leaf_In_Order(RopePieceBTreeLeaf *முனை) {
    ப்ரதிஜ்ஞா(!PrevLeaf && !NextLeaf && "Already in ordering");

    NextLeaf = முனை->NextLeaf;
    எனில் (NextLeaf)
      NextLeaf->PrevLeaf = &NextLeaf;
    PrevLeaf = &முனை->NextLeaf;
    முனை->NextLeaf = இது;
  }

  வெறுமை removeFromLeafInOrder() {
    எனில் (PrevLeaf) {
      *PrevLeaf = NextLeaf;
      எனில் (NextLeaf)
        NextLeaf->PrevLeaf = PrevLeaf;
    } இல்லையெனில் எனில் (NextLeaf) {
      NextLeaf->PrevLeaf = nullptr;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  வெறுமை FullRecomputeSizeLocally() {
    Size = 0;
    க்கு (unsigned i = 0, e = get_இலக்கம்_Pieces(); i != e; ++i)
      Size += getPiece(i).size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_முனை *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_முனை *செருகு(unsigned Offset, மாறிலி RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  வெறுமை erase(unsigned Offset, unsigned இலக்கம்_Bytes);

  நிலையான தருக்க classof(மாறிலி Rope_Piece_BTree_முனை *N) { திரும்பு N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_முனை *RopePieceBTreeLeaf::split(unsigned Offset) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  எனில் (Offset == 0 || Offset == size()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    திரும்பு nullptr;
  }

  // Find the piece that this offset lands in.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  போது (Offset >= PieceOffs + Pieces[i].size()) {
    PieceOffs += Pieces[i].size();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  எனில் (PieceOffs == Offset)
    திரும்பு nullptr;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  unsigned IntraPieceOffset = Offset - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece Tail(Pieces[i].StrData, Pieces[i].StartOffs + IntraPieceOffset,
                 Pieces[i].EndOffs);
  Size -= Pieces[i].size();
  Pieces[i].EndOffs = Pieces[i].StartOffs + IntraPieceOffset;
  Size += Pieces[i].size();

  திரும்பு செருகு(Offset, Tail);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_முனை *RopePieceBTreeLeaf::செருகு(unsigned Offset,
                                               மாறிலி RopePiece &R) {
  // If this node is not full, insert the piece.
  எனில் (!isFull()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    unsigned i = 0, e = get_இலக்கம்_Pieces();
    எனில் (Offset == size()) {
      // Fastpath for a common case.
      i = e;
    } இல்லையெனில் {
      unsigned SlotOffs = 0;
      க்கு (; Offset > SlotOffs; ++i)
        SlotOffs += getPiece(i).size();
      ப்ரதிஜ்ஞா(SlotOffs == Offset && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    க்கு (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++இலக்கம்_Pieces;
    Size += R.size();
    திரும்பு nullptr;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  RopePieceBTreeLeaf *New_முனை = புதிய RopePieceBTreeLeaf();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[WidthFactor], &Pieces[2 * WidthFactor],
            &New_முனை->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[WidthFactor], &Pieces[2 * WidthFactor], RopePiece());

  // Decrease the number of values in the two nodes.
  New_முனை->இலக்கம்_Pieces = இலக்கம்_Pieces = WidthFactor;

  // Recompute the two nodes' size.
  New_முனை->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();

  // Update the list of leaves.
  New_முனை->செருகு_After_Leaf_In_Order(இது);

  // These insertions can't fail.
  எனில் (இது->size() >= Offset)
    இது->செருகு(Offset, R);
  இல்லையெனில்
    New_முனை->செருகு(Offset - இது->size(), R);
  திரும்பு New_முனை;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
வெறுமை RopePieceBTreeLeaf::erase(unsigned Offset, unsigned இலக்கம்_Bytes) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  க்கு (; Offset > PieceOffs; ++i)
    PieceOffs += getPiece(i).size();
  ப்ரதிஜ்ஞா(PieceOffs == Offset && "Split didn't occur before erase!");

  unsigned StartPiece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  க்கு (; Offset + இலக்கம்_Bytes > PieceOffs + getPiece(i).size(); ++i)
    PieceOffs += getPiece(i).size();

  // If we exactly include the last one, include it in the region to delete.
  எனில் (Offset + இலக்கம்_Bytes == PieceOffs + getPiece(i).size()) {
    PieceOffs += getPiece(i).size();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  எனில் (i != StartPiece) {
    unsigned இலக்கம்_Deleted = i - StartPiece;
    க்கு (; i != get_இலக்கம்_Pieces(); ++i)
      Pieces[i - இலக்கம்_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_இலக்கம்_Pieces() - இலக்கம்_Deleted], &Pieces[get_இலக்கம்_Pieces()],
              RopePiece());
    இலக்கம்_Pieces -= இலக்கம்_Deleted;

    unsigned CoverBytes = PieceOffs - Offset;
    இலக்கம்_Bytes -= CoverBytes;
    Size -= CoverBytes;
  }

  // If we completely removed some stuff, we could be done.
  எனில் (இலக்கம்_Bytes == 0)
    திரும்பு;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  ப்ரதிஜ்ஞா(getPiece(StartPiece).size() > இலக்கம்_Bytes);
  Pieces[StartPiece].StartOffs += இலக்கம்_Bytes;

  // The size of this node just shrunk by NumBytes.
  Size -= இலக்கம்_Bytes;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

நாமாகாஶ {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
வகுப்பு RopePieceBTreeInterior : பொது Rope_Piece_BTree_முனை {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  unsigned எழுத்து இலக்கம்_Children = 0;

  Rope_Piece_BTree_முனை *Children[2 * WidthFactor];

பொது:
  RopePieceBTreeInterior() : Rope_Piece_BTree_முனை(பொய்) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_முனை *LHS, Rope_Piece_BTree_முனை *RHS)
      : Rope_Piece_BTree_முனை(பொய்) {
    Children[0] = LHS;
    Children[1] = RHS;
    இலக்கம்_Children = 2;
    Size = LHS->size() + RHS->size();
  }

  ~RopePieceBTreeInterior() {
    க்கு (unsigned i = 0, e = get_இலக்கம்_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  தருக்க isFull() மாறிலி { திரும்பு இலக்கம்_Children == 2 * WidthFactor; }

  unsigned get_இலக்கம்_Children() மாறிலி { திரும்பு இலக்கம்_Children; }

  மாறிலி Rope_Piece_BTree_முனை *getChild(unsigned i) மாறிலி {
    ப்ரதிஜ்ஞா(i < இலக்கம்_Children && "invalid child #");
    திரும்பு Children[i];
  }

  Rope_Piece_BTree_முனை *getChild(unsigned i) {
    ப்ரதிஜ்ஞா(i < இலக்கம்_Children && "invalid child #");
    திரும்பு Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  வெறுமை FullRecomputeSizeLocally() {
    Size = 0;
    க்கு (unsigned i = 0, e = get_இலக்கம்_Children(); i != e; ++i)
      Size += getChild(i)->size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_முனை *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_முனை *செருகு(unsigned Offset, மாறிலி RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_முனை *HandleChildPiece(unsigned i, Rope_Piece_BTree_முனை *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  வெறுமை erase(unsigned Offset, unsigned இலக்கம்_Bytes);

  நிலையான தருக்க classof(மாறிலி Rope_Piece_BTree_முனை *N) { திரும்பு !N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_முனை *RopePieceBTreeInterior::split(unsigned Offset) {
  // Figure out which child to split.
  எனில் (Offset == 0 || Offset == size())
    திரும்பு nullptr; // If we have an exact offset, we're already split.

  unsigned ChildOffset = 0;
  unsigned i = 0;
  க்கு (; Offset >= ChildOffset + getChild(i)->size(); ++i)
    ChildOffset += getChild(i)->size();

  // If already split there, we're done.
  எனில் (ChildOffset == Offset)
    திரும்பு nullptr;

  // Otherwise, recursively split the child.
  எனில் (Rope_Piece_BTree_முனை *RHS = getChild(i)->split(Offset - ChildOffset))
    திரும்பு HandleChildPiece(i, RHS);
  திரும்பு nullptr; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_முனை *RopePieceBTreeInterior::செருகு(unsigned Offset,
                                                   மாறிலி RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  unsigned i = 0, e = get_இலக்கம்_Children();

  unsigned ChildOffs = 0;
  எனில் (Offset == size()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    ChildOffs = size() - getChild(i)->size();
  } இல்லையெனில் {
    க்கு (; Offset > ChildOffs + getChild(i)->size(); ++i)
      ChildOffs += getChild(i)->size();
  }

  Size += R.size();

  // Insert at the end of this child.
  எனில் (Rope_Piece_BTree_முனை *RHS = getChild(i)->செருகு(Offset - ChildOffs, R))
    திரும்பு HandleChildPiece(i, RHS);

  திரும்பு nullptr;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_முனை *
RopePieceBTreeInterior::HandleChildPiece(unsigned i, Rope_Piece_BTree_முனை *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  எனில் (!isFull()) {
    // Insert RHS after child 'i'.
    எனில் (i + 1 != get_இலக்கம்_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_இலக்கம்_Children() - i - 1) * அளவு(Children[0]));
    Children[i + 1] = RHS;
    ++இலக்கம்_Children;
    திரும்பு nullptr;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_முனை = புதிய RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  memcpy(&New_முனை->Children[0], &Children[WidthFactor],
         WidthFactor * அளவு(Children[0]));

  // Decrease the number of values in the two nodes.
  New_முனை->இலக்கம்_Children = இலக்கம்_Children = WidthFactor;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  எனில் (i < WidthFactor)
    இது->HandleChildPiece(i, RHS);
  இல்லையெனில்
    New_முனை->HandleChildPiece(i - WidthFactor, RHS);

  // Recompute the two nodes' size.
  New_முனை->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();
  திரும்பு New_முனை;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
வெறுமை RopePieceBTreeInterior::erase(unsigned Offset, unsigned இலக்கம்_Bytes) {
  // This will shrink this node by NumBytes.
  Size -= இலக்கம்_Bytes;

  // Find the first child that overlaps with Offset.
  unsigned i = 0;
  க்கு (; Offset >= getChild(i)->size(); ++i)
    Offset -= getChild(i)->size();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  போது (இலக்கம்_Bytes) {
    Rope_Piece_BTree_முனை *CurChild = getChild(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    எனில் (Offset + இலக்கம்_Bytes < CurChild->size()) {
      CurChild->erase(Offset, இலக்கம்_Bytes);
      திரும்பு;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    எனில் (Offset) {
      unsigned BytesFromChild = CurChild->size() - Offset;
      CurChild->erase(Offset, BytesFromChild);
      இலக்கம்_Bytes -= BytesFromChild;
      // Start at the beginning of the next child.
      Offset = 0;
      ++i;
      தொடர்;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    இலக்கம்_Bytes -= CurChild->size();
    CurChild->Destroy();
    --இலக்கம்_Children;
    எனில் (i != get_இலக்கம்_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_இலக்கம்_Children() - i) * அளவு(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

வெறுமை Rope_Piece_BTree_முனை::Destroy() {
  எனில் (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(இது))
    delete Leaf;
  இல்லையெனில்
    delete cast<RopePieceBTreeInterior>(இது);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_முனை *Rope_Piece_BTree_முனை::split(unsigned Offset) {
  ப்ரதிஜ்ஞா(Offset <= size() && "Invalid offset to split!");
  எனில் (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(இது))
    திரும்பு Leaf->split(Offset);
  திரும்பு cast<RopePieceBTreeInterior>(இது)->split(Offset);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_முனை *Rope_Piece_BTree_முனை::செருகு(unsigned Offset,
                                               மாறிலி RopePiece &R) {
  ப்ரதிஜ்ஞா(Offset <= size() && "Invalid offset to insert!");
  எனில் (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(இது))
    திரும்பு Leaf->செருகு(Offset, R);
  திரும்பு cast<RopePieceBTreeInterior>(இது)->செருகு(Offset, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
வெறுமை Rope_Piece_BTree_முனை::erase(unsigned Offset, unsigned இலக்கம்_Bytes) {
  ப்ரதிஜ்ஞா(Offset + இலக்கம்_Bytes <= size() && "Invalid offset to erase!");
  எனில் (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(இது))
    திரும்பு Leaf->erase(Offset, இலக்கம்_Bytes);
  திரும்பு cast<RopePieceBTreeInterior>(இது)->erase(Offset, இலக்கம்_Bytes);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

நிலையான மாறிலி RopePieceBTreeLeaf *getCN(மாறிலி வெறுமை *P) {
  திரும்பு static_cast<மாறிலி RopePieceBTreeLeaf *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(மாறிலி வெறுமை *n) {
  மாறிலி auto *N = static_cast<மாறிலி Rope_Piece_BTree_முனை *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  போது (மாறிலி auto *IN = dyn_cast<RopePieceBTreeInterior>(N))
    N = IN->getChild(0);

  // We must have at least one leaf.
  Cur_முனை = cast<RopePieceBTreeLeaf>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  போது (Cur_முனை && getCN(Cur_முனை)->get_இலக்கம்_Pieces() == 0)
    Cur_முனை = getCN(Cur_முனை)->getNextLeafInOrder();

  எனில் (Cur_முனை)
    CurPiece = &getCN(Cur_முனை)->getPiece(0);
  இல்லையெனில் // Empty tree, this is an end() iterator.
    CurPiece = nullptr;
  CurChar = 0;
}

வெறுமை RopePieceBTreeIterator::MoveToNextPiece() {
  எனில் (CurPiece !=
      &getCN(Cur_முனை)->getPiece(getCN(Cur_முனை)->get_இலக்கம்_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    திரும்பு;
  }

  // Find the next non-empty leaf node.
  செய்
    Cur_முனை = getCN(Cur_முனை)->getNextLeafInOrder();
  போது (Cur_முனை && getCN(Cur_முனை)->get_இலக்கம்_Pieces() == 0);

  எனில் (Cur_முனை)
    CurPiece = &getCN(Cur_முனை)->getPiece(0);
  இல்லையெனில் // Hit end().
    CurPiece = nullptr;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

நிலையான Rope_Piece_BTree_முனை *get_வேர்(வெறுமை *P) {
  திரும்பு static_cast<Rope_Piece_BTree_முனை *>(P);
}

RopePieceBTree::RopePieceBTree() { வேர்_2 = புதிய RopePieceBTreeLeaf(); }

RopePieceBTree::RopePieceBTree(மாறிலி RopePieceBTree &RHS) {
  ப்ரதிஜ்ஞா(RHS.empty() && "Can't copy non-empty tree yet");
  வேர்_2 = புதிய RopePieceBTreeLeaf();
}

RopePieceBTree::~RopePieceBTree() { get_வேர்(வேர்_2)->Destroy(); }

unsigned RopePieceBTree::size() மாறிலி { திரும்பு get_வேர்(வேர்_2)->size(); }

வெறுமை RopePieceBTree::clear() {
  எனில் (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(get_வேர்(வேர்_2)))
    Leaf->clear();
  இல்லையெனில் {
    get_வேர்(வேர்_2)->Destroy();
    வேர்_2 = புதிய RopePieceBTreeLeaf();
  }
}

வெறுமை RopePieceBTree::செருகு(unsigned Offset, மாறிலி RopePiece &R) {
  // #1. Split at Offset.
  எனில் (Rope_Piece_BTree_முனை *RHS = get_வேர்(வேர்_2)->split(Offset))
    வேர்_2 = புதிய RopePieceBTreeInterior(get_வேர்(வேர்_2), RHS);

  // #2. Do the insertion.
  எனில் (Rope_Piece_BTree_முனை *RHS = get_வேர்(வேர்_2)->செருகு(Offset, R))
    வேர்_2 = புதிய RopePieceBTreeInterior(get_வேர்(வேர்_2), RHS);
}

வெறுமை RopePieceBTree::erase(unsigned Offset, unsigned இலக்கம்_Bytes) {
  // #1. Split at Offset.
  எனில் (Rope_Piece_BTree_முனை *RHS = get_வேர்(வேர்_2)->split(Offset))
    வேர்_2 = புதிய RopePieceBTreeInterior(get_வேர்(வேர்_2), RHS);

  // #2. Do the erasing.
  get_வேர்(வேர்_2)->erase(Offset, இலக்கம்_Bytes);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(மாறிலி எழுத்து *Start, மாறிலி எழுத்து *End) {
  unsigned Len = End - Start;
  ப்ரதிஜ்ஞா(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  எனில் (AllocOffs + Len <= AllocChunkSize) {
    memcpy(AllocBuffer->Data + AllocOffs, Start, Len);
    AllocOffs += Len;
    திரும்பு RopePiece(AllocBuffer, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  எனில் (Len > AllocChunkSize) {
    unsigned Size = End - Start + அளவு(Rope_Ref_மொத்தம்_String) - 1;
    auto *Res = reinterpret_cast<Rope_Ref_மொத்தம்_String *>(புதிய எழுத்து[Size]);
    Res->Ref_மொத்தம் = 0;
    memcpy(Res->Data, Start, End - Start);
    திரும்பு RopePiece(Res, 0, End - Start);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  unsigned AllocSize = offsetof(Rope_Ref_மொத்தம்_String, Data) + AllocChunkSize;
  auto *Res = reinterpret_cast<Rope_Ref_மொத்தம்_String *>(புதிய எழுத்து[AllocSize]);
  Res->Ref_மொத்தம் = 0;
  memcpy(Res->Data, Start, Len);
  AllocBuffer = Res;
  AllocOffs = Len;

  திரும்பு RopePiece(AllocBuffer, 0, Len);
}
