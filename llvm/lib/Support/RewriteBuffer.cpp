//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/RewriteBuffer.h"
#ضمن "llvm/Support/raw_ostream.h"

استفاده نطاق llvm;

raw_ostream &Rewrite_میانگیر::اكتب(raw_ostream &جریان) ثابت {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  لكل (RopePieceBTreeIterator I = begin(), E = انتها(); I != E;
       I.Move_مقصد_بعدی_Piece())
    جریان << I.piece();
  أرجع جریان;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
ساكن مضمن منطقي isWhitespaceExceptNL(بیعلامت حرف c) {
  أرجع c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

فراغ Rewrite_میانگیر::Remove_متن(بیعلامت Orig_جابه_جایی, بیعلامت اندازه,
                               منطقي remove_سطر_If_خالی) {
  // Nothing to remove, exit early.
  إذا (اندازه == 0)
    أرجع;

  بیعلامت Real_جابه_جایی = get_Mapped_جابه_جایی(Orig_جابه_جایی, درست);
  أكد(Real_جابه_جایی + اندازه <= میانگیر.اندازه_2() && "Invalid location");

  // Remove the dead characters.
  میانگیر.erase(Real_جابه_جایی, اندازه);

  // Add a delta so that future changes are offset correctly.
  جمع_کن_Replace_Delta(Orig_جابه_جایی, -اندازه);

  إذا (remove_سطر_If_خالی) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_سطر_شروع = begin();
    بیعلامت cur_سطر_شروع_Offs = 0;
    iterator posI = begin();
    لكل (بیعلامت i = 0; i != Real_جابه_جایی; ++i) {
      إذا (*posI == '\n') {
        cur_سطر_شروع = posI;
        ++cur_سطر_شروع;
        cur_سطر_شروع_Offs = i + 1;
      }
      ++posI;
    }

    بیعلامت سطر_اندازه = 0;
    posI = cur_سطر_شروع;
    طالما (posI != انتها() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++سطر_اندازه;
    }
    إذا (posI != انتها() && *posI == '\n') {
      میانگیر.erase(cur_سطر_شروع_Offs, سطر_اندازه + 1 /* + '\n'*/);
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
      جمع_کن_Replace_Delta(cur_سطر_شروع_Offs, -(سطر_اندازه + 1 /* + '\n'*/));
    }
  }
}

فراغ Rewrite_میانگیر::درج_متن_2(بیعلامت Orig_جابه_جایی, StringRef Str,
                               منطقي درج_After) {
  // Nothing to insert, exit early.
  إذا (Str.خالی())
    أرجع;

  بیعلامت Real_جابه_جایی = get_Mapped_جابه_جایی(Orig_جابه_جایی, درج_After);
  میانگیر.درج(Real_جابه_جایی, Str.begin(), Str.انتها());

  // Add a delta so that future changes are offset correctly.
  جمع_کن_درج_Delta(Orig_جابه_جایی, Str.اندازه_2());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
فراغ Rewrite_میانگیر::Replace_متن(بیعلامت Orig_جابه_جایی, بیعلامت Orig_درازی,
                                StringRef NewStr) {
  بیعلامت Real_جابه_جایی = get_Mapped_جابه_جایی(Orig_جابه_جایی, درست);
  میانگیر.erase(Real_جابه_جایی, Orig_درازی);
  میانگیر.درج(Real_جابه_جایی, NewStr.begin(), NewStr.انتها());
  إذا (Orig_درازی != NewStr.اندازه_2())
    جمع_کن_Replace_Delta(Orig_جابه_جایی, NewStr.اندازه_2() - Orig_درازی);
}
