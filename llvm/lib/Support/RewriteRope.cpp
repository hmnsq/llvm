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

using نطاق llvm;

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
صنف Rope_Piece_BTree_عقدة {
محمي:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  تعداد { WidthFactor = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  unsigned Size = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  منطقي IsLeaf;

  Rope_Piece_BTree_عقدة(منطقي isLeaf) : IsLeaf(isLeaf) {}
  ~Rope_Piece_BTree_عقدة() = افتراضي;

عام:
  منطقي isLeaf() ثابت { أرجع IsLeaf; }
  unsigned size() ثابت { أرجع Size; }

  فراغ Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_عقدة *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_عقدة *أدرج(unsigned Offset, ثابت RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  فراغ erase(unsigned Offset, unsigned رقم_Bytes);
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
صنف RopePieceBTreeLeaf : عام Rope_Piece_BTree_عقدة {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  unsigned حرف رقم_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * WidthFactor];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  RopePieceBTreeLeaf **PrevLeaf = nullptr;
  RopePieceBTreeLeaf *NextLeaf = nullptr;

عام:
  RopePieceBTreeLeaf() : Rope_Piece_BTree_عقدة(true) {}

  ~RopePieceBTreeLeaf() {
    إذا (PrevLeaf || NextLeaf)
      removeFromLeafInOrder();
    clear();
  }

  منطقي isFull() ثابت { أرجع رقم_Pieces == 2 * WidthFactor; }

  /// clear - Remove all rope pieces from this leaf.
  فراغ clear() {
    طالما (رقم_Pieces)
      Pieces[--رقم_Pieces] = RopePiece();
    Size = 0;
  }

  unsigned get_رقم_Pieces() ثابت { أرجع رقم_Pieces; }

  ثابت RopePiece &getPiece(unsigned i) ثابت {
    أكد(i < get_رقم_Pieces() && "Invalid piece ID");
    أرجع Pieces[i];
  }

  ثابت RopePieceBTreeLeaf *getNextLeafInOrder() ثابت { أرجع NextLeaf; }

  فراغ أدرج_After_Leaf_In_Order(RopePieceBTreeLeaf *عقدة) {
    أكد(!PrevLeaf && !NextLeaf && "Already in ordering");

    NextLeaf = عقدة->NextLeaf;
    إذا (NextLeaf)
      NextLeaf->PrevLeaf = &NextLeaf;
    PrevLeaf = &عقدة->NextLeaf;
    عقدة->NextLeaf = هذا;
  }

  فراغ removeFromLeafInOrder() {
    إذا (PrevLeaf) {
      *PrevLeaf = NextLeaf;
      إذا (NextLeaf)
        NextLeaf->PrevLeaf = PrevLeaf;
    } وإلا إذا (NextLeaf) {
      NextLeaf->PrevLeaf = nullptr;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  فراغ FullRecomputeSizeLocally() {
    Size = 0;
    لكل (unsigned i = 0, e = get_رقم_Pieces(); i != e; ++i)
      Size += getPiece(i).size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_عقدة *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_عقدة *أدرج(unsigned Offset, ثابت RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  فراغ erase(unsigned Offset, unsigned رقم_Bytes);

  ساكن منطقي classof(ثابت Rope_Piece_BTree_عقدة *N) { أرجع N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_عقدة *RopePieceBTreeLeaf::split(unsigned Offset) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  إذا (Offset == 0 || Offset == size()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    أرجع nullptr;
  }

  // Find the piece that this offset lands in.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  طالما (Offset >= PieceOffs + Pieces[i].size()) {
    PieceOffs += Pieces[i].size();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  إذا (PieceOffs == Offset)
    أرجع nullptr;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  unsigned IntraPieceOffset = Offset - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece Tail(Pieces[i].StrData, Pieces[i].StartOffs + IntraPieceOffset,
                 Pieces[i].EndOffs);
  Size -= Pieces[i].size();
  Pieces[i].EndOffs = Pieces[i].StartOffs + IntraPieceOffset;
  Size += Pieces[i].size();

  أرجع أدرج(Offset, Tail);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_عقدة *RopePieceBTreeLeaf::أدرج(unsigned Offset,
                                               ثابت RopePiece &R) {
  // If this node is not full, insert the piece.
  إذا (!isFull()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    unsigned i = 0, e = get_رقم_Pieces();
    إذا (Offset == size()) {
      // Fastpath for a common case.
      i = e;
    } وإلا {
      unsigned SlotOffs = 0;
      لكل (; Offset > SlotOffs; ++i)
        SlotOffs += getPiece(i).size();
      أكد(SlotOffs == Offset && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    لكل (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++رقم_Pieces;
    Size += R.size();
    أرجع nullptr;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  RopePieceBTreeLeaf *New_عقدة = جديد RopePieceBTreeLeaf();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[WidthFactor], &Pieces[2 * WidthFactor],
            &New_عقدة->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[WidthFactor], &Pieces[2 * WidthFactor], RopePiece());

  // Decrease the number of values in the two nodes.
  New_عقدة->رقم_Pieces = رقم_Pieces = WidthFactor;

  // Recompute the two nodes' size.
  New_عقدة->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();

  // Update the list of leaves.
  New_عقدة->أدرج_After_Leaf_In_Order(هذا);

  // These insertions can't fail.
  إذا (هذا->size() >= Offset)
    هذا->أدرج(Offset, R);
  وإلا
    New_عقدة->أدرج(Offset - هذا->size(), R);
  أرجع New_عقدة;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
فراغ RopePieceBTreeLeaf::erase(unsigned Offset, unsigned رقم_Bytes) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  لكل (; Offset > PieceOffs; ++i)
    PieceOffs += getPiece(i).size();
  أكد(PieceOffs == Offset && "Split didn't occur before erase!");

  unsigned StartPiece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  لكل (; Offset + رقم_Bytes > PieceOffs + getPiece(i).size(); ++i)
    PieceOffs += getPiece(i).size();

  // If we exactly include the last one, include it in the region to delete.
  إذا (Offset + رقم_Bytes == PieceOffs + getPiece(i).size()) {
    PieceOffs += getPiece(i).size();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  إذا (i != StartPiece) {
    unsigned رقم_Deleted = i - StartPiece;
    لكل (; i != get_رقم_Pieces(); ++i)
      Pieces[i - رقم_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_رقم_Pieces() - رقم_Deleted], &Pieces[get_رقم_Pieces()],
              RopePiece());
    رقم_Pieces -= رقم_Deleted;

    unsigned CoverBytes = PieceOffs - Offset;
    رقم_Bytes -= CoverBytes;
    Size -= CoverBytes;
  }

  // If we completely removed some stuff, we could be done.
  إذا (رقم_Bytes == 0)
    أرجع;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  أكد(getPiece(StartPiece).size() > رقم_Bytes);
  Pieces[StartPiece].StartOffs += رقم_Bytes;

  // The size of this node just shrunk by NumBytes.
  Size -= رقم_Bytes;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

نطاق {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
صنف RopePieceBTreeInterior : عام Rope_Piece_BTree_عقدة {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  unsigned حرف رقم_Children = 0;

  Rope_Piece_BTree_عقدة *Children[2 * WidthFactor];

عام:
  RopePieceBTreeInterior() : Rope_Piece_BTree_عقدة(خطأ) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_عقدة *LHS, Rope_Piece_BTree_عقدة *RHS)
      : Rope_Piece_BTree_عقدة(خطأ) {
    Children[0] = LHS;
    Children[1] = RHS;
    رقم_Children = 2;
    Size = LHS->size() + RHS->size();
  }

  ~RopePieceBTreeInterior() {
    لكل (unsigned i = 0, e = get_رقم_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  منطقي isFull() ثابت { أرجع رقم_Children == 2 * WidthFactor; }

  unsigned get_رقم_Children() ثابت { أرجع رقم_Children; }

  ثابت Rope_Piece_BTree_عقدة *getChild(unsigned i) ثابت {
    أكد(i < رقم_Children && "invalid child #");
    أرجع Children[i];
  }

  Rope_Piece_BTree_عقدة *getChild(unsigned i) {
    أكد(i < رقم_Children && "invalid child #");
    أرجع Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  فراغ FullRecomputeSizeLocally() {
    Size = 0;
    لكل (unsigned i = 0, e = get_رقم_Children(); i != e; ++i)
      Size += getChild(i)->size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_عقدة *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_عقدة *أدرج(unsigned Offset, ثابت RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_عقدة *HandleChildPiece(unsigned i, Rope_Piece_BTree_عقدة *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  فراغ erase(unsigned Offset, unsigned رقم_Bytes);

  ساكن منطقي classof(ثابت Rope_Piece_BTree_عقدة *N) { أرجع !N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_عقدة *RopePieceBTreeInterior::split(unsigned Offset) {
  // Figure out which child to split.
  إذا (Offset == 0 || Offset == size())
    أرجع nullptr; // If we have an exact offset, we're already split.

  unsigned ChildOffset = 0;
  unsigned i = 0;
  لكل (; Offset >= ChildOffset + getChild(i)->size(); ++i)
    ChildOffset += getChild(i)->size();

  // If already split there, we're done.
  إذا (ChildOffset == Offset)
    أرجع nullptr;

  // Otherwise, recursively split the child.
  إذا (Rope_Piece_BTree_عقدة *RHS = getChild(i)->split(Offset - ChildOffset))
    أرجع HandleChildPiece(i, RHS);
  أرجع nullptr; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_عقدة *RopePieceBTreeInterior::أدرج(unsigned Offset,
                                                   ثابت RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  unsigned i = 0, e = get_رقم_Children();

  unsigned ChildOffs = 0;
  إذا (Offset == size()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    ChildOffs = size() - getChild(i)->size();
  } وإلا {
    لكل (; Offset > ChildOffs + getChild(i)->size(); ++i)
      ChildOffs += getChild(i)->size();
  }

  Size += R.size();

  // Insert at the end of this child.
  إذا (Rope_Piece_BTree_عقدة *RHS = getChild(i)->أدرج(Offset - ChildOffs, R))
    أرجع HandleChildPiece(i, RHS);

  أرجع nullptr;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_عقدة *
RopePieceBTreeInterior::HandleChildPiece(unsigned i, Rope_Piece_BTree_عقدة *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  إذا (!isFull()) {
    // Insert RHS after child 'i'.
    إذا (i + 1 != get_رقم_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_رقم_Children() - i - 1) * حجم(Children[0]));
    Children[i + 1] = RHS;
    ++رقم_Children;
    أرجع nullptr;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_عقدة = جديد RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  memcpy(&New_عقدة->Children[0], &Children[WidthFactor],
         WidthFactor * حجم(Children[0]));

  // Decrease the number of values in the two nodes.
  New_عقدة->رقم_Children = رقم_Children = WidthFactor;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  إذا (i < WidthFactor)
    هذا->HandleChildPiece(i, RHS);
  وإلا
    New_عقدة->HandleChildPiece(i - WidthFactor, RHS);

  // Recompute the two nodes' size.
  New_عقدة->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();
  أرجع New_عقدة;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
فراغ RopePieceBTreeInterior::erase(unsigned Offset, unsigned رقم_Bytes) {
  // This will shrink this node by NumBytes.
  Size -= رقم_Bytes;

  // Find the first child that overlaps with Offset.
  unsigned i = 0;
  لكل (; Offset >= getChild(i)->size(); ++i)
    Offset -= getChild(i)->size();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  طالما (رقم_Bytes) {
    Rope_Piece_BTree_عقدة *CurChild = getChild(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    إذا (Offset + رقم_Bytes < CurChild->size()) {
      CurChild->erase(Offset, رقم_Bytes);
      أرجع;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    إذا (Offset) {
      unsigned BytesFromChild = CurChild->size() - Offset;
      CurChild->erase(Offset, BytesFromChild);
      رقم_Bytes -= BytesFromChild;
      // Start at the beginning of the next child.
      Offset = 0;
      ++i;
      استمر;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    رقم_Bytes -= CurChild->size();
    CurChild->Destroy();
    --رقم_Children;
    إذا (i != get_رقم_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_رقم_Children() - i) * حجم(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

فراغ Rope_Piece_BTree_عقدة::Destroy() {
  إذا (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(هذا))
    delete Leaf;
  وإلا
    delete cast<RopePieceBTreeInterior>(هذا);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_عقدة *Rope_Piece_BTree_عقدة::split(unsigned Offset) {
  أكد(Offset <= size() && "Invalid offset to split!");
  إذا (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(هذا))
    أرجع Leaf->split(Offset);
  أرجع cast<RopePieceBTreeInterior>(هذا)->split(Offset);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_عقدة *Rope_Piece_BTree_عقدة::أدرج(unsigned Offset,
                                               ثابت RopePiece &R) {
  أكد(Offset <= size() && "Invalid offset to insert!");
  إذا (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(هذا))
    أرجع Leaf->أدرج(Offset, R);
  أرجع cast<RopePieceBTreeInterior>(هذا)->أدرج(Offset, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
فراغ Rope_Piece_BTree_عقدة::erase(unsigned Offset, unsigned رقم_Bytes) {
  أكد(Offset + رقم_Bytes <= size() && "Invalid offset to erase!");
  إذا (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(هذا))
    أرجع Leaf->erase(Offset, رقم_Bytes);
  أرجع cast<RopePieceBTreeInterior>(هذا)->erase(Offset, رقم_Bytes);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

ساكن ثابت RopePieceBTreeLeaf *getCN(ثابت فراغ *P) {
  أرجع static_cast<ثابت RopePieceBTreeLeaf *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(ثابت فراغ *n) {
  ثابت auto *N = static_cast<ثابت Rope_Piece_BTree_عقدة *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  طالما (ثابت auto *IN = dyn_cast<RopePieceBTreeInterior>(N))
    N = IN->getChild(0);

  // We must have at least one leaf.
  Cur_عقدة = cast<RopePieceBTreeLeaf>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  طالما (Cur_عقدة && getCN(Cur_عقدة)->get_رقم_Pieces() == 0)
    Cur_عقدة = getCN(Cur_عقدة)->getNextLeafInOrder();

  إذا (Cur_عقدة)
    CurPiece = &getCN(Cur_عقدة)->getPiece(0);
  وإلا // Empty tree, this is an end() iterator.
    CurPiece = nullptr;
  CurChar = 0;
}

فراغ RopePieceBTreeIterator::MoveToNextPiece() {
  إذا (CurPiece !=
      &getCN(Cur_عقدة)->getPiece(getCN(Cur_عقدة)->get_رقم_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    أرجع;
  }

  // Find the next non-empty leaf node.
  افعل
    Cur_عقدة = getCN(Cur_عقدة)->getNextLeafInOrder();
  طالما (Cur_عقدة && getCN(Cur_عقدة)->get_رقم_Pieces() == 0);

  إذا (Cur_عقدة)
    CurPiece = &getCN(Cur_عقدة)->getPiece(0);
  وإلا // Hit end().
    CurPiece = nullptr;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

ساكن Rope_Piece_BTree_عقدة *get_جذر(فراغ *P) {
  أرجع static_cast<Rope_Piece_BTree_عقدة *>(P);
}

RopePieceBTree::RopePieceBTree() { جذر_2 = جديد RopePieceBTreeLeaf(); }

RopePieceBTree::RopePieceBTree(ثابت RopePieceBTree &RHS) {
  أكد(RHS.empty() && "Can't copy non-empty tree yet");
  جذر_2 = جديد RopePieceBTreeLeaf();
}

RopePieceBTree::~RopePieceBTree() { get_جذر(جذر_2)->Destroy(); }

unsigned RopePieceBTree::size() ثابت { أرجع get_جذر(جذر_2)->size(); }

فراغ RopePieceBTree::clear() {
  إذا (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(get_جذر(جذر_2)))
    Leaf->clear();
  وإلا {
    get_جذر(جذر_2)->Destroy();
    جذر_2 = جديد RopePieceBTreeLeaf();
  }
}

فراغ RopePieceBTree::أدرج(unsigned Offset, ثابت RopePiece &R) {
  // #1. Split at Offset.
  إذا (Rope_Piece_BTree_عقدة *RHS = get_جذر(جذر_2)->split(Offset))
    جذر_2 = جديد RopePieceBTreeInterior(get_جذر(جذر_2), RHS);

  // #2. Do the insertion.
  إذا (Rope_Piece_BTree_عقدة *RHS = get_جذر(جذر_2)->أدرج(Offset, R))
    جذر_2 = جديد RopePieceBTreeInterior(get_جذر(جذر_2), RHS);
}

فراغ RopePieceBTree::erase(unsigned Offset, unsigned رقم_Bytes) {
  // #1. Split at Offset.
  إذا (Rope_Piece_BTree_عقدة *RHS = get_جذر(جذر_2)->split(Offset))
    جذر_2 = جديد RopePieceBTreeInterior(get_جذر(جذر_2), RHS);

  // #2. Do the erasing.
  get_جذر(جذر_2)->erase(Offset, رقم_Bytes);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(ثابت حرف *Start, ثابت حرف *End) {
  unsigned Len = End - Start;
  أكد(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  إذا (AllocOffs + Len <= AllocChunkSize) {
    memcpy(AllocBuffer->Data + AllocOffs, Start, Len);
    AllocOffs += Len;
    أرجع RopePiece(AllocBuffer, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  إذا (Len > AllocChunkSize) {
    unsigned Size = End - Start + حجم(Rope_Ref_عدد_String) - 1;
    auto *Res = reinterpret_cast<Rope_Ref_عدد_String *>(جديد حرف[Size]);
    Res->Ref_عدد = 0;
    memcpy(Res->Data, Start, End - Start);
    أرجع RopePiece(Res, 0, End - Start);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  unsigned AllocSize = offsetof(Rope_Ref_عدد_String, Data) + AllocChunkSize;
  auto *Res = reinterpret_cast<Rope_Ref_عدد_String *>(جديد حرف[AllocSize]);
  Res->Ref_عدد = 0;
  memcpy(Res->Data, Start, Len);
  AllocBuffer = Res;
  AllocOffs = Len;

  أرجع RopePiece(AllocBuffer, 0, Len);
}
