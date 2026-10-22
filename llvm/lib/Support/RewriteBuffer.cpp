//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/ADT/RewriteBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

raw_ostream &Rewrite_ಬಫರ್::ಲಿಖ(raw_ostream &ಹರಿವು) ಸ್ಥಿರಾಂಕ {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  ಗಾಗಿ (RopePieceBTreeIterator I = begin(), E = ಕೊನೆ(); I != E;
       I.Move_ಗಮ್ಯ_ಮುಂದಿನ_Piece())
    ಹರಿವು << I.piece();
  ಹಿಂತಿರುಗಿ ಹರಿವು;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ತಾರ್ಕಿಕ isWhitespaceExceptNL(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ c) {
  ಹಿಂತಿರುಗಿ c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

ಶೂನ್ಯ Rewrite_ಬಫರ್::Remove_ಪಠ್ಯ(ಚಿಹ್ನೆರಹಿತ Orig_ಸ್ಥಾನಾಂತರ, ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2,
                               ತಾರ್ಕಿಕ remove_ಸಾಲು_If_ಖಾಲಿಗಳು) {
  // Nothing to remove, exit early.
  ಆದರೆ (ಗಾತ್ರ_2 == 0)
    ಹಿಂತಿರುಗಿ;

  ಚಿಹ್ನೆರಹಿತ Real_ಸ್ಥಾನಾಂತರ = get_Mapped_ಸ್ಥಾನಾಂತರ(Orig_ಸ್ಥಾನಾಂತರ, ನಿಜ);
  ಪ್ರತಿಜ್ಞಾ(Real_ಸ್ಥಾನಾಂತರ + ಗಾತ್ರ_2 <= ಬಫರ್.ಗಾತ್ರ_3() && "Invalid location");

  // Remove the dead characters.
  ಬಫರ್.erase(Real_ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2);

  // Add a delta so that future changes are offset correctly.
  ಕೂಡಿಸು_Replace_Delta(Orig_ಸ್ಥಾನಾಂತರ, -ಗಾತ್ರ_2);

  ಆದರೆ (remove_ಸಾಲು_If_ಖಾಲಿಗಳು) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_ಸಾಲು_ಆರಂಭ = begin();
    ಚಿಹ್ನೆರಹಿತ cur_ಸಾಲು_ಆರಂಭ_Offs = 0;
    iterator posI = begin();
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0; i != Real_ಸ್ಥಾನಾಂತರ; ++i) {
      ಆದರೆ (*posI == '\n') {
        cur_ಸಾಲು_ಆರಂಭ = posI;
        ++cur_ಸಾಲು_ಆರಂಭ;
        cur_ಸಾಲು_ಆರಂಭ_Offs = i + 1;
      }
      ++posI;
    }

    ಚಿಹ್ನೆರಹಿತ ಸಾಲು_ಗಾತ್ರ = 0;
    posI = cur_ಸಾಲು_ಆರಂಭ;
    ಆಗ (posI != ಕೊನೆ() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++ಸಾಲು_ಗಾತ್ರ;
    }
    ಆದರೆ (posI != ಕೊನೆ() && *posI == '\n') {
      ಬಫರ್.erase(cur_ಸಾಲು_ಆರಂಭ_Offs, ಸಾಲು_ಗಾತ್ರ + 1 /* + '\n'*/);
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
      ಕೂಡಿಸು_Replace_Delta(cur_ಸಾಲು_ಆರಂಭ_Offs, -(ಸಾಲು_ಗಾತ್ರ + 1 /* + '\n'*/));
    }
  }
}

ಶೂನ್ಯ Rewrite_ಬಫರ್::ಸೇರಿಸು_ಪಠ್ಯ_2(ಚಿಹ್ನೆರಹಿತ Orig_ಸ್ಥಾನಾಂತರ, StringRef Str,
                               ತಾರ್ಕಿಕ ಸೇರಿಸು_After) {
  // Nothing to insert, exit early.
  ಆದರೆ (Str.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ;

  ಚಿಹ್ನೆರಹಿತ Real_ಸ್ಥಾನಾಂತರ = get_Mapped_ಸ್ಥಾನಾಂತರ(Orig_ಸ್ಥಾನಾಂತರ, ಸೇರಿಸು_After);
  ಬಫರ್.ಸೇರಿಸು(Real_ಸ್ಥಾನಾಂತರ, Str.begin(), Str.ಕೊನೆ());

  // Add a delta so that future changes are offset correctly.
  ಕೂಡಿಸು_ಸೇರಿಸು_Delta(Orig_ಸ್ಥಾನಾಂತರ, Str.ಗಾತ್ರ_3());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
ಶೂನ್ಯ Rewrite_ಬಫರ್::Replace_ಪಠ್ಯ(ಚಿಹ್ನೆರಹಿತ Orig_ಸ್ಥಾನಾಂತರ, ಚಿಹ್ನೆರಹಿತ Orig_ಉದ್ದಳತೆ,
                                StringRef NewStr) {
  ಚಿಹ್ನೆರಹಿತ Real_ಸ್ಥಾನಾಂತರ = get_Mapped_ಸ್ಥಾನಾಂತರ(Orig_ಸ್ಥಾನಾಂತರ, ನಿಜ);
  ಬಫರ್.erase(Real_ಸ್ಥಾನಾಂತರ, Orig_ಉದ್ದಳತೆ);
  ಬಫರ್.ಸೇರಿಸು(Real_ಸ್ಥಾನಾಂತರ, NewStr.begin(), NewStr.ಕೊನೆ());
  ಆದರೆ (Orig_ಉದ್ದಳತೆ != NewStr.ಗಾತ್ರ_3())
    ಕೂಡಿಸು_Replace_Delta(Orig_ಸ್ಥಾನಾಂತರ, NewStr.ಗಾತ್ರ_3() - Orig_ಉದ್ದಳತೆ);
}
