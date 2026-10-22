//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/RewriteBuffer.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"

using ନାମାକାଶ llvm;

raw_ostream &RewriteBuffer::ଲିଖ(raw_ostream &Stream) ଅଚଳ {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  ପାଇଁ (RopePieceBTreeIterator I = begin(), E = end(); I != E;
       I.MoveToNextPiece())
    Stream << I.piece();
  ଫେରନ୍ତୁ Stream;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ତାର୍କିକ isWhitespaceExceptNL(unsigned ଅକ୍ଷର c) {
  ଫେରନ୍ତୁ c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

ଶୂନ୍ୟ RewriteBuffer::RemoveText(unsigned OrigOffset, unsigned Size,
                               ତାର୍କିକ remove_ଧାଡ଼ି_If_Empty) {
  // Nothing to remove, exit early.
  ଯଦି (Size == 0)
    ଫେରନ୍ତୁ;

  unsigned RealOffset = getMappedOffset(OrigOffset, ସତ୍ୟ);
  ପ୍ରତିଜ୍ଞା(RealOffset + Size <= Buffer.size() && "Invalid location");

  // Remove the dead characters.
  Buffer.erase(RealOffset, Size);

  // Add a delta so that future changes are offset correctly.
  AddReplaceDelta(OrigOffset, -Size);

  ଯଦି (remove_ଧାଡ଼ି_If_Empty) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_ଧାଡ଼ି_Start = begin();
    unsigned cur_ଧାଡ଼ି_Start_Offs = 0;
    iterator posI = begin();
    ପାଇଁ (unsigned i = 0; i != RealOffset; ++i) {
      ଯଦି (*posI == '\n') {
        cur_ଧାଡ଼ି_Start = posI;
        ++cur_ଧାଡ଼ି_Start;
        cur_ଧାଡ଼ି_Start_Offs = i + 1;
      }
      ++posI;
    }

    unsigned ଧାଡ଼ି_Size = 0;
    posI = cur_ଧାଡ଼ି_Start;
    ଯେତେବେଳେ (posI != end() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++ଧାଡ଼ି_Size;
    }
    ଯଦି (posI != end() && *posI == '\n') {
      Buffer.erase(cur_ଧାଡ଼ି_Start_Offs, ଧାଡ଼ି_Size + 1 /* + '\n'*/);
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
      AddReplaceDelta(cur_ଧାଡ଼ି_Start_Offs, -(ଧାଡ଼ି_Size + 1 /* + '\n'*/));
    }
  }
}

ଶୂନ୍ୟ RewriteBuffer::ଭର୍ତ୍ତି_Text_2(unsigned OrigOffset, StringRef Str,
                               ତାର୍କିକ ଭର୍ତ୍ତି_After) {
  // Nothing to insert, exit early.
  ଯଦି (Str.empty())
    ଫେରନ୍ତୁ;

  unsigned RealOffset = getMappedOffset(OrigOffset, ଭର୍ତ୍ତି_After);
  Buffer.ଭର୍ତ୍ତି(RealOffset, Str.begin(), Str.end());

  // Add a delta so that future changes are offset correctly.
  Add_ଭର୍ତ୍ତି_Delta(OrigOffset, Str.size());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
ଶୂନ୍ୟ RewriteBuffer::ReplaceText(unsigned OrigOffset, unsigned OrigLength,
                                StringRef NewStr) {
  unsigned RealOffset = getMappedOffset(OrigOffset, ସତ୍ୟ);
  Buffer.erase(RealOffset, OrigLength);
  Buffer.ଭର୍ତ୍ତି(RealOffset, NewStr.begin(), NewStr.end());
  ଯଦି (OrigLength != NewStr.size())
    AddReplaceDelta(OrigOffset, NewStr.size() - OrigLength);
}
