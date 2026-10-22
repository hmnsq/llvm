//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/ADT/RewriteBuffer.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

using नामाकाश llvm;

raw_ostream &RewriteBuffer::लिख(raw_ostream &Stream) ध्रुव {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  प्रति (RopePieceBTreeIterator I = begin(), E = end(); I != E;
       I.MoveToNextPiece())
    Stream << I.piece();
  प्रत्यागम Stream;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
स्थिर अन्तःस्थ तार्किक isWhitespaceExceptNL(अचिन्हित वर्ण c) {
  प्रत्यागम c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

शून्य RewriteBuffer::RemoveText(अचिन्हित OrigOffset, अचिन्हित Size,
                               तार्किक remove_पंक्ति_If_Empty) {
  // Nothing to remove, exit early.
  यदि (Size == 0)
    प्रत्यागम;

  अचिन्हित RealOffset = getMappedOffset(OrigOffset, सत्य);
  प्रतिज्ञा(RealOffset + Size <= Buffer.size() && "Invalid location");

  // Remove the dead characters.
  Buffer.erase(RealOffset, Size);

  // Add a delta so that future changes are offset correctly.
  AddReplaceDelta(OrigOffset, -Size);

  यदि (remove_पंक्ति_If_Empty) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_पंक्ति_Start = begin();
    अचिन्हित cur_पंक्ति_Start_Offs = 0;
    iterator posI = begin();
    प्रति (अचिन्हित i = 0; i != RealOffset; ++i) {
      यदि (*posI == '\n') {
        cur_पंक्ति_Start = posI;
        ++cur_पंक्ति_Start;
        cur_पंक्ति_Start_Offs = i + 1;
      }
      ++posI;
    }

    अचिन्हित पंक्ति_Size = 0;
    posI = cur_पंक्ति_Start;
    यावत् (posI != end() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++पंक्ति_Size;
    }
    यदि (posI != end() && *posI == '\n') {
      Buffer.erase(cur_पंक्ति_Start_Offs, पंक्ति_Size + 1 /* + '\n'*/);
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
      AddReplaceDelta(cur_पंक्ति_Start_Offs, -(पंक्ति_Size + 1 /* + '\n'*/));
    }
  }
}

शून्य RewriteBuffer::जोड़ो_Text_2(अचिन्हित OrigOffset, StringRef Str,
                               तार्किक जोड़ो_After) {
  // Nothing to insert, exit early.
  यदि (Str.empty())
    प्रत्यागम;

  अचिन्हित RealOffset = getMappedOffset(OrigOffset, जोड़ो_After);
  Buffer.जोड़ो(RealOffset, Str.begin(), Str.end());

  // Add a delta so that future changes are offset correctly.
  Add_जोड़ो_Delta(OrigOffset, Str.size());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
शून्य RewriteBuffer::ReplaceText(अचिन्हित OrigOffset, अचिन्हित OrigLength,
                                StringRef NewStr) {
  अचिन्हित RealOffset = getMappedOffset(OrigOffset, सत्य);
  Buffer.erase(RealOffset, OrigLength);
  Buffer.जोड़ो(RealOffset, NewStr.begin(), NewStr.end());
  यदि (OrigLength != NewStr.size())
    AddReplaceDelta(OrigOffset, NewStr.size() - OrigLength);
}
