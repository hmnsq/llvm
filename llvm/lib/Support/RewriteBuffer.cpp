//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/ADT/RewriteBuffer.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"

using നാമാകാശ llvm;

raw_ostream &RewriteBuffer::ലിഖ(raw_ostream &Stream) സ്ഥിരം {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  വേണ്ടി (RopePieceBTreeIterator I = begin(), E = end(); I != E;
       I.MoveToNextPiece())
    Stream << I.piece();
  തിരികെ Stream;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
സ്ഥിതം വരിയിൽ തർക്കം isWhitespaceExceptNL(unsigned അക്ഷരം c) {
  തിരികെ c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

ശൂന്യം RewriteBuffer::RemoveText(unsigned OrigOffset, unsigned Size,
                               തർക്കം remove_വരി_If_Empty) {
  // Nothing to remove, exit early.
  എങ്കിൽ (Size == 0)
    തിരികെ;

  unsigned RealOffset = getMappedOffset(OrigOffset, സത്യം);
  പ്രതിജ്ഞാ(RealOffset + Size <= Buffer.size() && "Invalid location");

  // Remove the dead characters.
  Buffer.erase(RealOffset, Size);

  // Add a delta so that future changes are offset correctly.
  AddReplaceDelta(OrigOffset, -Size);

  എങ്കിൽ (remove_വരി_If_Empty) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_വരി_Start = begin();
    unsigned cur_വരി_Start_Offs = 0;
    iterator posI = begin();
    വേണ്ടി (unsigned i = 0; i != RealOffset; ++i) {
      എങ്കിൽ (*posI == '\n') {
        cur_വരി_Start = posI;
        ++cur_വരി_Start;
        cur_വരി_Start_Offs = i + 1;
      }
      ++posI;
    }

    unsigned വരി_Size = 0;
    posI = cur_വരി_Start;
    ആകുമ്പോൾ (posI != end() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++വരി_Size;
    }
    എങ്കിൽ (posI != end() && *posI == '\n') {
      Buffer.erase(cur_വരി_Start_Offs, വരി_Size + 1 /* + '\n'*/);
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
      AddReplaceDelta(cur_വരി_Start_Offs, -(വരി_Size + 1 /* + '\n'*/));
    }
  }
}

ശൂന്യം RewriteBuffer::തിരുകുക_Text_2(unsigned OrigOffset, StringRef Str,
                               തർക്കം തിരുകുക_After) {
  // Nothing to insert, exit early.
  എങ്കിൽ (Str.empty())
    തിരികെ;

  unsigned RealOffset = getMappedOffset(OrigOffset, തിരുകുക_After);
  Buffer.തിരുകുക(RealOffset, Str.begin(), Str.end());

  // Add a delta so that future changes are offset correctly.
  Add_തിരുകുക_Delta(OrigOffset, Str.size());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
ശൂന്യം RewriteBuffer::ReplaceText(unsigned OrigOffset, unsigned OrigLength,
                                StringRef NewStr) {
  unsigned RealOffset = getMappedOffset(OrigOffset, സത്യം);
  Buffer.erase(RealOffset, OrigLength);
  Buffer.തിരുകുക(RealOffset, NewStr.begin(), NewStr.end());
  എങ്കിൽ (OrigLength != NewStr.size())
    AddReplaceDelta(OrigOffset, NewStr.size() - OrigLength);
}
