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

उपयुज्य नामाकाश llvm;

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
वर्ग Rope_Piece_BTree_ग्रन्थिः {
रक्षित:
  /// WidthFactor - This controls the number of K/V slots held in the BTree:
  /// how wide it is.  Each level of the BTree is guaranteed to have at least
  /// 'WidthFactor' elements in it (either ropepieces or children), (except
  /// the root, which may have less) and may have at most 2*WidthFactor
  /// elements.
  गणना { विस्तारः_गुणकः = 8 };

  /// Size - This is the number of bytes of file this node (including any
  /// potential children) covers.
  अचिह्नित आकार = 0;

  /// IsLeaf - True if this is an instance of RopePieceBTreeLeaf, false if it
  /// is an instance of RopePieceBTreeInterior.
  तार्किक Is_पर्णम्;

  Rope_Piece_BTree_ग्रन्थिः(तार्किक is_पर्णम्) : Is_पर्णम्(is_पर्णम्) {}
  ~Rope_Piece_BTree_ग्रन्थिः() = स्वभाव;

सार्वजनिक:
  तार्किक is_पर्णम्() ध्रुव { प्रत्यागम Is_पर्णम्; }
  अचिह्नित आकार_2() ध्रुव { प्रत्यागम आकार; }

  शून्य Destroy();

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ग्रन्थिः *विभज(अचिह्नित विस्थापनम्);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ग्रन्थिः *निवेशय(अचिह्नित विस्थापनम्, ध्रुव RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  शून्य erase(अचिह्नित विस्थापनम्, अचिह्नित अङ्कः_अष्टकानि);
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
वर्ग Rope_Piece_BTree_पर्णम् : सार्वजनिक Rope_Piece_BTree_ग्रन्थिः {
  /// NumPieces - This holds the number of rope pieces currently active in the
  /// Pieces array.
  अचिह्नित वर्ण अङ्कः_Pieces = 0;

  /// Pieces - This tracks the file chunks currently in this leaf.
  RopePiece Pieces[2 * विस्तारः_गुणकः];

  /// NextLeaf - This is a pointer to the next leaf in the tree, allowing
  /// efficient in-order forward iteration of the tree without traversal.
  Rope_Piece_BTree_पर्णम् **पूर्व_पर्णम् = शून्यसूचकः;
  Rope_Piece_BTree_पर्णम् *अग्र_पर्णम् = शून्यसूचकः;

सार्वजनिक:
  Rope_Piece_BTree_पर्णम्() : Rope_Piece_BTree_ग्रन्थिः(सत्य) {}

  ~Rope_Piece_BTree_पर्णम्() {
    यदि (पूर्व_पर्णम् || अग्र_पर्णम्)
      remove_From_पर्णम्_अन्तर्_क्रमः();
    clear();
  }

  तार्किक is_पूर्णम्() ध्रुव { प्रत्यागम अङ्कः_Pieces == 2 * विस्तारः_गुणकः; }

  /// clear - Remove all rope pieces from this leaf.
  शून्य clear() {
    यावत् (अङ्कः_Pieces)
      Pieces[--अङ्कः_Pieces] = RopePiece();
    आकार = 0;
  }

  अचिह्नित get_अङ्कः_Pieces() ध्रुव { प्रत्यागम अङ्कः_Pieces; }

  ध्रुव RopePiece &getPiece(अचिह्नित i) ध्रुव {
    प्रतिज्ञा(i < get_अङ्कः_Pieces() && "Invalid piece ID");
    प्रत्यागम Pieces[i];
  }

  ध्रुव Rope_Piece_BTree_पर्णम् *get_अग्र_पर्णम्_अन्तर्_क्रमः() ध्रुव { प्रत्यागम अग्र_पर्णम्; }

  शून्य निवेशय_After_पर्णम्_अन्तर्_क्रमः(Rope_Piece_BTree_पर्णम् *ग्रन्थिः) {
    प्रतिज्ञा(!पूर्व_पर्णम् && !अग्र_पर्णम् && "Already in ordering");

    अग्र_पर्णम् = ग्रन्थिः->अग्र_पर्णम्;
    यदि (अग्र_पर्णम्)
      अग्र_पर्णम्->पूर्व_पर्णम् = &अग्र_पर्णम्;
    पूर्व_पर्णम् = &ग्रन्थिः->अग्र_पर्णम्;
    ग्रन्थिः->अग्र_पर्णम् = एतत्;
  }

  शून्य remove_From_पर्णम्_अन्तर्_क्रमः() {
    यदि (पूर्व_पर्णम्) {
      *पूर्व_पर्णम् = अग्र_पर्णम्;
      यदि (अग्र_पर्णम्)
        अग्र_पर्णम्->पूर्व_पर्णम् = पूर्व_पर्णम्;
    } अन्यथा यदि (अग्र_पर्णम्) {
      अग्र_पर्णम्->पूर्व_पर्णम् = शून्यसूचकः;
    }
  }

  /// FullRecomputeSizeLocally - This method recomputes the 'Size' field by
  /// summing the size of all RopePieces.
  शून्य पूर्णम्_Recompute_आकार_Locally() {
    आकार = 0;
    प्रति (अचिह्नित i = 0, e = get_अङ्कः_Pieces(); i != e; ++i)
      आकार += getPiece(i).आकार_2();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ग्रन्थिः *विभज(अचिह्नित विस्थापनम्);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ग्रन्थिः *निवेशय(अचिह्नित विस्थापनम्, ध्रुव RopePiece &R);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  शून्य erase(अचिह्नित विस्थापनम्, अचिह्नित अङ्कः_अष्टकानि);

  स्थिर तार्किक classof(ध्रुव Rope_Piece_BTree_ग्रन्थिः *N) { प्रत्यागम N->is_पर्णम्(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ग्रन्थिः *Rope_Piece_BTree_पर्णम्::विभज(अचिह्नित विस्थापनम्) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  यदि (विस्थापनम् == 0 || विस्थापनम् == आकार_2()) {
    // Fastpath for a common case.  There is already a splitpoint at the end.
    प्रत्यागम शून्यसूचकः;
  }

  // Find the piece that this offset lands in.
  अचिह्नित PieceOffs = 0;
  अचिह्नित i = 0;
  यावत् (विस्थापनम् >= PieceOffs + Pieces[i].आकार_2()) {
    PieceOffs += Pieces[i].आकार_2();
    ++i;
  }

  // If there is already a split point at the specified offset, just return
  // success.
  यदि (PieceOffs == विस्थापनम्)
    प्रत्यागम शून्यसूचकः;

  // Otherwise, we need to split piece 'i' at Offset-PieceOffs.  Convert Offset
  // to being Piece relative.
  अचिह्नित Intra_Piece_विस्थापनम् = विस्थापनम् - PieceOffs;

  // We do this by shrinking the RopePiece and then doing an insert of the tail.
  RopePiece पुच्छ(Pieces[i].Str_दत्त, Pieces[i].StartOffs + Intra_Piece_विस्थापनम्,
                 Pieces[i].अन्तः_Offs);
  आकार -= Pieces[i].आकार_2();
  Pieces[i].अन्तः_Offs = Pieces[i].StartOffs + Intra_Piece_विस्थापनम्;
  आकार += Pieces[i].आकार_2();

  प्रत्यागम निवेशय(विस्थापनम्, पुच्छ);
}

/// insert - Insert the specified RopePiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ग्रन्थिः *Rope_Piece_BTree_पर्णम्::निवेशय(अचिह्नित विस्थापनम्,
                                               ध्रुव RopePiece &R) {
  // If this node is not full, insert the piece.
  यदि (!is_पूर्णम्()) {
    // Find the insertion point.  We are guaranteed that there is a split at the
    // specified offset so find it.
    अचिह्नित i = 0, e = get_अङ्कः_Pieces();
    यदि (विस्थापनम् == आकार_2()) {
      // Fastpath for a common case.
      i = e;
    } अन्यथा {
      अचिह्नित SlotOffs = 0;
      प्रति (; विस्थापनम् > SlotOffs; ++i)
        SlotOffs += getPiece(i).आकार_2();
      प्रतिज्ञा(SlotOffs == विस्थापनम् && "Split didn't occur before insertion!");
    }

    // For an insertion into a non-full leaf node, just insert the value in
    // its sorted position.  This requires moving later values over.
    प्रति (; i != e; --e)
      Pieces[e] = Pieces[e - 1];
    Pieces[i] = R;
    ++अङ्कः_Pieces;
    आकार += R.आकार_2();
    प्रत्यागम शून्यसूचकः;
  }

  // Otherwise, if this is leaf is full, split it in two halves.  Since this
  // node is full, it contains 2*WidthFactor values.  We move the first
  // 'WidthFactor' values to the LHS child (which we leave in this node) and
  // move the last 'WidthFactor' values into the RHS child.

  // Create the new node.
  Rope_Piece_BTree_पर्णम् *New_ग्रन्थिः = नूतन Rope_Piece_BTree_पर्णम्();

  // Move over the last 'WidthFactor' values from here to NewNode.
  std::copy(&Pieces[विस्तारः_गुणकः], &Pieces[2 * विस्तारः_गुणकः],
            &New_ग्रन्थिः->Pieces[0]);
  // Replace old pieces with null RopePieces to drop refcounts.
  std::fill(&Pieces[विस्तारः_गुणकः], &Pieces[2 * विस्तारः_गुणकः], RopePiece());

  // Decrease the number of values in the two nodes.
  New_ग्रन्थिः->अङ्कः_Pieces = अङ्कः_Pieces = विस्तारः_गुणकः;

  // Recompute the two nodes' size.
  New_ग्रन्थिः->पूर्णम्_Recompute_आकार_Locally();
  पूर्णम्_Recompute_आकार_Locally();

  // Update the list of leaves.
  New_ग्रन्थिः->निवेशय_After_पर्णम्_अन्तर्_क्रमः(एतत्);

  // These insertions can't fail.
  यदि (एतत्->आकार_2() >= विस्थापनम्)
    एतत्->निवेशय(विस्थापनम्, R);
  अन्यथा
    New_ग्रन्थिः->निवेशय(विस्थापनम् - एतत्->आकार_2(), R);
  प्रत्यागम New_ग्रन्थिः;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
शून्य Rope_Piece_BTree_पर्णम्::erase(अचिह्नित विस्थापनम्, अचिह्नित अङ्कः_अष्टकानि) {
  // Since we are guaranteed that there is a split at Offset, we start by
  // finding the Piece that starts there.
  अचिह्नित PieceOffs = 0;
  अचिह्नित i = 0;
  प्रति (; विस्थापनम् > PieceOffs; ++i)
    PieceOffs += getPiece(i).आकार_2();
  प्रतिज्ञा(PieceOffs == विस्थापनम् && "Split didn't occur before erase!");

  अचिह्नित StartPiece = i;

  // Figure out how many pieces completely cover 'NumBytes'.  We want to remove
  // all of them.
  प्रति (; विस्थापनम् + अङ्कः_अष्टकानि > PieceOffs + getPiece(i).आकार_2(); ++i)
    PieceOffs += getPiece(i).आकार_2();

  // If we exactly include the last one, include it in the region to delete.
  यदि (विस्थापनम् + अङ्कः_अष्टकानि == PieceOffs + getPiece(i).आकार_2()) {
    PieceOffs += getPiece(i).आकार_2();
    ++i;
  }

  // If we completely cover some RopePieces, erase them now.
  यदि (i != StartPiece) {
    अचिह्नित अङ्कः_Deleted = i - StartPiece;
    प्रति (; i != get_अङ्कः_Pieces(); ++i)
      Pieces[i - अङ्कः_Deleted] = Pieces[i];

    // Drop references to dead rope pieces.
    std::fill(&Pieces[get_अङ्कः_Pieces() - अङ्कः_Deleted], &Pieces[get_अङ्कः_Pieces()],
              RopePiece());
    अङ्कः_Pieces -= अङ्कः_Deleted;

    अचिह्नित Cover_अष्टकानि = PieceOffs - विस्थापनम्;
    अङ्कः_अष्टकानि -= Cover_अष्टकानि;
    आकार -= Cover_अष्टकानि;
  }

  // If we completely removed some stuff, we could be done.
  यदि (अङ्कः_अष्टकानि == 0)
    प्रत्यागम;

  // Okay, now might be erasing part of some Piece.  If this is the case, then
  // move the start point of the piece.
  प्रतिज्ञा(getPiece(StartPiece).आकार_2() > अङ्कः_अष्टकानि);
  Pieces[StartPiece].StartOffs += अङ्कः_अष्टकानि;

  // The size of this node just shrunk by NumBytes.
  आकार -= अङ्कः_अष्टकानि;
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeInterior Class
//===----------------------------------------------------------------------===//

नामाकाश {

/// RopePieceBTreeInterior - This represents an interior node in the B+Tree,
/// which holds up to 2*WidthFactor pointers to child nodes.
वर्ग RopePieceBTreeInterior : सार्वजनिक Rope_Piece_BTree_ग्रन्थिः {
  /// NumChildren - This holds the number of children currently active in the
  /// Children array.
  अचिह्नित वर्ण अङ्कः_Children = 0;

  Rope_Piece_BTree_ग्रन्थिः *Children[2 * विस्तारः_गुणकः];

सार्वजनिक:
  RopePieceBTreeInterior() : Rope_Piece_BTree_ग्रन्थिः(असत्य) {}

  RopePieceBTreeInterior(Rope_Piece_BTree_ग्रन्थिः *LHS, Rope_Piece_BTree_ग्रन्थिः *RHS)
      : Rope_Piece_BTree_ग्रन्थिः(असत्य) {
    Children[0] = LHS;
    Children[1] = RHS;
    अङ्कः_Children = 2;
    आकार = LHS->आकार_2() + RHS->आकार_2();
  }

  ~RopePieceBTreeInterior() {
    प्रति (अचिह्नित i = 0, e = get_अङ्कः_Children(); i != e; ++i)
      Children[i]->Destroy();
  }

  तार्किक is_पूर्णम्() ध्रुव { प्रत्यागम अङ्कः_Children == 2 * विस्तारः_गुणकः; }

  अचिह्नित get_अङ्कः_Children() ध्रुव { प्रत्यागम अङ्कः_Children; }

  ध्रुव Rope_Piece_BTree_ग्रन्थिः *getChild(अचिह्नित i) ध्रुव {
    प्रतिज्ञा(i < अङ्कः_Children && "invalid child #");
    प्रत्यागम Children[i];
  }

  Rope_Piece_BTree_ग्रन्थिः *getChild(अचिह्नित i) {
    प्रतिज्ञा(i < अङ्कः_Children && "invalid child #");
    प्रत्यागम Children[i];
  }

  /// FullRecomputeSizeLocally - Recompute the Size field of this node by
  /// summing up the sizes of the child nodes.
  शून्य पूर्णम्_Recompute_आकार_Locally() {
    आकार = 0;
    प्रति (अचिह्नित i = 0, e = get_अङ्कः_Children(); i != e; ++i)
      आकार += getChild(i)->आकार_2();
  }

  /// split - Split the range containing the specified offset so that we are
  /// guaranteed that there is a place to do an insertion at the specified
  /// offset.  The offset is relative, so "0" is the start of the node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ग्रन्थिः *विभज(अचिह्नित विस्थापनम्);

  /// insert - Insert the specified ropepiece into this tree node at the
  /// specified offset.  The offset is relative, so "0" is the start of the
  /// node.
  ///
  /// If there is no space in this subtree for the extra piece, the extra tree
  /// node is returned and must be inserted into a parent.
  Rope_Piece_BTree_ग्रन्थिः *निवेशय(अचिह्नित विस्थापनम्, ध्रुव RopePiece &R);

  /// HandleChildPiece - A child propagated an insertion result up to us.
  /// Insert the new child, and/or propagate the result further up the tree.
  Rope_Piece_BTree_ग्रन्थिः *HandleChildPiece(अचिह्नित i, Rope_Piece_BTree_ग्रन्थिः *RHS);

  /// erase - Remove NumBytes from this node at the specified offset.  We are
  /// guaranteed that there is a split at Offset.
  शून्य erase(अचिह्नित विस्थापनम्, अचिह्नित अङ्कः_अष्टकानि);

  स्थिर तार्किक classof(ध्रुव Rope_Piece_BTree_ग्रन्थिः *N) { प्रत्यागम !N->is_पर्णम्(); }
};

} // namespace

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ग्रन्थिः *RopePieceBTreeInterior::विभज(अचिह्नित विस्थापनम्) {
  // Figure out which child to split.
  यदि (विस्थापनम् == 0 || विस्थापनम् == आकार_2())
    प्रत्यागम शून्यसूचकः; // If we have an exact offset, we're already split.

  अचिह्नित Child_विस्थापनम् = 0;
  अचिह्नित i = 0;
  प्रति (; विस्थापनम् >= Child_विस्थापनम् + getChild(i)->आकार_2(); ++i)
    Child_विस्थापनम् += getChild(i)->आकार_2();

  // If already split there, we're done.
  यदि (Child_विस्थापनम् == विस्थापनम्)
    प्रत्यागम शून्यसूचकः;

  // Otherwise, recursively split the child.
  यदि (Rope_Piece_BTree_ग्रन्थिः *RHS = getChild(i)->विभज(विस्थापनम् - Child_विस्थापनम्))
    प्रत्यागम HandleChildPiece(i, RHS);
  प्रत्यागम शून्यसूचकः; // Done!
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ग्रन्थिः *RopePieceBTreeInterior::निवेशय(अचिह्नित विस्थापनम्,
                                                   ध्रुव RopePiece &R) {
  // Find the insertion point.  We are guaranteed that there is a split at the
  // specified offset so find it.
  अचिह्नित i = 0, e = get_अङ्कः_Children();

  अचिह्नित ChildOffs = 0;
  यदि (विस्थापनम् == आकार_2()) {
    // Fastpath for a common case.  Insert at end of last child.
    i = e - 1;
    ChildOffs = आकार_2() - getChild(i)->आकार_2();
  } अन्यथा {
    प्रति (; विस्थापनम् > ChildOffs + getChild(i)->आकार_2(); ++i)
      ChildOffs += getChild(i)->आकार_2();
  }

  आकार += R.आकार_2();

  // Insert at the end of this child.
  यदि (Rope_Piece_BTree_ग्रन्थिः *RHS = getChild(i)->निवेशय(विस्थापनम् - ChildOffs, R))
    प्रत्यागम HandleChildPiece(i, RHS);

  प्रत्यागम शून्यसूचकः;
}

/// HandleChildPiece - A child propagated an insertion result up to us.
/// Insert the new child, and/or propagate the result further up the tree.
Rope_Piece_BTree_ग्रन्थिः *
RopePieceBTreeInterior::HandleChildPiece(अचिह्नित i, Rope_Piece_BTree_ग्रन्थिः *RHS) {
  // Otherwise the child propagated a subtree up to us as a new child.  See if
  // we have space for it here.
  यदि (!is_पूर्णम्()) {
    // Insert RHS after child 'i'.
    यदि (i + 1 != get_अङ्कः_Children())
      memmove(&Children[i + 2], &Children[i + 1],
              (get_अङ्कः_Children() - i - 1) * परिमाण(Children[0]));
    Children[i + 1] = RHS;
    ++अङ्कः_Children;
    प्रत्यागम शून्यसूचकः;
  }

  // Okay, this node is full.  Split it in half, moving WidthFactor children to
  // a newly allocated interior node.

  // Create the new node.
  RopePieceBTreeInterior *New_ग्रन्थिः = नूतन RopePieceBTreeInterior();

  // Move over the last 'WidthFactor' values from here to NewNode.
  स्मृतिं_प्रतिलिख(&New_ग्रन्थिः->Children[0], &Children[विस्तारः_गुणकः],
         विस्तारः_गुणकः * परिमाण(Children[0]));

  // Decrease the number of values in the two nodes.
  New_ग्रन्थिः->अङ्कः_Children = अङ्कः_Children = विस्तारः_गुणकः;

  // Finally, insert the two new children in the side the can (now) hold them.
  // These insertions can't fail.
  यदि (i < विस्तारः_गुणकः)
    एतत्->HandleChildPiece(i, RHS);
  अन्यथा
    New_ग्रन्थिः->HandleChildPiece(i - विस्तारः_गुणकः, RHS);

  // Recompute the two nodes' size.
  New_ग्रन्थिः->पूर्णम्_Recompute_आकार_Locally();
  पूर्णम्_Recompute_आकार_Locally();
  प्रत्यागम New_ग्रन्थिः;
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
शून्य RopePieceBTreeInterior::erase(अचिह्नित विस्थापनम्, अचिह्नित अङ्कः_अष्टकानि) {
  // This will shrink this node by NumBytes.
  आकार -= अङ्कः_अष्टकानि;

  // Find the first child that overlaps with Offset.
  अचिह्नित i = 0;
  प्रति (; विस्थापनम् >= getChild(i)->आकार_2(); ++i)
    विस्थापनम् -= getChild(i)->आकार_2();

  // Propagate the delete request into overlapping children, or completely
  // delete the children as appropriate.
  यावत् (अङ्कः_अष्टकानि) {
    Rope_Piece_BTree_ग्रन्थिः *CurChild = getChild(i);

    // If we are deleting something contained entirely in the child, pass on the
    // request.
    यदि (विस्थापनम् + अङ्कः_अष्टकानि < CurChild->आकार_2()) {
      CurChild->erase(विस्थापनम्, अङ्कः_अष्टकानि);
      प्रत्यागम;
    }

    // If this deletion request starts somewhere in the middle of the child, it
    // must be deleting to the end of the child.
    यदि (विस्थापनम्) {
      अचिह्नित अष्टकानि_From_Child = CurChild->आकार_2() - विस्थापनम्;
      CurChild->erase(विस्थापनम्, अष्टकानि_From_Child);
      अङ्कः_अष्टकानि -= अष्टकानि_From_Child;
      // Start at the beginning of the next child.
      विस्थापनम् = 0;
      ++i;
      अनुवर्त;
    }

    // If the deletion request completely covers the child, delete it and move
    // the rest down.
    अङ्कः_अष्टकानि -= CurChild->आकार_2();
    CurChild->Destroy();
    --अङ्कः_Children;
    यदि (i != get_अङ्कः_Children())
      memmove(&Children[i], &Children[i + 1],
              (get_अङ्कः_Children() - i) * परिमाण(Children[0]));
  }
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeNode Implementation
//===----------------------------------------------------------------------===//

शून्य Rope_Piece_BTree_ग्रन्थिः::Destroy() {
  यदि (स्वचालित *पर्णम् = dyn_cast<Rope_Piece_BTree_पर्णम्>(एतत्))
    विलोपय पर्णम्;
  अन्यथा
    विलोपय cast<RopePieceBTreeInterior>(एतत्);
}

/// split - Split the range containing the specified offset so that we are
/// guaranteed that there is a place to do an insertion at the specified
/// offset.  The offset is relative, so "0" is the start of the node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ग्रन्थिः *Rope_Piece_BTree_ग्रन्थिः::विभज(अचिह्नित विस्थापनम्) {
  प्रतिज्ञा(विस्थापनम् <= आकार_2() && "Invalid offset to split!");
  यदि (स्वचालित *पर्णम् = dyn_cast<Rope_Piece_BTree_पर्णम्>(एतत्))
    प्रत्यागम पर्णम्->विभज(विस्थापनम्);
  प्रत्यागम cast<RopePieceBTreeInterior>(एतत्)->विभज(विस्थापनम्);
}

/// insert - Insert the specified ropepiece into this tree node at the
/// specified offset.  The offset is relative, so "0" is the start of the
/// node.
///
/// If there is no space in this subtree for the extra piece, the extra tree
/// node is returned and must be inserted into a parent.
Rope_Piece_BTree_ग्रन्थिः *Rope_Piece_BTree_ग्रन्थिः::निवेशय(अचिह्नित विस्थापनम्,
                                               ध्रुव RopePiece &R) {
  प्रतिज्ञा(विस्थापनम् <= आकार_2() && "Invalid offset to insert!");
  यदि (स्वचालित *पर्णम् = dyn_cast<Rope_Piece_BTree_पर्णम्>(एतत्))
    प्रत्यागम पर्णम्->निवेशय(विस्थापनम्, R);
  प्रत्यागम cast<RopePieceBTreeInterior>(एतत्)->निवेशय(विस्थापनम्, R);
}

/// erase - Remove NumBytes from this node at the specified offset.  We are
/// guaranteed that there is a split at Offset.
शून्य Rope_Piece_BTree_ग्रन्थिः::erase(अचिह्नित विस्थापनम्, अचिह्नित अङ्कः_अष्टकानि) {
  प्रतिज्ञा(विस्थापनम् + अङ्कः_अष्टकानि <= आकार_2() && "Invalid offset to erase!");
  यदि (स्वचालित *पर्णम् = dyn_cast<Rope_Piece_BTree_पर्णम्>(एतत्))
    प्रत्यागम पर्णम्->erase(विस्थापनम्, अङ्कः_अष्टकानि);
  प्रत्यागम cast<RopePieceBTreeInterior>(एतत्)->erase(विस्थापनम्, अङ्कः_अष्टकानि);
}

//===----------------------------------------------------------------------===//
// RopePieceBTreeIterator Implementation
//===----------------------------------------------------------------------===//

स्थिर ध्रुव Rope_Piece_BTree_पर्णम् *getCN(ध्रुव शून्य *P) {
  प्रत्यागम static_cast<ध्रुव Rope_Piece_BTree_पर्णम् *>(P);
}

// begin iterator.
RopePieceBTreeIterator::RopePieceBTreeIterator(ध्रुव शून्य *n) {
  ध्रुव स्वचालित *N = static_cast<ध्रुव Rope_Piece_BTree_ग्रन्थिः *>(n);

  // Walk down the left side of the tree until we get to a leaf.
  यावत् (ध्रुव स्वचालित *अन्तर् = dyn_cast<RopePieceBTreeInterior>(N))
    N = अन्तर्->getChild(0);

  // We must have at least one leaf.
  Cur_ग्रन्थिः = cast<Rope_Piece_BTree_पर्णम्>(N);

  // If we found a leaf that happens to be empty, skip over it until we get
  // to something full.
  यावत् (Cur_ग्रन्थिः && getCN(Cur_ग्रन्थिः)->get_अङ्कः_Pieces() == 0)
    Cur_ग्रन्थिः = getCN(Cur_ग्रन्थिः)->get_अग्र_पर्णम्_अन्तर्_क्रमः();

  यदि (Cur_ग्रन्थिः)
    CurPiece = &getCN(Cur_ग्रन्थिः)->getPiece(0);
  अन्यथा // Empty tree, this is an end() iterator.
    CurPiece = शून्यसूचकः;
  CurChar = 0;
}

शून्य RopePieceBTreeIterator::Move_गन्तव्यम्_अग्र_Piece() {
  यदि (CurPiece !=
      &getCN(Cur_ग्रन्थिः)->getPiece(getCN(Cur_ग्रन्थिः)->get_अङ्कः_Pieces() - 1)) {
    CurChar = 0;
    ++CurPiece;
    प्रत्यागम;
  }

  // Find the next non-empty leaf node.
  कुरु
    Cur_ग्रन्थिः = getCN(Cur_ग्रन्थिः)->get_अग्र_पर्णम्_अन्तर्_क्रमः();
  यावत् (Cur_ग्रन्थिः && getCN(Cur_ग्रन्थिः)->get_अङ्कः_Pieces() == 0);

  यदि (Cur_ग्रन्थिः)
    CurPiece = &getCN(Cur_ग्रन्थिः)->getPiece(0);
  अन्यथा // Hit end().
    CurPiece = शून्यसूचकः;
  CurChar = 0;
}

//===----------------------------------------------------------------------===//
// RopePieceBTree Implementation
//===----------------------------------------------------------------------===//

स्थिर Rope_Piece_BTree_ग्रन्थिः *get_मूलम्(शून्य *P) {
  प्रत्यागम static_cast<Rope_Piece_BTree_ग्रन्थिः *>(P);
}

RopePieceBTree::RopePieceBTree() { मूलम्_2 = नूतन Rope_Piece_BTree_पर्णम्(); }

RopePieceBTree::RopePieceBTree(ध्रुव RopePieceBTree &RHS) {
  प्रतिज्ञा(RHS.रिक्तानि() && "Can't copy non-empty tree yet");
  मूलम्_2 = नूतन Rope_Piece_BTree_पर्णम्();
}

RopePieceBTree::~RopePieceBTree() { get_मूलम्(मूलम्_2)->Destroy(); }

अचिह्नित RopePieceBTree::आकार_2() ध्रुव { प्रत्यागम get_मूलम्(मूलम्_2)->आकार_2(); }

शून्य RopePieceBTree::clear() {
  यदि (स्वचालित *पर्णम् = dyn_cast<Rope_Piece_BTree_पर्णम्>(get_मूलम्(मूलम्_2)))
    पर्णम्->clear();
  अन्यथा {
    get_मूलम्(मूलम्_2)->Destroy();
    मूलम्_2 = नूतन Rope_Piece_BTree_पर्णम्();
  }
}

शून्य RopePieceBTree::निवेशय(अचिह्नित विस्थापनम्, ध्रुव RopePiece &R) {
  // #1. Split at Offset.
  यदि (Rope_Piece_BTree_ग्रन्थिः *RHS = get_मूलम्(मूलम्_2)->विभज(विस्थापनम्))
    मूलम्_2 = नूतन RopePieceBTreeInterior(get_मूलम्(मूलम्_2), RHS);

  // #2. Do the insertion.
  यदि (Rope_Piece_BTree_ग्रन्थिः *RHS = get_मूलम्(मूलम्_2)->निवेशय(विस्थापनम्, R))
    मूलम्_2 = नूतन RopePieceBTreeInterior(get_मूलम्(मूलम्_2), RHS);
}

शून्य RopePieceBTree::erase(अचिह्नित विस्थापनम्, अचिह्नित अङ्कः_अष्टकानि) {
  // #1. Split at Offset.
  यदि (Rope_Piece_BTree_ग्रन्थिः *RHS = get_मूलम्(मूलम्_2)->विभज(विस्थापनम्))
    मूलम्_2 = नूतन RopePieceBTreeInterior(get_मूलम्(मूलम्_2), RHS);

  // #2. Do the erasing.
  get_मूलम्(मूलम्_2)->erase(विस्थापनम्, अङ्कः_अष्टकानि);
}

//===----------------------------------------------------------------------===//
// RewriteRope Implementation
//===----------------------------------------------------------------------===//

/// MakeRopeString - This copies the specified byte range into some instance of
/// RopeRefCountString, and return a RopePiece that represents it.  This uses
/// the AllocBuffer object to aggregate requests for small strings into one
/// allocation instead of doing tons of tiny allocations.
RopePiece RewriteRope::MakeRopeString(ध्रुव वर्ण *Start, ध्रुव वर्ण *अन्तः_2) {
  अचिह्नित Len = अन्तः_2 - Start;
  प्रतिज्ञा(Len && "Zero length RopePiece is invalid!");

  // If we have space for this string in the current alloc buffer, use it.
  यदि (AllocOffs + Len <= Alloc_Chunk_आकार) {
    स्मृतिं_प्रतिलिख(Alloc_बफर->दत्त_2 + AllocOffs, Start, Len);
    AllocOffs += Len;
    प्रत्यागम RopePiece(Alloc_बफर, AllocOffs - Len, AllocOffs);
  }

  // If we don't have enough room because this specific allocation is huge,
  // just allocate a new rope piece for it alone.
  यदि (Len > Alloc_Chunk_आकार) {
    अचिह्नित आकार = अन्तः_2 - Start + परिमाण(Rope_Ref_गणनम्_String) - 1;
    स्वचालित *Res = reinterpret_cast<Rope_Ref_गणनम्_String *>(नूतन वर्ण[आकार]);
    Res->Ref_गणनम् = 0;
    स्मृतिं_प्रतिलिख(Res->दत्त_2, Start, अन्तः_2 - Start);
    प्रत्यागम RopePiece(Res, 0, अन्तः_2 - Start);
  }

  // Otherwise, this was a small request but we just don't have space for it
  // Make a new chunk and share it with later allocations.

  अचिह्नित Alloc_आकार = offsetof(Rope_Ref_गणनम्_String, दत्त_2) + Alloc_Chunk_आकार;
  स्वचालित *Res = reinterpret_cast<Rope_Ref_गणनम्_String *>(नूतन वर्ण[Alloc_आकार]);
  Res->Ref_गणनम् = 0;
  स्मृतिं_प्रतिलिख(Res->दत्त_2, Start, Len);
  Alloc_बफर = Res;
  AllocOffs = Len;

  प्रत्यागम RopePiece(Alloc_बफर, 0, Len);
}
