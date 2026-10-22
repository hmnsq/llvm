//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/ADT/RewriteBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"

using ನಾಮಾಕಾಶ llvm;

raw_ostream &RewriteBuffer::ಲಿಖ(raw_ostream &Stream) ಧ್ರುವ {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  ಪ್ರತಿ (RopePieceBTreeIterator I = begin(), E = end(); I != E;
       I.MoveToNextPiece())
    Stream << I.piece();
  ಪ್ರತ್ಯಾಗಮ Stream;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ತಾರ್ಕಿಕ isWhitespaceExceptNL(unsigned ವರ್ಣ c) {
  ಪ್ರತ್ಯಾಗಮ c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

ಶೂನ್ಯ RewriteBuffer::RemoveText(unsigned OrigOffset, unsigned Size,
                               ತಾರ್ಕಿಕ remove_ಸಾಲು_If_Empty) {
  // Nothing to remove, exit early.
  ಯದಿ (Size == 0)
    ಪ್ರತ್ಯಾಗಮ;

  unsigned RealOffset = getMappedOffset(OrigOffset, ಸತ್ಯ);
  ಪ್ರತಿಜ್ಞಾ(RealOffset + Size <= Buffer.size() && "Invalid location");

  // Remove the dead characters.
  Buffer.erase(RealOffset, Size);

  // Add a delta so that future changes are offset correctly.
  AddReplaceDelta(OrigOffset, -Size);

  ಯದಿ (remove_ಸಾಲು_If_Empty) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_ಸಾಲು_Start = begin();
    unsigned cur_ಸಾಲು_Start_Offs = 0;
    iterator posI = begin();
    ಪ್ರತಿ (unsigned i = 0; i != RealOffset; ++i) {
      ಯದಿ (*posI == '\n') {
        cur_ಸಾಲು_Start = posI;
        ++cur_ಸಾಲು_Start;
        cur_ಸಾಲು_Start_Offs = i + 1;
      }
      ++posI;
    }

    unsigned ಸಾಲು_Size = 0;
    posI = cur_ಸಾಲು_Start;
    ಯಾವತ್ (posI != end() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++ಸಾಲು_Size;
    }
    ಯದಿ (posI != end() && *posI == '\n') {
      Buffer.erase(cur_ಸಾಲು_Start_Offs, ಸಾಲು_Size + 1 /* + '\n'*/);
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
      AddReplaceDelta(cur_ಸಾಲು_Start_Offs, -(ಸಾಲು_Size + 1 /* + '\n'*/));
    }
  }
}

ಶೂನ್ಯ RewriteBuffer::ಸೇರಿಸು_Text_2(unsigned OrigOffset, StringRef Str,
                               ತಾರ್ಕಿಕ ಸೇರಿಸು_After) {
  // Nothing to insert, exit early.
  ಯದಿ (Str.empty())
    ಪ್ರತ್ಯಾಗಮ;

  unsigned RealOffset = getMappedOffset(OrigOffset, ಸೇರಿಸು_After);
  Buffer.ಸೇರಿಸು(RealOffset, Str.begin(), Str.end());

  // Add a delta so that future changes are offset correctly.
  Add_ಸೇರಿಸು_Delta(OrigOffset, Str.size());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
ಶೂನ್ಯ RewriteBuffer::ReplaceText(unsigned OrigOffset, unsigned OrigLength,
                                StringRef NewStr) {
  unsigned RealOffset = getMappedOffset(OrigOffset, ಸತ್ಯ);
  Buffer.erase(RealOffset, OrigLength);
  Buffer.ಸೇರಿಸು(RealOffset, NewStr.begin(), NewStr.end());
  ಯದಿ (OrigLength != NewStr.size())
    AddReplaceDelta(OrigOffset, NewStr.size() - OrigLength);
}
