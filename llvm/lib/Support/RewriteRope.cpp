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
מחלקה RopePieceBTreeNode {
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

  RopePieceBTreeNode(منطقي isLeaf) : IsLeaf(isLeaf) {}
  ~RopePieceBTreeNode() = افتراضي;

عام:
  منطقي isLeaf() ثابت { החזר IsLeaf; }
  unsigned size() ثابت { החזר Size; }

  فراغ Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  RopePieceBTreeNode *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  RopePieceBTreeNode *insert(unsigned Offset, ثابت RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  فراغ erase(unsigned Offset, unsigned NumBytes);
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
מחלקה RopePieceBTreeLeaf : عام RopePieceBTreeNode {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  unsigned حرف NumPieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * WidthFactor];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  RopePieceBTreeLeaf **PrevLeaf = nullptr;
  RopePieceBTreeLeaf *NextLeaf = nullptr;

عام:
  RopePieceBTreeLeaf() : RopePieceBTreeNode(אמת) {}

  ~RopePieceBTreeLeaf() {
    אם (PrevLeaf || NextLeaf)
      removeFromLeafInOrder();
    clear();
  }

  منطقي isFull() ثابت { החזר NumPieces == 2 * WidthFactor; }

  /// clear - Remove all rope pieces from this leaf.
  فراغ clear() {
    כלעוד (NumPieces)
      Pieces[--NumPieces] = RopePiece();
    Size = 0;
  }

  unsigned getNumPieces() ثابت { החזר NumPieces; }

  ثابت RopePiece &getPiece(unsigned i) ثابت {
    أكد(i < getNumPieces() && "Invalid piece ID");
    החזר Pieces[i];
  }

  ثابت RopePieceBTreeLeaf *getNextLeafInOrder() ثابت { החזר NextLeaf; }

  فراغ insertAfterLeafInOrder(RopePieceBTreeLeaf *Node) {
    أكد(!PrevLeaf && !NextLeaf && "Already in ordering");

    NextLeaf = Node->NextLeaf;
    אם (NextLeaf)
      NextLeaf->PrevLeaf = &NextLeaf;
    PrevLeaf = &Node->NextLeaf;
    Node->NextLeaf = هذا;
  }

  فراغ removeFromLeafInOrder() {
    אם (PrevLeaf) {
      *PrevLeaf = NextLeaf;
      אם (NextLeaf)
        NextLeaf->PrevLeaf = PrevLeaf;
    } אחרת אם (NextLeaf) {
      NextLeaf->PrevLeaf = nullptr;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  فراغ FullRecomputeSizeLocally() {
    Size = 0;
    עבור (unsigned i = 0, e = getNumPieces(); i != e; ++i)
      Size += getPiece(i).size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  RopePieceBTreeNode *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  RopePieceBTreeNode *insert(unsigned Offset, ثابت RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  فراغ erase(unsigned Offset, unsigned NumBytes);

  ساكن منطقي classof(ثابت RopePieceBTreeNode *N) { החזר N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
RopePieceBTreeNode *RopePieceBTreeLeaf::split(unsigned Offset) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  אם (Offset == 0 || Offset == size()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    החזר nullptr;
  }

  // Find the piece that this offset lands in.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  כלעוד (Offset >= PieceOffs + Pieces[i].size()) {
    PieceOffs += Pieces[i].size();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  אם (PieceOffs == Offset)
    החזר nullptr;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  unsigned IntraPieceOffset = Offset - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece Tail(Pieces[i].StrData, Pieces[i].StartOffs + IntraPieceOffset,
                 Pieces[i].EndOffs);
  Size -= Pieces[i].size();
  Pieces[i].EndOffs = Pieces[i].StartOffs + IntraPieceOffset;
  Size += Pieces[i].size();

  החזר insert(Offset, Tail);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
RopePieceBTreeNode *RopePieceBTreeLeaf::insert(unsigned Offset,
                                               ثابت RopePiece &R) {
  // If this node is not full, insert the piece.
  אם (!isFull()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    unsigned i = 0, e = getNumPieces();
    אם (Offset == size()) {
      // Fastpath for a common case.
      i = e;
    } אחרת {
      unsigned SlotOffs = 0;
      עבור (; Offset > SlotOffs; ++i)
        SlotOffs += getPiece(i).size();
      أكد(SlotOffs == Offset && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    עבור (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++NumPieces;
    Size += R.size();
    החזר nullptr;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  RopePieceBTreeLeaf *NewNode = جديد RopePieceBTreeLeaf();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[WidthFactor], &Pieces[2 * WidthFactor],
            &NewNode->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[WidthFactor], &Pieces[2 * WidthFactor], RopePiece());

  // Decrease the number of values in the two nodes.
  NewNode->NumPieces = NumPieces = WidthFactor;

  // Recompute the two nodes' size.
  NewNode->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();

  // Update the list of leaves.
  NewNode->insertAfterLeafInOrder(هذا);

  // These insertions can't fail.
  אם (هذا->size() >= Offset)
    هذا->insert(Offset, R);
  אחרת
    NewNode->insert(Offset - هذا->size(), R);
  החזר NewNode;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
فراغ RopePieceBTreeLeaf::erase(unsigned Offset, unsigned NumBytes) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  unsigned PieceOffs = 0;
  unsigned i = 0;
  עבור (; Offset > PieceOffs; ++i)
    PieceOffs += getPiece(i).size();
  أكد(PieceOffs == Offset && "Split didn't occur before erase!");

  unsigned StartPiece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  עבור (; Offset + NumBytes > PieceOffs + getPiece(i).size(); ++i)
    PieceOffs += getPiece(i).size();

  // If we exactly include the last one, include it in the region to delete.
  אם (Offset + NumBytes == PieceOffs + getPiece(i).size()) {
    PieceOffs += getPiece(i).size();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  אם (i != StartPiece) {
    unsigned NumDeleted = i - StartPiece;
    עבור (; i != getNumPieces(); ++i)
      Pieces[i - NumDeleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[getNumPieces() - NumDeleted], &Pieces[getNumPieces()],
              RopePiece());
    NumPieces -= NumDeleted;

    unsigned CoverBytes = PieceOffs - Offset;
    NumBytes -= CoverBytes;
    Size -= CoverBytes;
  }

  // If we completely removed some stuff, we could be done.
  אם (NumBytes == 0)
    החזר;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  أكد(getPiece(StartPiece).size() > NumBytes);
  Pieces[StartPiece].StartOffs += NumBytes;

  // The size of this node just shrunk by NumBytes.
  Size -= NumBytes;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

نطاق {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
מחלקה RopePieceBTreeInterior : عام RopePieceBTreeNode {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  unsigned حرف NumChildren = 0;

  RopePieceBTreeNode *Children[2 * WidthFactor];

عام:
  RopePieceBTreeInterior() : RopePieceBTreeNode(שקר) {}

  RopePieceBTreeInterior(RopePieceBTreeNode *LHS, RopePieceBTreeNode *RHS)
      : RopePieceBTreeNode(שקר) {
    Children[0] = LHS;
    Children[1] = RHS;
    NumChildren = 2;
    Size = LHS->size() + RHS->size();
  }

  ~RopePieceBTreeInterior() {
    עבור (unsigned i = 0, e = getNumChildren(); i != e; ++i)
      Children[i]->Destroy();
  }

  منطقي isFull() ثابت { החזר NumChildren == 2 * WidthFactor; }

  unsigned getNumChildren() ثابت { החזר NumChildren; }

  ثابت RopePieceBTreeNode *getChild(unsigned i) ثابت {
    أكد(i < NumChildren && "invalid child #");
    החזר Children[i];
  }

  RopePieceBTreeNode *getChild(unsigned i) {
    أكد(i < NumChildren && "invalid child #");
    החזר Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  فراغ FullRecomputeSizeLocally() {
    Size = 0;
    עבור (unsigned i = 0, e = getNumChildren(); i != e; ++i)
      Size += getChild(i)->size();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  RopePieceBTreeNode *split(unsigned Offset);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  RopePieceBTreeNode *insert(unsigned Offset, ثابت RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  RopePieceBTreeNode *HandleChildPiece(unsigned i, RopePieceBTreeNode *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  فراغ erase(unsigned Offset, unsigned NumBytes);

  ساكن منطقي classof(ثابت RopePieceBTreeNode *N) { החזר !N->isLeaf(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
RopePieceBTreeNode *RopePieceBTreeInterior::split(unsigned Offset) {
  // Figure out which child to split.
  אם (Offset == 0 || Offset == size())
    החזר nullptr; // If we have an exact offset, we're already split.

  unsigned ChildOffset = 0;
  unsigned i = 0;
  עבור (; Offset >= ChildOffset + getChild(i)->size(); ++i)
    ChildOffset += getChild(i)->size();

  // If already split there, we're done.
  אם (ChildOffset == Offset)
    החזר nullptr;

  // Otherwise, recursively split the child.
  אם (RopePieceBTreeNode *RHS = getChild(i)->split(Offset - ChildOffset))
    החזר HandleChildPiece(i, RHS);
  החזר nullptr; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
RopePieceBTreeNode *RopePieceBTreeInterior::insert(unsigned Offset,
                                                   ثابت RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  unsigned i = 0, e = getNumChildren();

  unsigned ChildOffs = 0;
  אם (Offset == size()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    ChildOffs = size() - getChild(i)->size();
  } אחרת {
    עבור (; Offset > ChildOffs + getChild(i)->size(); ++i)
      ChildOffs += getChild(i)->size();
  }

  Size += R.size();

  // Insert at the end of this child.
  אם (RopePieceBTreeNode *RHS = getChild(i)->insert(Offset - ChildOffs, R))
    החזר HandleChildPiece(i, RHS);

  החזר nullptr;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
RopePieceBTreeNode *
RopePieceBTreeInterior::HandleChildPiece(unsigned i, RopePieceBTreeNode *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  אם (!isFull()) {
    // Insert RHS after child 'i'.
    אם (i + 1 != getNumChildren())
      memmove(&Children[i + 2], &Children[i + 1],
              (getNumChildren() - i - 1) * حجم(Children[0]));
    Children[i + 1] = RHS;
    ++NumChildren;
    החזר nullptr;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *NewNode = جديد RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  memcpy(&NewNode->Children[0], &Children[WidthFactor],
         WidthFactor * حجم(Children[0]));

  // Decrease the number of values in the two nodes.
  NewNode->NumChildren = NumChildren = WidthFactor;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  אם (i < WidthFactor)
    هذا->HandleChildPiece(i, RHS);
  אחרת
    NewNode->HandleChildPiece(i - WidthFactor, RHS);

  // Recompute the two nodes' size.
  NewNode->FullRecomputeSizeLocally();
  FullRecomputeSizeLocally();
  החזר NewNode;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
فراغ RopePieceBTreeInterior::erase(unsigned Offset, unsigned NumBytes) {
  // This will shrink this node by NumBytes.
  Size -= NumBytes;

  // Find the first child that overlaps with Offset.
  unsigned i = 0;
  עבור (; Offset >= getChild(i)->size(); ++i)
    Offset -= getChild(i)->size();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  כלעוד (NumBytes) {
    RopePieceBTreeNode *CurChild = getChild(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    אם (Offset + NumBytes < CurChild->size()) {
      CurChild->erase(Offset, NumBytes);
      החזר;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    אם (Offset) {
      unsigned BytesFromChild = CurChild->size() - Offset;
      CurChild->erase(Offset, BytesFromChild);
      NumBytes -= BytesFromChild;
      // Start at the beginning of the next child.
      Offset = 0;
      ++i;
      استمر;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    NumBytes -= CurChild->size();
    CurChild->Destroy();
    --NumChildren;
    אם (i != getNumChildren())
      memmove(&Children[i], &Children[i + 1],
              (getNumChildren() - i) * حجم(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

فراغ RopePieceBTreeNode::Destroy() {
  אם (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(هذا))
    delete Leaf;
  אחרת
    delete cast<RopePieceBTreeInterior>(هذا);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
RopePieceBTreeNode *RopePieceBTreeNode::split(unsigned Offset) {
  أكد(Offset <= size() && "Invalid offset to split!");
  אם (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(هذا))
    החזר Leaf->split(Offset);
  החזר cast<RopePieceBTreeInterior>(هذا)->split(Offset);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
RopePieceBTreeNode *RopePieceBTreeNode::insert(unsigned Offset,
                                               ثابت RopePiece &R) {
  أكد(Offset <= size() && "Invalid offset to insert!");
  אם (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(هذا))
    החזר Leaf->insert(Offset, R);
  החזר cast<RopePieceBTreeInterior>(هذا)->insert(Offset, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
فراغ RopePieceBTreeNode::erase(unsigned Offset, unsigned NumBytes) {
  أكد(Offset + NumBytes <= size() && "Invalid offset to erase!");
  אם (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(هذا))
    החזר Leaf->erase(Offset, NumBytes);
  החזר cast<RopePieceBTreeInterior>(هذا)->erase(Offset, NumBytes);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

ساكن ثابت RopePieceBTreeLeaf *getCN(ثابت فراغ *P) {
  החזר static_cast<ثابت RopePieceBTreeLeaf *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(ثابت فراغ *n) {
  ثابت auto *N = static_cast<ثابت RopePieceBTreeNode *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  כלעוד (ثابت auto *IN = dyn_cast<RopePieceBTreeInterior>(N))
    N = IN->getChild(0);

  // We must have at least one leaf.
  CurNode = cast<RopePieceBTreeLeaf>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  כלעוד (CurNode && getCN(CurNode)->getNumPieces() == 0)
    CurNode = getCN(CurNode)->getNextLeafInOrder();

  אם (CurNode)
    CurPiece = &getCN(CurNode)->getPiece(0);
  אחרת // Empty tree, this is an end() iterator.
    CurPiece = nullptr;
  CurChar = 0;
}

فراغ RopePieceBTreeIterator::MoveToNextPiece() {
  אם (CurPiece !=
      &getCN(CurNode)->getPiece(getCN(CurNode)->getNumPieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    החזר;
  }

  // Find the next non-empty leaf node.
  עשה
    CurNode = getCN(CurNode)->getNextLeafInOrder();
  כלעוד (CurNode && getCN(CurNode)->getNumPieces() == 0);

  אם (CurNode)
    CurPiece = &getCN(CurNode)->getPiece(0);
  אחרת // Hit end().
    CurPiece = nullptr;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

ساكن RopePieceBTreeNode *getRoot(فراغ *P) {
  החזר static_cast<RopePieceBTreeNode *>(P);
}

RopePieceBTree::RopePieceBTree() { Root = جديد RopePieceBTreeLeaf(); }

RopePieceBTree::RopePieceBTree(ثابت RopePieceBTree &RHS) {
  أكد(RHS.empty() && "Can't copy non-empty tree yet");
  Root = جديد RopePieceBTreeLeaf();
}

RopePieceBTree::~RopePieceBTree() { getRoot(Root)->Destroy(); }

unsigned RopePieceBTree::size() ثابت { החזר getRoot(Root)->size(); }

فراغ RopePieceBTree::clear() {
  אם (auto *Leaf = dyn_cast<RopePieceBTreeLeaf>(getRoot(Root)))
    Leaf->clear();
  אחרת {
    getRoot(Root)->Destroy();
    Root = جديد RopePieceBTreeLeaf();
  }
}

فراغ RopePieceBTree::insert(unsigned Offset, ثابت RopePiece &R) {
  // #1. Split at Offset.
  אם (RopePieceBTreeNode *RHS = getRoot(Root)->split(Offset))
    Root = جديد RopePieceBTreeInterior(getRoot(Root), RHS);

  // #2. Do the insertion.
  אם (RopePieceBTreeNode *RHS = getRoot(Root)->insert(Offset, R))
    Root = جديد RopePieceBTreeInterior(getRoot(Root), RHS);
}

فراغ RopePieceBTree::erase(unsigned Offset, unsigned NumBytes) {
  // #1. Split at Offset.
  אם (RopePieceBTreeNode *RHS = getRoot(Root)->split(Offset))
    Root = جديد RopePieceBTreeInterior(getRoot(Root), RHS);

  // #2. Do the erasing.
  getRoot(Root)->erase(Offset, NumBytes);
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
  אם (AllocOffs + Len <= AllocChunkSize) {
    memcpy(AllocBuffer->Data + AllocOffs, Start, Len);
    AllocOffs += Len;
    החזר RopePiece(AllocBuffer, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  אם (Len > AllocChunkSize) {
    unsigned Size = End - Start + حجم(RopeRefCountString) - 1;
    auto *Res = reinterpret_cast<RopeRefCountString *>(جديد حرف[Size]);
    Res->RefCount = 0;
    memcpy(Res->Data, Start, End - Start);
    החזר RopePiece(Res, 0, End - Start);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  unsigned AllocSize = offsetof(RopeRefCountString, Data) + AllocChunkSize;
  auto *Res = reinterpret_cast<RopeRefCountString *>(جديد حرف[AllocSize]);
  Res->RefCount = 0;
  memcpy(Res->Data, Start, Len);
  AllocBuffer = Res;
  AllocOffs = Len;

  החזר RopePiece(AllocBuffer, 0, Len);
}
