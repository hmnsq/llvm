//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/ADT/RewriteBuffer.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"

using નામાકાશ llvm;

raw_ostream &RewriteBuffer::લિખ(raw_ostream &Stream) અચલ {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  માટે (RopePieceBTreeIterator I = begin(), E = end(); I != E;
       I.MoveToNextPiece())
    Stream << I.piece();
  પાછા Stream;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
સ્થિર અંતઃરેખ તાર્કિક isWhitespaceExceptNL(unsigned અક્ષર c) {
  પાછા c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

શૂન્ય RewriteBuffer::RemoveText(unsigned OrigOffset, unsigned Size,
                               તાર્કિક remove_લીટી_If_Empty) {
  // Nothing to remove, exit early.
  જો (Size == 0)
    પાછા;

  unsigned RealOffset = getMappedOffset(OrigOffset, સાચું);
  પ્રતિજ્ઞા(RealOffset + Size <= Buffer.size() && "Invalid location");

  // Remove the dead characters.
  Buffer.erase(RealOffset, Size);

  // Add a delta so that future changes are offset correctly.
  AddReplaceDelta(OrigOffset, -Size);

  જો (remove_લીટી_If_Empty) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_લીટી_Start = begin();
    unsigned cur_લીટી_Start_Offs = 0;
    iterator posI = begin();
    માટે (unsigned i = 0; i != RealOffset; ++i) {
      જો (*posI == '\n') {
        cur_લીટી_Start = posI;
        ++cur_લીટી_Start;
        cur_લીટી_Start_Offs = i + 1;
      }
      ++posI;
    }

    unsigned લીટી_Size = 0;
    posI = cur_લીટી_Start;
    જ્યારે (posI != end() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++લીટી_Size;
    }
    જો (posI != end() && *posI == '\n') {
      Buffer.erase(cur_લીટી_Start_Offs, લીટી_Size + 1 /* + '\n'*/);
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
      AddReplaceDelta(cur_લીટી_Start_Offs, -(લીટી_Size + 1 /* + '\n'*/));
    }
  }
}

શૂન્ય RewriteBuffer::ઉમેરો_Text_2(unsigned OrigOffset, StringRef Str,
                               તાર્કિક ઉમેરો_After) {
  // Nothing to insert, exit early.
  જો (Str.empty())
    પાછા;

  unsigned RealOffset = getMappedOffset(OrigOffset, ઉમેરો_After);
  Buffer.ઉમેરો(RealOffset, Str.begin(), Str.end());

  // Add a delta so that future changes are offset correctly.
  Add_ઉમેરો_Delta(OrigOffset, Str.size());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
શૂન્ય RewriteBuffer::ReplaceText(unsigned OrigOffset, unsigned OrigLength,
                                StringRef NewStr) {
  unsigned RealOffset = getMappedOffset(OrigOffset, સાચું);
  Buffer.erase(RealOffset, OrigLength);
  Buffer.ઉમેરો(RealOffset, NewStr.begin(), NewStr.end());
  જો (OrigLength != NewStr.size())
    AddReplaceDelta(OrigOffset, NewStr.size() - OrigLength);
}
