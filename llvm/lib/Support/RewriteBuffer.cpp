//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/ADT/RewriteBuffer.h"
#समावेश "llvm/Support/raw_ostream.h"

using namespace llvm;

raw_ostream &Rewrite_बफर::ग_लिखो(raw_ostream &Stream) स्थिर {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  क्रम (RopePieceBTreeIterator I = begin(), E = end(); I != E;
       I.Move_To_अग्र_Piece())
    Stream << I.piece();
  वापस Stream;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
अटल अंतरभूत bool isWhitespaceExceptNL(अचिन्हित अक्षर c) {
  वापस c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

व्योम Rewrite_बफर::RemoveText(अचिन्हित OrigOffset, अचिन्हित आकार,
                               bool remove_पंक्ति_If_Empty) {
  // Nothing to remove, exit early.
  अगर (आकार == 0)
    वापस;

  अचिन्हित RealOffset = getMappedOffset(OrigOffset, true);
  निश्चित(RealOffset + आकार <= बफर.आकार_2() && "Invalid location");

  // Remove the dead characters.
  बफर.erase(RealOffset, आकार);

  // Add a delta so that future changes are offset correctly.
  AddReplaceDelta(OrigOffset, -आकार);

  अगर (remove_पंक्ति_If_Empty) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_पंक्ति_Start = begin();
    अचिन्हित cur_पंक्ति_Start_Offs = 0;
    iterator posI = begin();
    क्रम (अचिन्हित i = 0; i != RealOffset; ++i) {
      अगर (*posI == '\n') {
        cur_पंक्ति_Start = posI;
        ++cur_पंक्ति_Start;
        cur_पंक्ति_Start_Offs = i + 1;
      }
      ++posI;
    }

    अचिन्हित पंक्ति_आकार = 0;
    posI = cur_पंक्ति_Start;
    जबतक (posI != end() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++पंक्ति_आकार;
    }
    अगर (posI != end() && *posI == '\n') {
      बफर.erase(cur_पंक्ति_Start_Offs, पंक्ति_आकार + 1 /* + '\n'*/);
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
      AddReplaceDelta(cur_पंक्ति_Start_Offs, -(पंक्ति_आकार + 1 /* + '\n'*/));
    }
  }
}

व्योम Rewrite_बफर::जोड़ो_Text_2(अचिन्हित OrigOffset, StringRef Str,
                               bool जोड़ो_After) {
  // Nothing to insert, exit early.
  अगर (Str.empty())
    वापस;

  अचिन्हित RealOffset = getMappedOffset(OrigOffset, जोड़ो_After);
  बफर.जोड़ो(RealOffset, Str.begin(), Str.end());

  // Add a delta so that future changes are offset correctly.
  Add_जोड़ो_Delta(OrigOffset, Str.आकार_2());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
व्योम Rewrite_बफर::ReplaceText(अचिन्हित OrigOffset, अचिन्हित Orig_लंबाई,
                                StringRef NewStr) {
  अचिन्हित RealOffset = getMappedOffset(OrigOffset, true);
  बफर.erase(RealOffset, Orig_लंबाई);
  बफर.जोड़ो(RealOffset, NewStr.begin(), NewStr.end());
  अगर (Orig_लंबाई != NewStr.आकार_2())
    AddReplaceDelta(OrigOffset, NewStr.आकार_2() - Orig_लंबाई);
}
