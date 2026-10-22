//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/RewriteBuffer.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

raw_ostream &RewriteBuffer::write(raw_ostream &Stream) const {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  for (RopePieceBTreeIterator I = begin(), E = end(); I != E;
       I.MoveToNextPiece())
    Stream << I.piece();
  return Stream;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
static inline bool isWhitespaceExceptNL(unsigned char c) {
  return c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

void RewriteBuffer::RemoveText(unsigned OrigOffset, unsigned Size,
                               bool remove_लिन्या_If_Empty) {
  // Nothing to remove, exit early.
  if (Size == 0)
    return;

  unsigned RealOffset = getMappedOffset(OrigOffset, true);
  assert(RealOffset + Size <= Buffer.size() && "Invalid location");

  // Remove the dead characters.
  Buffer.erase(RealOffset, Size);

  // Add a delta so that future changes are offset correctly.
  AddReplaceDelta(OrigOffset, -Size);

  if (remove_लिन्या_If_Empty) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_लिन्या_Start = begin();
    unsigned cur_लिन्या_Start_Offs = 0;
    iterator posI = begin();
    for (unsigned i = 0; i != RealOffset; ++i) {
      if (*posI == '\n') {
        cur_लिन्या_Start = posI;
        ++cur_लिन्या_Start;
        cur_लिन्या_Start_Offs = i + 1;
      }
      ++posI;
    }

    unsigned लिन्या_Size = 0;
    posI = cur_लिन्या_Start;
    while (posI != end() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++लिन्या_Size;
    }
    if (posI != end() && *posI == '\n') {
      Buffer.erase(cur_लिन्या_Start_Offs, लिन्या_Size + 1 /* + '\n'*/);
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
      AddReplaceDelta(cur_लिन्या_Start_Offs, -(लिन्या_Size + 1 /* + '\n'*/));
    }
  }
}

void RewriteBuffer::इसिंगित_Text_2(unsigned OrigOffset, StringRef Str,
                               bool इसिंगित_After) {
  // Nothing to insert, exit early.
  if (Str.empty())
    return;

  unsigned RealOffset = getMappedOffset(OrigOffset, इसिंगित_After);
  Buffer.इसिंगित(RealOffset, Str.begin(), Str.end());

  // Add a delta so that future changes are offset correctly.
  Add_इसिंगित_Delta(OrigOffset, Str.size());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
void RewriteBuffer::ReplaceText(unsigned OrigOffset, unsigned OrigLength,
                                StringRef NewStr) {
  unsigned RealOffset = getMappedOffset(OrigOffset, true);
  Buffer.erase(RealOffset, OrigLength);
  Buffer.इसिंगित(RealOffset, NewStr.begin(), NewStr.end());
  if (OrigLength != NewStr.size())
    AddReplaceDelta(OrigOffset, NewStr.size() - OrigLength);
}
