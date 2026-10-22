//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/RewriteBuffer.h"
#ضمن "llvm/Support/raw_ostream.h"

using نطاق llvm;

raw_ostream &RewriteBuffer::اكتب(raw_ostream &Stream) ثابت {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  لكل (RopePieceBTreeIterator I = begin(), E = end(); I != E;
       I.MoveToNextPiece())
    Stream << I.piece();
  أرجع Stream;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
ساكن مضمن منطقي isWhitespaceExceptNL(unsigned حرف c) {
  أرجع c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

فراغ RewriteBuffer::RemoveText(unsigned OrigOffset, unsigned Size,
                               منطقي remove_سطر_If_Empty) {
  // Nothing to remove, exit early.
  إذا (Size == 0)
    أرجع;

  unsigned RealOffset = getMappedOffset(OrigOffset, true);
  أكد(RealOffset + Size <= Buffer.size() && "Invalid location");

  // Remove the dead characters.
  Buffer.erase(RealOffset, Size);

  // Add a delta so that future changes are offset correctly.
  AddReplaceDelta(OrigOffset, -Size);

  إذا (remove_سطر_If_Empty) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_سطر_Start = begin();
    unsigned cur_سطر_Start_Offs = 0;
    iterator posI = begin();
    لكل (unsigned i = 0; i != RealOffset; ++i) {
      إذا (*posI == '\n') {
        cur_سطر_Start = posI;
        ++cur_سطر_Start;
        cur_سطر_Start_Offs = i + 1;
      }
      ++posI;
    }

    unsigned سطر_Size = 0;
    posI = cur_سطر_Start;
    طالما (posI != end() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++سطر_Size;
    }
    إذا (posI != end() && *posI == '\n') {
      Buffer.erase(cur_سطر_Start_Offs, سطر_Size + 1 /* + '\n'*/);
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
      AddReplaceDelta(cur_سطر_Start_Offs, -(سطر_Size + 1 /* + '\n'*/));
    }
  }
}

فراغ RewriteBuffer::أدرج_Text_2(unsigned OrigOffset, StringRef Str,
                               منطقي أدرج_After) {
  // Nothing to insert, exit early.
  إذا (Str.empty())
    أرجع;

  unsigned RealOffset = getMappedOffset(OrigOffset, أدرج_After);
  Buffer.أدرج(RealOffset, Str.begin(), Str.end());

  // Add a delta so that future changes are offset correctly.
  Add_أدرج_Delta(OrigOffset, Str.size());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
فراغ RewriteBuffer::ReplaceText(unsigned OrigOffset, unsigned OrigLength,
                                StringRef NewStr) {
  unsigned RealOffset = getMappedOffset(OrigOffset, true);
  Buffer.erase(RealOffset, OrigLength);
  Buffer.أدرج(RealOffset, NewStr.begin(), NewStr.end());
  إذا (OrigLength != NewStr.size())
    AddReplaceDelta(OrigOffset, NewStr.size() - OrigLength);
}
