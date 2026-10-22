//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/ADT/RewriteBuffer.h"
#شامل "llvm/Support/raw_ostream.h"

استعمال نامخلا llvm;

raw_ostream &Rewrite_بفر::اكتب(raw_ostream &دھارا) ثابت {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  برائے (RopePieceBTreeIterator I = begin(), E = انتہا(); I != E;
       I.Move_مقصد_اگلا_Piece())
    دھارا << I.piece();
  واپس دھارا;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
ساکن اندرونی منطقی isWhitespaceExceptNL(غیرعلامتی حرف c) {
  واپس c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

خالی Rewrite_بفر::Remove_متن(غیرعلامتی Orig_انحراف, غیرعلامتی سائز,
                               منطقی remove_سطر_If_خالی_خانے) {
  // Nothing to remove, exit early.
  اگر (سائز == 0)
    واپس;

  غیرعلامتی Real_انحراف = get_Mapped_انحراف(Orig_انحراف, سچ);
  أكد(Real_انحراف + سائز <= بفر.سائز_2() && "Invalid location");

  // Remove the dead characters.
  بفر.erase(Real_انحراف, سائز);

  // Add a delta so that future changes are offset correctly.
  جوڑو_Replace_Delta(Orig_انحراف, -سائز);

  اگر (remove_سطر_If_خالی_خانے) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_سطر_آغاز = begin();
    غیرعلامتی cur_سطر_آغاز_Offs = 0;
    iterator posI = begin();
    برائے (غیرعلامتی i = 0; i != Real_انحراف; ++i) {
      اگر (*posI == '\n') {
        cur_سطر_آغاز = posI;
        ++cur_سطر_آغاز;
        cur_سطر_آغاز_Offs = i + 1;
      }
      ++posI;
    }

    غیرعلامتی سطر_سائز = 0;
    posI = cur_سطر_آغاز;
    جب (posI != انتہا() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++سطر_سائز;
    }
    اگر (posI != انتہا() && *posI == '\n') {
      بفر.erase(cur_سطر_آغاز_Offs, سطر_سائز + 1 /* + '\n'*/);
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
      جوڑو_Replace_Delta(cur_سطر_آغاز_Offs, -(سطر_سائز + 1 /* + '\n'*/));
    }
  }
}

خالی Rewrite_بفر::داخل_کرو_متن_2(غیرعلامتی Orig_انحراف, StringRef Str,
                               منطقی داخل_کرو_After) {
  // Nothing to insert, exit early.
  اگر (Str.خالی_خانے())
    واپس;

  غیرعلامتی Real_انحراف = get_Mapped_انحراف(Orig_انحراف, داخل_کرو_After);
  بفر.داخل_کرو(Real_انحراف, Str.begin(), Str.انتہا());

  // Add a delta so that future changes are offset correctly.
  جوڑو_داخل_کرو_Delta(Orig_انحراف, Str.سائز_2());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
خالی Rewrite_بفر::Replace_متن(غیرعلامتی Orig_انحراف, غیرعلامتی Orig_لمبائی,
                                StringRef NewStr) {
  غیرعلامتی Real_انحراف = get_Mapped_انحراف(Orig_انحراف, سچ);
  بفر.erase(Real_انحراف, Orig_لمبائی);
  بفر.داخل_کرو(Real_انحراف, NewStr.begin(), NewStr.انتہا());
  اگر (Orig_لمبائی != NewStr.سائز_2())
    جوڑو_Replace_Delta(Orig_انحراف, NewStr.سائز_2() - Orig_لمبائی);
}
