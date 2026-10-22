//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/ADT/RewriteBuffer.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"

using নামাকাশ llvm;

raw_ostream &Rewrite_বাফার::লিখ(raw_ostream &Stream) ধ্রুবক {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  জন্য (RopePieceBTreeIterator I = begin(), E = end(); I != E;
       I.Move_To_পরবর্তী_Piece())
    Stream << I.piece();
  ফেরত Stream;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
স্থির অন্তঃরেখ যৌক্তিক isWhitespaceExceptNL(unsigned অক্ষর c) {
  ফেরত c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

শূন্য Rewrite_বাফার::RemoveText(unsigned OrigOffset, unsigned আকার_2,
                               যৌক্তিক remove_সারি_If_Empty) {
  // Nothing to remove, exit early.
  যদি (আকার_2 == 0)
    ফেরত;

  unsigned RealOffset = getMappedOffset(OrigOffset, সত্য);
  প্রতিজ্ঞা(RealOffset + আকার_2 <= বাফার.আকার_3() && "Invalid location");

  // Remove the dead characters.
  বাফার.erase(RealOffset, আকার_2);

  // Add a delta so that future changes are offset correctly.
  AddReplaceDelta(OrigOffset, -আকার_2);

  যদি (remove_সারি_If_Empty) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_সারি_Start = begin();
    unsigned cur_সারি_Start_Offs = 0;
    iterator posI = begin();
    জন্য (unsigned i = 0; i != RealOffset; ++i) {
      যদি (*posI == '\n') {
        cur_সারি_Start = posI;
        ++cur_সারি_Start;
        cur_সারি_Start_Offs = i + 1;
      }
      ++posI;
    }

    unsigned সারি_আকার = 0;
    posI = cur_সারি_Start;
    যখন (posI != end() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++সারি_আকার;
    }
    যদি (posI != end() && *posI == '\n') {
      বাফার.erase(cur_সারি_Start_Offs, সারি_আকার + 1 /* + '\n'*/);
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
      AddReplaceDelta(cur_সারি_Start_Offs, -(সারি_আকার + 1 /* + '\n'*/));
    }
  }
}

শূন্য Rewrite_বাফার::ঢোকাও_Text_2(unsigned OrigOffset, StringRef Str,
                               যৌক্তিক ঢোকাও_After) {
  // Nothing to insert, exit early.
  যদি (Str.empty())
    ফেরত;

  unsigned RealOffset = getMappedOffset(OrigOffset, ঢোকাও_After);
  বাফার.ঢোকাও(RealOffset, Str.begin(), Str.end());

  // Add a delta so that future changes are offset correctly.
  Add_ঢোকাও_Delta(OrigOffset, Str.আকার_3());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
শূন্য Rewrite_বাফার::ReplaceText(unsigned OrigOffset, unsigned Orig_দৈর্ঘ্য,
                                StringRef NewStr) {
  unsigned RealOffset = getMappedOffset(OrigOffset, সত্য);
  বাফার.erase(RealOffset, Orig_দৈর্ঘ্য);
  বাফার.ঢোকাও(RealOffset, NewStr.begin(), NewStr.end());
  যদি (Orig_দৈর্ঘ্য != NewStr.আকার_3())
    AddReplaceDelta(OrigOffset, NewStr.আকার_3() - Orig_দৈর্ঘ্য);
}
