//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/RewriteBuffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

raw_ostream &Rewrite_ਬਫਰ::ਲਿਖ(raw_ostream &ਧਾਰਾ) ਅਚਲ {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  ਲਈ (RopePieceBTreeIterator I = begin(), E = ਸਿਰਾ(); I != E;
       I.Move_ਮੰਜ਼ਿਲ_ਅਗਲਾ_Piece())
    ਧਾਰਾ << I.piece();
  ਵਾਪਸ ਧਾਰਾ;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
ਸਥਿਰ ਸਤਰਅੰਦਰ ਤਾਰਕਿਕ isWhitespaceExceptNL(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ c) {
  ਵਾਪਸ c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

ਖਾਲੀ Rewrite_ਬਫਰ::Remove_ਪਾਠ(ਬਿਨਾਨਿਸ਼ਾਨੀ Orig_ਵਿੱਥ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ,
                               ਤਾਰਕਿਕ remove_ਸਤਰ_If_ਖਾਲੀ_ਥਾਵਾਂ) {
  // Nothing to remove, exit early.
  ਜੇ (ਮਾਪ == 0)
    ਵਾਪਸ;

  ਬਿਨਾਨਿਸ਼ਾਨੀ Real_ਵਿੱਥ = get_Mapped_ਵਿੱਥ(Orig_ਵਿੱਥ, ਸੱਚ);
  ਪ੍ਰਤਿਜ੍ਞਾ(Real_ਵਿੱਥ + ਮਾਪ <= ਬਫਰ.ਮਾਪ_2() && "Invalid location");

  // Remove the dead characters.
  ਬਫਰ.erase(Real_ਵਿੱਥ, ਮਾਪ);

  // Add a delta so that future changes are offset correctly.
  ਜੋੜੋ_Replace_Delta(Orig_ਵਿੱਥ, -ਮਾਪ);

  ਜੇ (remove_ਸਤਰ_If_ਖਾਲੀ_ਥਾਵਾਂ) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_ਸਤਰ_ਸ਼ੁਰੂ = begin();
    ਬਿਨਾਨਿਸ਼ਾਨੀ cur_ਸਤਰ_ਸ਼ੁਰੂ_Offs = 0;
    iterator posI = begin();
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0; i != Real_ਵਿੱਥ; ++i) {
      ਜੇ (*posI == '\n') {
        cur_ਸਤਰ_ਸ਼ੁਰੂ = posI;
        ++cur_ਸਤਰ_ਸ਼ੁਰੂ;
        cur_ਸਤਰ_ਸ਼ੁਰੂ_Offs = i + 1;
      }
      ++posI;
    }

    ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_ਮਾਪ = 0;
    posI = cur_ਸਤਰ_ਸ਼ੁਰੂ;
    ਜਦੋਂ (posI != ਸਿਰਾ() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++ਸਤਰ_ਮਾਪ;
    }
    ਜੇ (posI != ਸਿਰਾ() && *posI == '\n') {
      ਬਫਰ.erase(cur_ਸਤਰ_ਸ਼ੁਰੂ_Offs, ਸਤਰ_ਮਾਪ + 1 /* + '\n'*/);
      // FIXME: Here, the offset of the start of the line is supposed to be
      // expressed in terms of the original input not the "real" rewrite
      // buffer.  How do we compute that reliably?  It might be tempting to use
      // curLineStartOffs + OrigOffset - RealOffset, but that assumes the
      // difference between the original and real offset is the same at the
      // removed text and at the start of the line, but that's not true if
      // edits were previously made earlier on the line.  This bug is also
      // documented by a FIXME on the definition of
      // clang::Rewriter::RewriteOptions::RemoveLineIfEmpty.  A reproducer for
      // the implementation below is the test RemoveLineIfEmpty in
      // clang/unittests/Rewrite/RewriteBufferTest.cpp.
      ਜੋੜੋ_Replace_Delta(cur_ਸਤਰ_ਸ਼ੁਰੂ_Offs, -(ਸਤਰ_ਮਾਪ + 1 /* + '\n'*/));
    }
  }
}

ਖਾਲੀ Rewrite_ਬਫਰ::ਪਾਓ_ਪਾਠ_2(ਬਿਨਾਨਿਸ਼ਾਨੀ Orig_ਵਿੱਥ, StringRef Str,
                               ਤਾਰਕਿਕ ਪਾਓ_After) {
  // Nothing to insert, exit early.
  ਜੇ (Str.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ;

  ਬਿਨਾਨਿਸ਼ਾਨੀ Real_ਵਿੱਥ = get_Mapped_ਵਿੱਥ(Orig_ਵਿੱਥ, ਪਾਓ_After);
  ਬਫਰ.ਪਾਓ(Real_ਵਿੱਥ, Str.begin(), Str.ਸਿਰਾ());

  // Add a delta so that future changes are offset correctly.
  ਜੋੜੋ_ਪਾਓ_Delta(Orig_ਵਿੱਥ, Str.ਮਾਪ_2());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
ਖਾਲੀ Rewrite_ਬਫਰ::Replace_ਪਾਠ(ਬਿਨਾਨਿਸ਼ਾਨੀ Orig_ਵਿੱਥ, ਬਿਨਾਨਿਸ਼ਾਨੀ Orig_ਲੰਮਾਈ,
                                StringRef NewStr) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ Real_ਵਿੱਥ = get_Mapped_ਵਿੱਥ(Orig_ਵਿੱਥ, ਸੱਚ);
  ਬਫਰ.erase(Real_ਵਿੱਥ, Orig_ਲੰਮਾਈ);
  ਬਫਰ.ਪਾਓ(Real_ਵਿੱਥ, NewStr.begin(), NewStr.ਸਿਰਾ());
  ਜੇ (Orig_ਲੰਮਾਈ != NewStr.ਮਾਪ_2())
    ਜੋੜੋ_Replace_Delta(Orig_ਵਿੱਥ, NewStr.ਮਾਪ_2() - Orig_ਲੰਮਾਈ);
}
