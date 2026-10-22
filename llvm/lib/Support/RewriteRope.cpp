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

#അന്തര്ഭാവ "llvm/ADT/RewriteRope.h"
#അന്തര്ഭാവ "llvm/Support/Casting.h"
#അന്തര്ഭാവ <algorithm>
#അന്തര്ഭാവ <cassert>
#അന്തര്ഭാവ <cstring>

using നാമാകാശ llvm;

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

നാമാകാശ {

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Class
//===----------------------------------------------------------------------===//

/// RopePieceBTreeNode - Common base class of RopePieceBTreeLeaf and
/// RopePieceBTreeInterior.  This provides some 'virtual' dispatching methods
/// and a flag that determines which subclass the instance is.  Also
/// important, this node knows the full extend of the node, including any
/// children that it has.  This allows efficient skipping over entire subtrees
/// when looking for an offset in the BTree.
വർഗ്ഗം Rope_Piece_BTree_കണ്ണി {
സംരക്ഷിത:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  എണ്ണൽ { WidthFactor = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  unsigned Size = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  തർക്കം IsLeaf;

  Rope_Piece_BTree_കണ്ണി(തർക്കം isLeaf) : IsLeaf(isLeaf) {}
  ~Rope_Piece_BTree_കണ്ണി() = സ്ഥിരസ്ഥിതി;

പൊതു:
  തർക്കം isLeaf() സ്ഥിരം { തിരികെ IsLeaf; }
  unsigned size() സ്ഥിരം { തിരികെ Size; }

  ശൂന്യം Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_കണ്ണി *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_കണ്ണി *തിരുകുക(unsigned Offset, സ്ഥിരം RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  ശൂന്യം erase(unsigned Offset, unsigned അക്കം_Bytes);
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
വർഗ്ഗം RopePieceBTreeLeaf : പൊതു Rope_Piece_BTree_കണ്ണി {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  unsigned അക്ഷരം അക്കം_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * WidthFactor];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  RopePieceBTreeLeaf **PrevLeaf = nullptr;
  RopePieceBTreeLeaf *NextLeaf = nullptr;

പൊതു:
  RopePieceBTreeLeaf() : Rope_Piece_BTree_കണ്ണി(സത്യം) {}

  ~RopePieceBTreeLeaf() {
    എങ്കിൽ (PrevLeaf || NextLeaf)
      removeFromLeafInOrder();
    clear();
  }

  തർക്കം isFull() സ്ഥിരം { തിരികെ അക്കം_Pieces == 2 * WidthFactor; }

  /// clear - Remove all rope pieces from this leaf.
  ശൂന്യം clear() {
    ആകുമ്പോൾ (അക്കം_Pieces)
      Pieces[--അക്കം_Pieces] = RopePiece();
    Size = 0;
  }

  unsigned get_അക്കം_Pieces() സ്ഥിരം { തിരികെ അക്കം_Pieces; }

  സ്ഥിരം RopePiece &getPiece(unsigned i) സ്ഥിരം {
    പ്രതിജ്ഞാ(i < get_അക്കം_Pieces() && "Invalid piece ID");
    തിരികെ Pieces[i];
  }

  സ്ഥിരം RopePieceBTreeLeaf *getNextLeafInOrder() സ്ഥിരം { തിരികെ NextLeaf; }

  ശൂന്യം തിരുകുക_After_Leaf_In_Order(RopePieceBTreeLeaf *കണ്ണി) {
    പ്രതിജ്ഞാ(!PrevLeaf && !NextLeaf && "Already in ordering");

    NextLeaf = കണ്ണി->NextLeaf;
    എങ്കിൽ (NextLeaf)
      NextLeaf->PrevLeaf = &NextLeaf;
    PrevLeaf = &കണ്ണി->NextLeaf;
    കണ്ണി->NextLeaf = ഇത്;
  }

  ശൂന്യം removeFromLeafInOrder() {
    എങ്കിൽ (PrevLeaf) {
      *PrevLeaf = NextLeaf;
      എങ്കിൽ (NextLeaf)
        NextLeaf->PrevLeaf = PrevLeaf;
    } അല്ലെങ്കിൽ എങ്കിൽ (NextLeaf) {
      NextLeaf->PrevLeaf = nullptr;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  ശൂന്യം FullRecomputeSizeLocally() {
    Size = 0;
    വേണ്ടി (unsigned i = 0, e = get_അക്കം_Pieces(); i != e; ++i)
      Size += getPiece(i).size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_കണ്ണി *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_കണ്ണി *തിരുകുക(unsigned Offset, സ്ഥിരം RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  ശൂന്യം erase(unsigned Offset, unsigned അക്കം_Bytes);

  സ്ഥിതം തർക്കം classof(സ്ഥിരം Rope_Piece_BTree_കണ്ണി *N) { തിരികെ N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_കണ്ണി *RopePieceBTreeLeaf::split(unsigned Offset) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  എങ്കിൽ (Offset == 0 || Offset == size()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    തിരികെ nullptr;
  }

  // Find the piece that this offset lands in.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  ആകുമ്പോൾ (Offset >= PieceOffs + Pieces[i].size()) {
    PieceOffs += Pieces[i].size();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  എങ്കിൽ (PieceOffs == Offset)
    തിരികെ nullptr;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  unsigned IntraPieceOffset = Offset - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece Tail(Pieces[i].StrData, Pieces[i].StartOffs + IntraPieceOffset,
                 Pieces[i].EndOffs);
  Size -= Pieces[i].size();
  Pieces[i].EndOffs = Pieces[i].StartOffs + IntraPieceOffset;
  Size += Pieces[i].size();

  തിരികെ തിരുകുക(Offset, Tail);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_കണ്ണി *RopePieceBTreeLeaf::തിരുകുക(unsigned Offset,
                                               സ്ഥിരം RopePiece &R) {
  // If this node is not full, insert the piece.
  എങ്കിൽ (!isFull()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    unsigned i = 0, e = get_അക്കം_Pieces();
    എങ്കിൽ (Offset == size()) {
      // Fastpath for a common case.
      i = e;
    } അല്ലെങ്കിൽ {
      unsigned SlotOffs = 0;
      വേണ്ടി (; Offset > SlotOffs; ++i)
        SlotOffs += getPiece(i).size();
      പ്രതിജ്ഞാ(SlotOffs == Offset && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    വേണ്ടി (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++അക്കം_Pieces;
    Size += R.size();
    തിരികെ nullptr;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  RopePieceBTreeLeaf *New_കണ്ണി = പുതിയ RopePieceBTreeLeaf();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[WidthFactor], &Pieces[2 * WidthFactor],
            &New_കണ്ണി->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[WidthFactor], &Pieces[2 * WidthFactor], RopePiece());

  // Decrease the number of values in the two nodes.
  New_കണ്ണി->അക്കം_Pieces = അക്കം_Pieces = WidthFactor;

  // Recompute the two nodes' size.
  New_കണ്ണി->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();

  // Update the list of leaves.
  New_കണ്ണി->തിരുകുക_After_Leaf_In_Order(ഇത്);

  // These insertions can't fail.
  എങ്കിൽ (ഇത്->size() >= Offset)
    ഇത്->തിരുകുക(Offset, R);
  അല്ലെങ്കിൽ
    New_കണ്ണി->തിരുകുക(Offset - ഇത്->size(), R);
  തിരികെ New_കണ്ണി;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
ശൂന്യം RopePieceBTreeLeaf::erase(unsigned Offset, unsigned അക്കം_Bytes) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  വേണ്ടി (; Offset > PieceOffs; ++i)
    PieceOffs += getPiece(i).size();
  പ്രതിജ്ഞാ(PieceOffs == Offset && "Split didn't occur before erase!");

  unsigned StartPiece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  വേണ്ടി (; Offset + അക്കം_Bytes > PieceOffs + getPiece(i).size(); ++i)
    PieceOffs += getPiece(i).size();

  // If we exactly include the last one, include it in the region to delete.
  എങ്കിൽ (Offset + അക്കം_Bytes == PieceOffs + getPiece(i).size()) {
    PieceOffs += getPiece(i).size();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  എങ്കിൽ (i != StartPiece) {
    unsigned അക്കം_Deleted = i - StartPiece;
    വേണ്ടി (; i != get_അക്കം_Pieces(); ++i)
      Pieces[i - അക്കം_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_അക്കം_Pieces() - അക്കം_Deleted], &Pieces[get_അക്കം_Pieces()],
              RopePiece());
    അക്കം_Pieces -= അക്കം_Deleted;

    unsigned CoverBytes = PieceOffs - Offset;
    അക്കം_Bytes -= CoverBytes;
    Size -= CoverBytes;
  }

  // If we completely removed some stuff, we could be done.
  എങ്കിൽ (അക്കം_Bytes == 0)
    തിരികെ;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  പ്രതിജ്ഞാ(getPiece(StartPiece).size() > അക്കം_Bytes);
  Pieces[StartPiece].StartOffs += അക്കം_Bytes;

  // The size of this node just shrunk by NumBytes.
  Size -= അക്കം_Bytes;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

നാമാകാശ {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
വർഗ്ഗം RopePieceBTreeInterior : പൊതു Rope_Piece_BTree_കണ്ണി {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  unsigned അക്ഷരം അക്കം_Children = 0;

  Rope_Piece_BTree_കണ്ണി *Children[2 * WidthFactor];

പൊതു:
  RopePieceBTreeInterior() : Rope_Piece_BTree_കണ്ണി(അസത്യം) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_കണ്ണി *LHS, Rope_Piece_BTree_കണ്ണി *RHS)
      : Rope_Piece_BTree_കണ്ണി(അസത്യം) {
    Children[0] = LHS;
    Children[1] = RHS;
    അക്കം_Children = 2;
    Size = LHS->size() + RHS->size();
  }

  ~RopePieceBTreeInterior() {
    വേണ്ടി (unsigned i = 0, e = get_അക്കം_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  തർക്കം isFull() സ്ഥിരം { തിരികെ അക്കം_Children == 2 * WidthFactor; }

  unsigned get_അക്കം_Children() സ്ഥിരം { തിരികെ അക്കം_Children; }

  സ്ഥിരം Rope_Piece_BTree_കണ്ണി *getChild(unsigned i) സ്ഥിരം {
    പ്രതിജ്ഞാ(i < അക്കം_Children && "invalid child #");
    തിരികെ Children[i];
  }

  Rope_Piece_BTree_കണ്ണി *getChild(unsigned i) {
    പ്രതിജ്ഞാ(i < അക്കം_Children && "invalid child #");
    തിരികെ Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  ശൂന്യം FullRecomputeSizeLocally() {
    Size = 0;
    വേണ്ടി (unsigned i = 0, e = get_അക്കം_Children(); i != e; ++i)
      Size += getChild(i)->size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_കണ്ണി *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_കണ്ണി *തിരുകുക(unsigned Offset, സ്ഥിരം RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_കണ്ണി *HandleChildPiece(unsigned i, Rope_Piece_BTree_കണ്ണി *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  ശൂന്യം erase(unsigned Offset, unsigned അക്കം_Bytes);

  സ്ഥിതം തർക്കം classof(സ്ഥിരം Rope_Piece_BTree_കണ്ണി *N) { തിരികെ !N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_കണ്ണി *RopePieceBTreeInterior::split(unsigned Offset) {
  // Figure out which child to split.
  എങ്കിൽ (Offset == 0 || Offset == size())
    തിരികെ nullptr; // If we have an exact offset, we're already split.

  unsigned ChildOffset = 0;
  unsigned i = 0;
  വേണ്ടി (; Offset >= ChildOffset + getChild(i)->size(); ++i)
    ChildOffset += getChild(i)->size();

  // If already split there, we're done.
  എങ്കിൽ (ChildOffset == Offset)
    തിരികെ nullptr;

  // Otherwise, recursively split the child.
  എങ്കിൽ (Rope_Piece_BTree_കണ്ണി *RHS = getChild(i)->split(Offset - ChildOffset))
    തിരികെ HandleChildPiece(i, RHS);
  തിരികെ nullptr; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_കണ്ണി *RopePieceBTreeInterior::തിരുകുക(unsigned Offset,
                                                   സ്ഥിരം RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  unsigned i = 0, e = get_അക്കം_Children();

  unsigned ChildOffs = 0;
  എങ്കിൽ (Offset == size()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    ChildOffs = size() - getChild(i)->size();
  } അല്ലെങ്കിൽ {
    വേണ്ടി (; Offset > ChildOffs + getChild(i)->size(); ++i)
      ChildOffs += getChild(i)->size();
  }

  Size += R.size();

  // Insert at the end of this child.
  എങ്കിൽ (Rope_Piece_BTree_കണ്ണി *RHS = getChild(i)->തിരുകുക(Offset - ChildOffs, R))
    തിരികെ HandleChildPiece(i, RHS);

  തിരികെ nullptr;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_കണ്ണി *
RopePieceBTreeInterior::HandleChildPiece(unsigned i, Rope_Piece_BTree_കണ്ണി *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  എങ്കിൽ (!isFull()) {
    // Insert RHS after child 'i'.
    എങ്കിൽ (i + 1 != get_അക്കം_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_അക്കം_Children() - i - 1) * വലുപ്പം(Children[0]));
    Children[i + 1] = RHS;
    ++അക്കം_Children;
    തിരികെ nullptr;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_കണ്ണി = പുതിയ RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  memcpy(&New_കണ്ണി->Children[0], &Children[WidthFactor],
         WidthFactor * വലുപ്പം(Children[0]));

  // Decrease the number of values in the two nodes.
  New_കണ്ണി->അക്കം_Children = അക്കം_Children = WidthFactor;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  എങ്കിൽ (i < WidthFactor)
    ഇത്->HandleChildPiece(i, RHS);
  അല്ലെങ്കിൽ
    New_കണ്ണി->HandleChildPiece(i - WidthFactor, RHS);

  // Recompute the two nodes' size.
  New_കണ്ണി->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();
  തിരികെ New_കണ്ണി;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
ശൂന്യം RopePieceBTreeInterior::erase(unsigned Offset, unsigned അക്കം_Bytes) {
  // This will shrink this node by NumBytes.
  Size -= അക്കം_Bytes;

  // Find the first child that overlaps with Offset.
  unsigned i = 0;
  വേണ്ടി (; Offset >= getChild(i)->size(); ++i)
    Offset -= getChild(i)->size();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  ആകുമ്പോൾ (അക്കം_Bytes) {
    Rope_Piece_BTree_കണ്ണി *CurChild = getChild(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    എങ്കിൽ (Offset + അക്കം_Bytes < CurChild->size()) {
      CurChild->erase(Offset, അക്കം_Bytes);
      തിരികെ;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    എങ്കിൽ (Offset) {
      unsigned BytesFromChild = CurChild->size() - Offset;
      CurChild->erase(Offset, BytesFromChild);
      അക്കം_Bytes -= BytesFromChild;
      // Start at the beginning of the next child.
      Offset = 0;
      ++i;
      തുടരുക;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    അക്കം_Bytes -= CurChild->size();
    CurChild->Destroy();
    --അക്കം_Children;
    എങ്കിൽ (i != get_അക്കം_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_അക്കം_Children() - i) * വലുപ്പം(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

ശൂന്യം Rope_Piece_BTree_കണ്ണി::Destroy() {
  എങ്കിൽ (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(ഇത്))
    delete Leaf;
  അല്ലെങ്കിൽ
    delete cast<RopePieceBTreeInterior>(ഇത്);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_കണ്ണി *Rope_Piece_BTree_കണ്ണി::split(unsigned Offset) {
  പ്രതിജ്ഞാ(Offset <= size() && "Invalid offset to split!");
  എങ്കിൽ (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(ഇത്))
    തിരികെ Leaf->split(Offset);
  തിരികെ cast<RopePieceBTreeInterior>(ഇത്)->split(Offset);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_കണ്ണി *Rope_Piece_BTree_കണ്ണി::തിരുകുക(unsigned Offset,
                                               സ്ഥിരം RopePiece &R) {
  പ്രതിജ്ഞാ(Offset <= size() && "Invalid offset to insert!");
  എങ്കിൽ (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(ഇത്))
    തിരികെ Leaf->തിരുകുക(Offset, R);
  തിരികെ cast<RopePieceBTreeInterior>(ഇത്)->തിരുകുക(Offset, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
ശൂന്യം Rope_Piece_BTree_കണ്ണി::erase(unsigned Offset, unsigned അക്കം_Bytes) {
  പ്രതിജ്ഞാ(Offset + അക്കം_Bytes <= size() && "Invalid offset to erase!");
  എങ്കിൽ (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(ഇത്))
    തിരികെ Leaf->erase(Offset, അക്കം_Bytes);
  തിരികെ cast<RopePieceBTreeInterior>(ഇത്)->erase(Offset, അക്കം_Bytes);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

സ്ഥിതം സ്ഥിരം RopePieceBTreeLeaf *getCN(സ്ഥിരം ശൂന്യം *P) {
  തിരികെ static_cast<സ്ഥിരം RopePieceBTreeLeaf *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(സ്ഥിരം ശൂന്യം *n) {
  സ്ഥിരം auto *N = static_cast<സ്ഥിരം Rope_Piece_BTree_കണ്ണി *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  ആകുമ്പോൾ (സ്ഥിരം auto *IN = dyn_cast<RopePieceBTreeInterior>(N))
    N = IN->getChild(0);

  // We must have at least one leaf.
  Cur_കണ്ണി = cast<RopePieceBTreeLeaf>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  ആകുമ്പോൾ (Cur_കണ്ണി && getCN(Cur_കണ്ണി)->get_അക്കം_Pieces() == 0)
    Cur_കണ്ണി = getCN(Cur_കണ്ണി)->getNextLeafInOrder();

  എങ്കിൽ (Cur_കണ്ണി)
    CurPiece = &getCN(Cur_കണ്ണി)->getPiece(0);
  അല്ലെങ്കിൽ // Empty tree, this is an end() iterator.
    CurPiece = nullptr;
  CurChar = 0;
}

ശൂന്യം RopePieceBTreeIterator::MoveToNextPiece() {
  എങ്കിൽ (CurPiece !=
      &getCN(Cur_കണ്ണി)->getPiece(getCN(Cur_കണ്ണി)->get_അക്കം_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    തിരികെ;
  }

  // Find the next non-empty leaf node.
  ചെയ്യുക
    Cur_കണ്ണി = getCN(Cur_കണ്ണി)->getNextLeafInOrder();
  ആകുമ്പോൾ (Cur_കണ്ണി && getCN(Cur_കണ്ണി)->get_അക്കം_Pieces() == 0);

  എങ്കിൽ (Cur_കണ്ണി)
    CurPiece = &getCN(Cur_കണ്ണി)->getPiece(0);
  അല്ലെങ്കിൽ // Hit end().
    CurPiece = nullptr;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

സ്ഥിതം Rope_Piece_BTree_കണ്ണി *get_വേര്(ശൂന്യം *P) {
  തിരികെ static_cast<Rope_Piece_BTree_കണ്ണി *>(P);
}

RopePieceBTree::RopePieceBTree() { വേര്_2 = പുതിയ RopePieceBTreeLeaf(); }

RopePieceBTree::RopePieceBTree(സ്ഥിരം RopePieceBTree &RHS) {
  പ്രതിജ്ഞാ(RHS.empty() && "Can't copy non-empty tree yet");
  വേര്_2 = പുതിയ RopePieceBTreeLeaf();
}

RopePieceBTree::~RopePieceBTree() { get_വേര്(വേര്_2)->Destroy(); }

unsigned RopePieceBTree::size() സ്ഥിരം { തിരികെ get_വേര്(വേര്_2)->size(); }

ശൂന്യം RopePieceBTree::clear() {
  എങ്കിൽ (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(get_വേര്(വേര്_2)))
    Leaf->clear();
  അല്ലെങ്കിൽ {
    get_വേര്(വേര്_2)->Destroy();
    വേര്_2 = പുതിയ RopePieceBTreeLeaf();
  }
}

ശൂന്യം RopePieceBTree::തിരുകുക(unsigned Offset, സ്ഥിരം RopePiece &R) {
  // #1. Split at Offset.
  എങ്കിൽ (Rope_Piece_BTree_കണ്ണി *RHS = get_വേര്(വേര്_2)->split(Offset))
    വേര്_2 = പുതിയ RopePieceBTreeInterior(get_വേര്(വേര്_2), RHS);

  // #2. Do the insertion.
  എങ്കിൽ (Rope_Piece_BTree_കണ്ണി *RHS = get_വേര്(വേര്_2)->തിരുകുക(Offset, R))
    വേര്_2 = പുതിയ RopePieceBTreeInterior(get_വേര്(വേര്_2), RHS);
}

ശൂന്യം RopePieceBTree::erase(unsigned Offset, unsigned അക്കം_Bytes) {
  // #1. Split at Offset.
  എങ്കിൽ (Rope_Piece_BTree_കണ്ണി *RHS = get_വേര്(വേര്_2)->split(Offset))
    വേര്_2 = പുതിയ RopePieceBTreeInterior(get_വേര്(വേര്_2), RHS);

  // #2. Do the erasing.
  get_വേര്(വേര്_2)->erase(Offset, അക്കം_Bytes);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(സ്ഥിരം അക്ഷരം *Start, സ്ഥിരം അക്ഷരം *End) {
  unsigned Len = End - Start;
  പ്രതിജ്ഞാ(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  എങ്കിൽ (AllocOffs + Len <= AllocChunkSize) {
    memcpy(AllocBuffer->Data + AllocOffs, Start, Len);
    AllocOffs += Len;
    തിരികെ RopePiece(AllocBuffer, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  എങ്കിൽ (Len > AllocChunkSize) {
    unsigned Size = End - Start + വലുപ്പം(Rope_Ref_എണ്ണം_String) - 1;
    auto *Res = reinterpret_cast<Rope_Ref_എണ്ണം_String *>(പുതിയ അക്ഷരം[Size]);
    Res->Ref_എണ്ണം = 0;
    memcpy(Res->Data, Start, End - Start);
    തിരികെ RopePiece(Res, 0, End - Start);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  unsigned AllocSize = offsetof(Rope_Ref_എണ്ണം_String, Data) + AllocChunkSize;
  auto *Res = reinterpret_cast<Rope_Ref_എണ്ണം_String *>(പുതിയ അക്ഷരം[AllocSize]);
  Res->Ref_എണ്ണം = 0;
  memcpy(Res->Data, Start, Len);
  AllocBuffer = Res;
  AllocOffs = Len;

  തിരികെ RopePiece(AllocBuffer, 0, Len);
}
