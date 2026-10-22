//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/RewriteBuffer.h"
#ضمن "llvm/Support/raw_ostream.h"

کارول نطاق llvm;

raw_ostream &Rewrite_بفر::اكتب(raw_ostream &بهیر) ثابت {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  لكل (RopePieceBTreeIterator I = begin(), E = پای_ټکی(); I != E;
       I.Move_منزل_بل_Piece())
    بهیر << I.piece();
  أرجع بهیر;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
ساكن مضمن منطقي isWhitespaceExceptNL(بینښه حرف c) {
  أرجع c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

فراغ Rewrite_بفر::Remove_متن(بینښه Orig_دننه_واټن, بینښه اندازه,
                               منطقي remove_کرښه_If_تشه) {
  // Nothing to remove, exit early.
  إذا (اندازه == 0)
    أرجع;

  بینښه Real_دننه_واټن = get_Mapped_دننه_واټن(Orig_دننه_واټن, ریښتیا);
  أكد(Real_دننه_واټن + اندازه <= بفر.اندازه_2() && "Invalid location");

  // Remove the dead characters.
  بفر.erase(Real_دننه_واټن, اندازه);

  // Add a delta so that future changes are offset correctly.
  جمع_کړه_Replace_Delta(Orig_دننه_واټن, -اندازه);

  إذا (remove_کرښه_If_تشه) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_کرښه_پیل = begin();
    بینښه cur_کرښه_پیل_Offs = 0;
    iterator posI = begin();
    لكل (بینښه i = 0; i != Real_دننه_واټن; ++i) {
      إذا (*posI == '\n') {
        cur_کرښه_پیل = posI;
        ++cur_کرښه_پیل;
        cur_کرښه_پیل_Offs = i + 1;
      }
      ++posI;
    }

    بینښه کرښه_اندازه = 0;
    posI = cur_کرښه_پیل;
    طالما (posI != پای_ټکی() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++کرښه_اندازه;
    }
    إذا (posI != پای_ټکی() && *posI == '\n') {
      بفر.erase(cur_کرښه_پیل_Offs, کرښه_اندازه + 1 /* + '\n'*/);
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
      جمع_کړه_Replace_Delta(cur_کرښه_پیل_Offs, -(کرښه_اندازه + 1 /* + '\n'*/));
    }
  }
}

فراغ Rewrite_بفر::ورننباسه_متن_2(بینښه Orig_دننه_واټن, StringRef Str,
                               منطقي ورننباسه_After) {
  // Nothing to insert, exit early.
  إذا (Str.تشه())
    أرجع;

  بینښه Real_دننه_واټن = get_Mapped_دننه_واټن(Orig_دننه_واټن, ورننباسه_After);
  بفر.ورننباسه(Real_دننه_واټن, Str.begin(), Str.پای_ټکی());

  // Add a delta so that future changes are offset correctly.
  جمع_کړه_ورننباسه_Delta(Orig_دننه_واټن, Str.اندازه_2());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
فراغ Rewrite_بفر::Replace_متن(بینښه Orig_دننه_واټن, بینښه Orig_اوږدتیا,
                                StringRef NewStr) {
  بینښه Real_دننه_واټن = get_Mapped_دننه_واټن(Orig_دننه_واټن, ریښتیا);
  بفر.erase(Real_دننه_واټن, Orig_اوږدتیا);
  بفر.ورننباسه(Real_دننه_واټن, NewStr.begin(), NewStr.پای_ټکی());
  إذا (Orig_اوږدتیا != NewStr.اندازه_2())
    جمع_کړه_Replace_Delta(Orig_دننه_واټن, NewStr.اندازه_2() - Orig_اوږدتیا);
}
