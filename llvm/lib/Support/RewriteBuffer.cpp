//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/RewriteBuffer.h"
#ضمن "llvm/Support/raw_ostream.h"

استعمال نطاق llvm;

raw_ostream &Rewrite_بفر::اكتب(raw_ostream &وهڪرو) ثابت {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  لكل (RopePieceBTreeIterator I = begin(), E = پڇاڙي(); I != E;
       I.Move_منزل_ايندڙ_Piece())
    وهڪرو << I.piece();
  أرجع وهڪرو;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
ساكن مضمن منطقي isWhitespaceExceptNL(بنانشان حرف c) {
  أرجع c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

فراغ Rewrite_بفر::Remove_متن(بنانشان Orig_وٿي, بنانشان سائيز,
                               منطقي remove_سٽ_If_خالي_جايون) {
  // Nothing to remove, exit early.
  إذا (سائيز == 0)
    أرجع;

  بنانشان Real_وٿي = get_Mapped_وٿي(Orig_وٿي, سچ);
  أكد(Real_وٿي + سائيز <= بفر.سائيز_2() && "Invalid location");

  // Remove the dead characters.
  بفر.erase(Real_وٿي, سائيز);

  // Add a delta so that future changes are offset correctly.
  جوڙيو_Replace_Delta(Orig_وٿي, -سائيز);

  إذا (remove_سٽ_If_خالي_جايون) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_سٽ_شروعات = begin();
    بنانشان cur_سٽ_شروعات_Offs = 0;
    iterator posI = begin();
    لكل (بنانشان i = 0; i != Real_وٿي; ++i) {
      إذا (*posI == '\n') {
        cur_سٽ_شروعات = posI;
        ++cur_سٽ_شروعات;
        cur_سٽ_شروعات_Offs = i + 1;
      }
      ++posI;
    }

    بنانشان سٽ_سائيز = 0;
    posI = cur_سٽ_شروعات;
    طالما (posI != پڇاڙي() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++سٽ_سائيز;
    }
    إذا (posI != پڇاڙي() && *posI == '\n') {
      بفر.erase(cur_سٽ_شروعات_Offs, سٽ_سائيز + 1 /* + '\n'*/);
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
      جوڙيو_Replace_Delta(cur_سٽ_شروعات_Offs, -(سٽ_سائيز + 1 /* + '\n'*/));
    }
  }
}

فراغ Rewrite_بفر::وجهو_متن_2(بنانشان Orig_وٿي, StringRef Str,
                               منطقي وجهو_After) {
  // Nothing to insert, exit early.
  إذا (Str.خالي_جايون())
    أرجع;

  بنانشان Real_وٿي = get_Mapped_وٿي(Orig_وٿي, وجهو_After);
  بفر.وجهو(Real_وٿي, Str.begin(), Str.پڇاڙي());

  // Add a delta so that future changes are offset correctly.
  جوڙيو_وجهو_Delta(Orig_وٿي, Str.سائيز_2());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
فراغ Rewrite_بفر::Replace_متن(بنانشان Orig_وٿي, بنانشان Orig_ڊيگهائي,
                                StringRef NewStr) {
  بنانشان Real_وٿي = get_Mapped_وٿي(Orig_وٿي, سچ);
  بفر.erase(Real_وٿي, Orig_ڊيگهائي);
  بفر.وجهو(Real_وٿي, NewStr.begin(), NewStr.پڇاڙي());
  إذا (Orig_ڊيگهائي != NewStr.سائيز_2())
    جوڙيو_Replace_Delta(Orig_وٿي, NewStr.سائيز_2() - Orig_ڊيگهائي);
}
