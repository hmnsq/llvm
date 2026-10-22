//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/ADT/RewriteBuffer.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"

using நாமாகாஶ llvm;

raw_ostream &RewriteBuffer::லிக(raw_ostream &Stream) மாறிலி {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  க்கு (RopePieceBTreeIterator I = begin(), E = end(); I != E;
       I.MoveToNextPiece())
    Stream << I.piece();
  திரும்பு Stream;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
நிலையான உள்வரி தருக்க isWhitespaceExceptNL(unsigned எழுத்து c) {
  திரும்பு c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

வெறுமை RewriteBuffer::RemoveText(unsigned OrigOffset, unsigned Size,
                               தருக்க remove_வரி_If_Empty) {
  // Nothing to remove, exit early.
  எனில் (Size == 0)
    திரும்பு;

  unsigned RealOffset = getMappedOffset(OrigOffset, உண்மை);
  ப்ரதிஜ்ஞா(RealOffset + Size <= Buffer.size() && "Invalid location");

  // Remove the dead characters.
  Buffer.erase(RealOffset, Size);

  // Add a delta so that future changes are offset correctly.
  AddReplaceDelta(OrigOffset, -Size);

  எனில் (remove_வரி_If_Empty) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_வரி_Start = begin();
    unsigned cur_வரி_Start_Offs = 0;
    iterator posI = begin();
    க்கு (unsigned i = 0; i != RealOffset; ++i) {
      எனில் (*posI == '\n') {
        cur_வரி_Start = posI;
        ++cur_வரி_Start;
        cur_வரி_Start_Offs = i + 1;
      }
      ++posI;
    }

    unsigned வரி_Size = 0;
    posI = cur_வரி_Start;
    போது (posI != end() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++வரி_Size;
    }
    எனில் (posI != end() && *posI == '\n') {
      Buffer.erase(cur_வரி_Start_Offs, வரி_Size + 1 /* + '\n'*/);
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
      AddReplaceDelta(cur_வரி_Start_Offs, -(வரி_Size + 1 /* + '\n'*/));
    }
  }
}

வெறுமை RewriteBuffer::செருகு_Text_2(unsigned OrigOffset, StringRef Str,
                               தருக்க செருகு_After) {
  // Nothing to insert, exit early.
  எனில் (Str.empty())
    திரும்பு;

  unsigned RealOffset = getMappedOffset(OrigOffset, செருகு_After);
  Buffer.செருகு(RealOffset, Str.begin(), Str.end());

  // Add a delta so that future changes are offset correctly.
  Add_செருகு_Delta(OrigOffset, Str.size());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
வெறுமை RewriteBuffer::ReplaceText(unsigned OrigOffset, unsigned OrigLength,
                                StringRef NewStr) {
  unsigned RealOffset = getMappedOffset(OrigOffset, உண்மை);
  Buffer.erase(RealOffset, OrigLength);
  Buffer.செருகு(RealOffset, NewStr.begin(), NewStr.end());
  எனில் (OrigLength != NewStr.size())
    AddReplaceDelta(OrigOffset, NewStr.size() - OrigLength);
}
