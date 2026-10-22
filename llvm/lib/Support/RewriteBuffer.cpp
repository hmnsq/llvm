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

raw_ostream &RewriteBuffer::লিখ(raw_ostream &Stream) ধ্রুব {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  প্রতি (RopePieceBTreeIterator I = begin(), E = end(); I != E;
       I.MoveToNextPiece())
    Stream << I.piece();
  প্রত্যাগম Stream;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
স্থির অন্তঃস্থ তার্কিক isWhitespaceExceptNL(unsigned বর্ণ c) {
  প্রত্যাগম c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

শূন্য RewriteBuffer::RemoveText(unsigned OrigOffset, unsigned Size,
                               তার্কিক remove_সারি_If_Empty) {
  // Nothing to remove, exit early.
  যদি (Size == 0)
    প্রত্যাগম;

  unsigned RealOffset = getMappedOffset(OrigOffset, সত্য);
  প্রতিজ্ঞা(RealOffset + Size <= Buffer.size() && "Invalid location");

  // Remove the dead characters.
  Buffer.erase(RealOffset, Size);

  // Add a delta so that future changes are offset correctly.
  AddReplaceDelta(OrigOffset, -Size);

  যদি (remove_সারি_If_Empty) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_সারি_Start = begin();
    unsigned cur_সারি_Start_Offs = 0;
    iterator posI = begin();
    প্রতি (unsigned i = 0; i != RealOffset; ++i) {
      যদি (*posI == '\n') {
        cur_সারি_Start = posI;
        ++cur_সারি_Start;
        cur_সারি_Start_Offs = i + 1;
      }
      ++posI;
    }

    unsigned সারি_Size = 0;
    posI = cur_সারি_Start;
    যাবত্ (posI != end() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++সারি_Size;
    }
    যদি (posI != end() && *posI == '\n') {
      Buffer.erase(cur_সারি_Start_Offs, সারি_Size + 1 /* + '\n'*/);
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
      AddReplaceDelta(cur_সারি_Start_Offs, -(সারি_Size + 1 /* + '\n'*/));
    }
  }
}

শূন্য RewriteBuffer::ঢোকাও_Text_2(unsigned OrigOffset, StringRef Str,
                               তার্কিক ঢোকাও_After) {
  // Nothing to insert, exit early.
  যদি (Str.empty())
    প্রত্যাগম;

  unsigned RealOffset = getMappedOffset(OrigOffset, ঢোকাও_After);
  Buffer.ঢোকাও(RealOffset, Str.begin(), Str.end());

  // Add a delta so that future changes are offset correctly.
  Add_ঢোকাও_Delta(OrigOffset, Str.size());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
শূন্য RewriteBuffer::ReplaceText(unsigned OrigOffset, unsigned OrigLength,
                                StringRef NewStr) {
  unsigned RealOffset = getMappedOffset(OrigOffset, সত্য);
  Buffer.erase(RealOffset, OrigLength);
  Buffer.ঢোকাও(RealOffset, NewStr.begin(), NewStr.end());
  যদি (OrigLength != NewStr.size())
    AddReplaceDelta(OrigOffset, NewStr.size() - OrigLength);
}
