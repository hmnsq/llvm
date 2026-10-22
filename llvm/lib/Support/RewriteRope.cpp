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

using فضاینام llvm;

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

فضاینام {

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Class
//===----------------------------------------------------------------------===//

/// RopePieceBTreeNode - Common base class of RopePieceBTreeLeaf and
/// RopePieceBTreeInterior.  This provides some 'virtual' dispatching methods
/// and a flag that determines which subclass the instance is.  Also
/// important, this node knows the full extend of the node, including any
/// children that it has.  This allows efficient skipping over entire subtrees
/// when looking for an offset in the BTree.
رده Rope_Piece_BTree_گره {
محافظتشده:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  شمارش { WidthFactor = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  unsigned Size = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  منطقی IsLeaf;

  Rope_Piece_BTree_گره(منطقی isLeaf) : IsLeaf(isLeaf) {}
  ~Rope_Piece_BTree_گره() = پیشفرض;

عمومی:
  منطقی isLeaf() ثابت { بازگشت IsLeaf; }
  unsigned size() ثابت { بازگشت Size; }

  تهی Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گره *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گره *درج(unsigned Offset, ثابت RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  تهی erase(unsigned Offset, unsigned رقم_Bytes);
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
رده RopePieceBTreeLeaf : عمومی Rope_Piece_BTree_گره {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  unsigned نویسه رقم_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * WidthFactor];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  RopePieceBTreeLeaf **PrevLeaf = nullptr;
  RopePieceBTreeLeaf *NextLeaf = nullptr;

عمومی:
  RopePieceBTreeLeaf() : Rope_Piece_BTree_گره(درست) {}

  ~RopePieceBTreeLeaf() {
    اگر (PrevLeaf || NextLeaf)
      removeFromLeafInOrder();
    clear();
  }

  منطقی isFull() ثابت { بازگشت رقم_Pieces == 2 * WidthFactor; }

  /// clear - Remove all rope pieces from this leaf.
  تهی clear() {
    وقتی (رقم_Pieces)
      Pieces[--رقم_Pieces] = RopePiece();
    Size = 0;
  }

  unsigned get_رقم_Pieces() ثابت { بازگشت رقم_Pieces; }

  ثابت RopePiece &getPiece(unsigned i) ثابت {
    أكد(i < get_رقم_Pieces() && "Invalid piece ID");
    بازگشت Pieces[i];
  }

  ثابت RopePieceBTreeLeaf *getNextLeafInOrder() ثابت { بازگشت NextLeaf; }

  تهی درج_After_Leaf_In_Order(RopePieceBTreeLeaf *گره) {
    أكد(!PrevLeaf && !NextLeaf && "Already in ordering");

    NextLeaf = گره->NextLeaf;
    اگر (NextLeaf)
      NextLeaf->PrevLeaf = &NextLeaf;
    PrevLeaf = &گره->NextLeaf;
    گره->NextLeaf = این;
  }

  تهی removeFromLeafInOrder() {
    اگر (PrevLeaf) {
      *PrevLeaf = NextLeaf;
      اگر (NextLeaf)
        NextLeaf->PrevLeaf = PrevLeaf;
    } وگرنه اگر (NextLeaf) {
      NextLeaf->PrevLeaf = nullptr;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  تهی FullRecomputeSizeLocally() {
    Size = 0;
    برای (unsigned i = 0, e = get_رقم_Pieces(); i != e; ++i)
      Size += getPiece(i).size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گره *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گره *درج(unsigned Offset, ثابت RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  تهی erase(unsigned Offset, unsigned رقم_Bytes);

  ایستا منطقی classof(ثابت Rope_Piece_BTree_گره *N) { بازگشت N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گره *RopePieceBTreeLeaf::split(unsigned Offset) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  اگر (Offset == 0 || Offset == size()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    بازگشت nullptr;
  }

  // Find the piece that this offset lands in.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  وقتی (Offset >= PieceOffs + Pieces[i].size()) {
    PieceOffs += Pieces[i].size();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  اگر (PieceOffs == Offset)
    بازگشت nullptr;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  unsigned IntraPieceOffset = Offset - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece Tail(Pieces[i].StrData, Pieces[i].StartOffs + IntraPieceOffset,
                 Pieces[i].EndOffs);
  Size -= Pieces[i].size();
  Pieces[i].EndOffs = Pieces[i].StartOffs + IntraPieceOffset;
  Size += Pieces[i].size();

  بازگشت درج(Offset, Tail);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گره *RopePieceBTreeLeaf::درج(unsigned Offset,
                                               ثابت RopePiece &R) {
  // If this node is not full, insert the piece.
  اگر (!isFull()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    unsigned i = 0, e = get_رقم_Pieces();
    اگر (Offset == size()) {
      // Fastpath for a common case.
      i = e;
    } وگرنه {
      unsigned SlotOffs = 0;
      برای (; Offset > SlotOffs; ++i)
        SlotOffs += getPiece(i).size();
      أكد(SlotOffs == Offset && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    برای (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++رقم_Pieces;
    Size += R.size();
    بازگشت nullptr;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  RopePieceBTreeLeaf *New_گره = جدید RopePieceBTreeLeaf();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[WidthFactor], &Pieces[2 * WidthFactor],
            &New_گره->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[WidthFactor], &Pieces[2 * WidthFactor], RopePiece());

  // Decrease the number of values in the two nodes.
  New_گره->رقم_Pieces = رقم_Pieces = WidthFactor;

  // Recompute the two nodes' size.
  New_گره->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();

  // Update the list of leaves.
  New_گره->درج_After_Leaf_In_Order(این);

  // These insertions can't fail.
  اگر (این->size() >= Offset)
    این->درج(Offset, R);
  وگرنه
    New_گره->درج(Offset - این->size(), R);
  بازگشت New_گره;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
تهی RopePieceBTreeLeaf::erase(unsigned Offset, unsigned رقم_Bytes) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  برای (; Offset > PieceOffs; ++i)
    PieceOffs += getPiece(i).size();
  أكد(PieceOffs == Offset && "Split didn't occur before erase!");

  unsigned StartPiece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  برای (; Offset + رقم_Bytes > PieceOffs + getPiece(i).size(); ++i)
    PieceOffs += getPiece(i).size();

  // If we exactly include the last one, include it in the region to delete.
  اگر (Offset + رقم_Bytes == PieceOffs + getPiece(i).size()) {
    PieceOffs += getPiece(i).size();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  اگر (i != StartPiece) {
    unsigned رقم_Deleted = i - StartPiece;
    برای (; i != get_رقم_Pieces(); ++i)
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
  اگر (رقم_Bytes == 0)
    بازگشت;

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

فضاینام {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
رده RopePieceBTreeInterior : عمومی Rope_Piece_BTree_گره {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  unsigned نویسه رقم_Children = 0;

  Rope_Piece_BTree_گره *Children[2 * WidthFactor];

عمومی:
  RopePieceBTreeInterior() : Rope_Piece_BTree_گره(نادرست) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_گره *LHS, Rope_Piece_BTree_گره *RHS)
      : Rope_Piece_BTree_گره(نادرست) {
    Children[0] = LHS;
    Children[1] = RHS;
    رقم_Children = 2;
    Size = LHS->size() + RHS->size();
  }

  ~RopePieceBTreeInterior() {
    برای (unsigned i = 0, e = get_رقم_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  منطقی isFull() ثابت { بازگشت رقم_Children == 2 * WidthFactor; }

  unsigned get_رقم_Children() ثابت { بازگشت رقم_Children; }

  ثابت Rope_Piece_BTree_گره *getChild(unsigned i) ثابت {
    أكد(i < رقم_Children && "invalid child #");
    بازگشت Children[i];
  }

  Rope_Piece_BTree_گره *getChild(unsigned i) {
    أكد(i < رقم_Children && "invalid child #");
    بازگشت Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  تهی FullRecomputeSizeLocally() {
    Size = 0;
    برای (unsigned i = 0, e = get_رقم_Children(); i != e; ++i)
      Size += getChild(i)->size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گره *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_گره *درج(unsigned Offset, ثابت RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_گره *HandleChildPiece(unsigned i, Rope_Piece_BTree_گره *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  تهی erase(unsigned Offset, unsigned رقم_Bytes);

  ایستا منطقی classof(ثابت Rope_Piece_BTree_گره *N) { بازگشت !N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گره *RopePieceBTreeInterior::split(unsigned Offset) {
  // Figure out which child to split.
  اگر (Offset == 0 || Offset == size())
    بازگشت nullptr; // If we have an exact offset, we're already split.

  unsigned ChildOffset = 0;
  unsigned i = 0;
  برای (; Offset >= ChildOffset + getChild(i)->size(); ++i)
    ChildOffset += getChild(i)->size();

  // If already split there, we're done.
  اگر (ChildOffset == Offset)
    بازگشت nullptr;

  // Otherwise, recursively split the child.
  اگر (Rope_Piece_BTree_گره *RHS = getChild(i)->split(Offset - ChildOffset))
    بازگشت HandleChildPiece(i, RHS);
  بازگشت nullptr; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گره *RopePieceBTreeInterior::درج(unsigned Offset,
                                                   ثابت RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  unsigned i = 0, e = get_رقم_Children();

  unsigned ChildOffs = 0;
  اگر (Offset == size()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    ChildOffs = size() - getChild(i)->size();
  } وگرنه {
    برای (; Offset > ChildOffs + getChild(i)->size(); ++i)
      ChildOffs += getChild(i)->size();
  }

  Size += R.size();

  // Insert at the end of this child.
  اگر (Rope_Piece_BTree_گره *RHS = getChild(i)->درج(Offset - ChildOffs, R))
    بازگشت HandleChildPiece(i, RHS);

  بازگشت nullptr;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_گره *
RopePieceBTreeInterior::HandleChildPiece(unsigned i, Rope_Piece_BTree_گره *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  اگر (!isFull()) {
    // Insert RHS after child 'i'.
    اگر (i + 1 != get_رقم_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_رقم_Children() - i - 1) * اندازه(Children[0]));
    Children[i + 1] = RHS;
    ++رقم_Children;
    بازگشت nullptr;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_گره = جدید RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  memcpy(&New_گره->Children[0], &Children[WidthFactor],
         WidthFactor * اندازه(Children[0]));

  // Decrease the number of values in the two nodes.
  New_گره->رقم_Children = رقم_Children = WidthFactor;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  اگر (i < WidthFactor)
    این->HandleChildPiece(i, RHS);
  وگرنه
    New_گره->HandleChildPiece(i - WidthFactor, RHS);

  // Recompute the two nodes' size.
  New_گره->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();
  بازگشت New_گره;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
تهی RopePieceBTreeInterior::erase(unsigned Offset, unsigned رقم_Bytes) {
  // This will shrink this node by NumBytes.
  Size -= رقم_Bytes;

  // Find the first child that overlaps with Offset.
  unsigned i = 0;
  برای (; Offset >= getChild(i)->size(); ++i)
    Offset -= getChild(i)->size();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  وقتی (رقم_Bytes) {
    Rope_Piece_BTree_گره *CurChild = getChild(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    اگر (Offset + رقم_Bytes < CurChild->size()) {
      CurChild->erase(Offset, رقم_Bytes);
      بازگشت;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    اگر (Offset) {
      unsigned BytesFromChild = CurChild->size() - Offset;
      CurChild->erase(Offset, BytesFromChild);
      رقم_Bytes -= BytesFromChild;
      // Start at the beginning of the next child.
      Offset = 0;
      ++i;
      ادامه;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    رقم_Bytes -= CurChild->size();
    CurChild->Destroy();
    --رقم_Children;
    اگر (i != get_رقم_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_رقم_Children() - i) * اندازه(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

تهی Rope_Piece_BTree_گره::Destroy() {
  اگر (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(این))
    delete Leaf;
  وگرنه
    delete cast<RopePieceBTreeInterior>(این);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گره *Rope_Piece_BTree_گره::split(unsigned Offset) {
  أكد(Offset <= size() && "Invalid offset to split!");
  اگر (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(این))
    بازگشت Leaf->split(Offset);
  بازگشت cast<RopePieceBTreeInterior>(این)->split(Offset);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_گره *Rope_Piece_BTree_گره::درج(unsigned Offset,
                                               ثابت RopePiece &R) {
  أكد(Offset <= size() && "Invalid offset to insert!");
  اگر (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(این))
    بازگشت Leaf->درج(Offset, R);
  بازگشت cast<RopePieceBTreeInterior>(این)->درج(Offset, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
تهی Rope_Piece_BTree_گره::erase(unsigned Offset, unsigned رقم_Bytes) {
  أكد(Offset + رقم_Bytes <= size() && "Invalid offset to erase!");
  اگر (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(این))
    بازگشت Leaf->erase(Offset, رقم_Bytes);
  بازگشت cast<RopePieceBTreeInterior>(این)->erase(Offset, رقم_Bytes);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

ایستا ثابت RopePieceBTreeLeaf *getCN(ثابت تهی *P) {
  بازگشت static_cast<ثابت RopePieceBTreeLeaf *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(ثابت تهی *n) {
  ثابت auto *N = static_cast<ثابت Rope_Piece_BTree_گره *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  وقتی (ثابت auto *IN = dyn_cast<RopePieceBTreeInterior>(N))
    N = IN->getChild(0);

  // We must have at least one leaf.
  Cur_گره = cast<RopePieceBTreeLeaf>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  وقتی (Cur_گره && getCN(Cur_گره)->get_رقم_Pieces() == 0)
    Cur_گره = getCN(Cur_گره)->getNextLeafInOrder();

  اگر (Cur_گره)
    CurPiece = &getCN(Cur_گره)->getPiece(0);
  وگرنه // Empty tree, this is an end() iterator.
    CurPiece = nullptr;
  CurChar = 0;
}

تهی RopePieceBTreeIterator::MoveToNextPiece() {
  اگر (CurPiece !=
      &getCN(Cur_گره)->getPiece(getCN(Cur_گره)->get_رقم_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    بازگشت;
  }

  // Find the next non-empty leaf node.
  انجامبده
    Cur_گره = getCN(Cur_گره)->getNextLeafInOrder();
  وقتی (Cur_گره && getCN(Cur_گره)->get_رقم_Pieces() == 0);

  اگر (Cur_گره)
    CurPiece = &getCN(Cur_گره)->getPiece(0);
  وگرنه // Hit end().
    CurPiece = nullptr;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

ایستا Rope_Piece_BTree_گره *get_ریشه(تهی *P) {
  بازگشت static_cast<Rope_Piece_BTree_گره *>(P);
}

RopePieceBTree::RopePieceBTree() { ریشه_2 = جدید RopePieceBTreeLeaf(); }

RopePieceBTree::RopePieceBTree(ثابت RopePieceBTree &RHS) {
  أكد(RHS.empty() && "Can't copy non-empty tree yet");
  ریشه_2 = جدید RopePieceBTreeLeaf();
}

RopePieceBTree::~RopePieceBTree() { get_ریشه(ریشه_2)->Destroy(); }

unsigned RopePieceBTree::size() ثابت { بازگشت get_ریشه(ریشه_2)->size(); }

تهی RopePieceBTree::clear() {
  اگر (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(get_ریشه(ریشه_2)))
    Leaf->clear();
  وگرنه {
    get_ریشه(ریشه_2)->Destroy();
    ریشه_2 = جدید RopePieceBTreeLeaf();
  }
}

تهی RopePieceBTree::درج(unsigned Offset, ثابت RopePiece &R) {
  // #1. Split at Offset.
  اگر (Rope_Piece_BTree_گره *RHS = get_ریشه(ریشه_2)->split(Offset))
    ریشه_2 = جدید RopePieceBTreeInterior(get_ریشه(ریشه_2), RHS);

  // #2. Do the insertion.
  اگر (Rope_Piece_BTree_گره *RHS = get_ریشه(ریشه_2)->درج(Offset, R))
    ریشه_2 = جدید RopePieceBTreeInterior(get_ریشه(ریشه_2), RHS);
}

تهی RopePieceBTree::erase(unsigned Offset, unsigned رقم_Bytes) {
  // #1. Split at Offset.
  اگر (Rope_Piece_BTree_گره *RHS = get_ریشه(ریشه_2)->split(Offset))
    ریشه_2 = جدید RopePieceBTreeInterior(get_ریشه(ریشه_2), RHS);

  // #2. Do the erasing.
  get_ریشه(ریشه_2)->erase(Offset, رقم_Bytes);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(ثابت نویسه *Start, ثابت نویسه *End) {
  unsigned Len = End - Start;
  أكد(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  اگر (AllocOffs + Len <= AllocChunkSize) {
    memcpy(AllocBuffer->Data + AllocOffs, Start, Len);
    AllocOffs += Len;
    بازگشت RopePiece(AllocBuffer, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  اگر (Len > AllocChunkSize) {
    unsigned Size = End - Start + اندازه(Rope_Ref_شمار_String) - 1;
    auto *Res = reinterpret_cast<Rope_Ref_شمار_String *>(جدید نویسه[Size]);
    Res->Ref_شمار = 0;
    memcpy(Res->Data, Start, End - Start);
    بازگشت RopePiece(Res, 0, End - Start);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  unsigned AllocSize = offsetof(Rope_Ref_شمار_String, Data) + AllocChunkSize;
  auto *Res = reinterpret_cast<Rope_Ref_شمار_String *>(جدید نویسه[AllocSize]);
  Res->Ref_شمار = 0;
  memcpy(Res->Data, Start, Len);
  AllocBuffer = Res;
  AllocOffs = Len;

  بازگشت RopePiece(AllocBuffer, 0, Len);
}
