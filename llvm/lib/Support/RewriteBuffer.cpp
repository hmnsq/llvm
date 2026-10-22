//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/RewriteBuffer.h"
#ضمن "llvm/Support/raw_ostream.h"

ورتوں نطاق llvm;

raw_ostream &Rewrite_بفر::اكتب(raw_ostream &دھارا) ثابت {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  لكل (RopePieceBTreeIterator I = begin(), E = سرا(); I != E;
       I.Move_منزل_اگلا_Piece())
    دھارا << I.piece();
  أرجع دھارا;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
ساكن مضمن منطقي isWhitespaceExceptNL(بنانشانی حرف c) {
  أرجع c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

فراغ Rewrite_بفر::Remove_متن(بنانشانی Orig_وتھ, بنانشانی ناپ,
                               منطقي remove_سطر_If_خالی_تھاواں) {
  // Nothing to remove, exit early.
  إذا (ناپ == 0)
    أرجع;

  بنانشانی Real_وتھ = get_Mapped_وتھ(Orig_وتھ, سچ);
  أكد(Real_وتھ + ناپ <= بفر.ناپ_2() && "Invalid location");

  // Remove the dead characters.
  بفر.erase(Real_وتھ, ناپ);

  // Add a delta so that future changes are offset correctly.
  جوڑو_Replace_Delta(Orig_وتھ, -ناپ);

  إذا (remove_سطر_If_خالی_تھاواں) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_سطر_شروع = begin();
    بنانشانی cur_سطر_شروع_Offs = 0;
    iterator posI = begin();
    لكل (بنانشانی i = 0; i != Real_وتھ; ++i) {
      إذا (*posI == '\n') {
        cur_سطر_شروع = posI;
        ++cur_سطر_شروع;
        cur_سطر_شروع_Offs = i + 1;
      }
      ++posI;
    }

    بنانشانی سطر_ناپ = 0;
    posI = cur_سطر_شروع;
    طالما (posI != سرا() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++سطر_ناپ;
    }
    إذا (posI != سرا() && *posI == '\n') {
      بفر.erase(cur_سطر_شروع_Offs, سطر_ناپ + 1 /* + '\n'*/);
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
      جوڑو_Replace_Delta(cur_سطر_شروع_Offs, -(سطر_ناپ + 1 /* + '\n'*/));
    }
  }
}

فراغ Rewrite_بفر::داخل_کرو_متن_2(بنانشانی Orig_وتھ, StringRef Str,
                               منطقي داخل_کرو_After) {
  // Nothing to insert, exit early.
  إذا (Str.خالی_تھاواں())
    أرجع;

  بنانشانی Real_وتھ = get_Mapped_وتھ(Orig_وتھ, داخل_کرو_After);
  بفر.داخل_کرو(Real_وتھ, Str.begin(), Str.سرا());

  // Add a delta so that future changes are offset correctly.
  جوڑو_داخل_کرو_Delta(Orig_وتھ, Str.ناپ_2());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
فراغ Rewrite_بفر::Replace_متن(بنانشانی Orig_وتھ, بنانشانی Orig_لمائی,
                                StringRef NewStr) {
  بنانشانی Real_وتھ = get_Mapped_وتھ(Orig_وتھ, سچ);
  بفر.erase(Real_وتھ, Orig_لمائی);
  بفر.داخل_کرو(Real_وتھ, NewStr.begin(), NewStr.سرا());
  إذا (Orig_لمائی != NewStr.ناپ_2())
    جوڑو_Replace_Delta(Orig_وتھ, NewStr.ناپ_2() - Orig_لمائی);
}
