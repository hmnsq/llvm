//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/ADT/RewriteBuffer.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

उपयुज्य नामाकाश llvm;

raw_ostream &Rewrite_बफर::लिख(raw_ostream &प्रवाहः) ध्रुव {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  प्रति (RopePieceBTreeIterator I = begin(), E = अन्तः(); I != E;
       I.Move_गन्तव्यम्_अग्र_Piece())
    प्रवाहः << I.piece();
  प्रत्यागम प्रवाहः;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
स्थिर अन्तःस्थ तार्किक isWhitespaceExceptNL(अचिह्नित वर्ण c) {
  प्रत्यागम c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

शून्य Rewrite_बफर::RemoveText(अचिह्नित Orig_विस्थापनम्, अचिह्नित आकार,
                               तार्किक remove_पङ्क्तिः_If_रिक्तानि) {
  // Nothing to remove, exit early.
  यदि (आकार == 0)
    प्रत्यागम;

  अचिह्नित Real_विस्थापनम् = get_Mapped_विस्थापनम्(Orig_विस्थापनम्, सत्य);
  प्रतिज्ञा(Real_विस्थापनम् + आकार <= बफर.आकार_2() && "Invalid location");

  // Remove the dead characters.
  बफर.erase(Real_विस्थापनम्, आकार);

  // Add a delta so that future changes are offset correctly.
  सङ्कलय_Replace_Delta(Orig_विस्थापनम्, -आकार);

  यदि (remove_पङ्क्तिः_If_रिक्तानि) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_पङ्क्तिः_Start = begin();
    अचिह्नित cur_पङ्क्तिः_Start_Offs = 0;
    iterator posI = begin();
    प्रति (अचिह्नित i = 0; i != Real_विस्थापनम्; ++i) {
      यदि (*posI == '\n') {
        cur_पङ्क्तिः_Start = posI;
        ++cur_पङ्क्तिः_Start;
        cur_पङ्क्तिः_Start_Offs = i + 1;
      }
      ++posI;
    }

    अचिह्नित पङ्क्तिः_आकार = 0;
    posI = cur_पङ्क्तिः_Start;
    यावत् (posI != अन्तः() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++पङ्क्तिः_आकार;
    }
    यदि (posI != अन्तः() && *posI == '\n') {
      बफर.erase(cur_पङ्क्तिः_Start_Offs, पङ्क्तिः_आकार + 1 /* + '\n'*/);
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
      सङ्कलय_Replace_Delta(cur_पङ्क्तिः_Start_Offs, -(पङ्क्तिः_आकार + 1 /* + '\n'*/));
    }
  }
}

शून्य Rewrite_बफर::निवेशय_Text_2(अचिह्नित Orig_विस्थापनम्, StringRef Str,
                               तार्किक निवेशय_After) {
  // Nothing to insert, exit early.
  यदि (Str.रिक्तानि())
    प्रत्यागम;

  अचिह्नित Real_विस्थापनम् = get_Mapped_विस्थापनम्(Orig_विस्थापनम्, निवेशय_After);
  बफर.निवेशय(Real_विस्थापनम्, Str.begin(), Str.अन्तः());

  // Add a delta so that future changes are offset correctly.
  सङ्कलय_निवेशय_Delta(Orig_विस्थापनम्, Str.आकार_2());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
शून्य Rewrite_बफर::ReplaceText(अचिह्नित Orig_विस्थापनम्, अचिह्नित Orig_दीर्घता,
                                StringRef NewStr) {
  अचिह्नित Real_विस्थापनम् = get_Mapped_विस्थापनम्(Orig_विस्थापनम्, सत्य);
  बफर.erase(Real_विस्थापनम्, Orig_दीर्घता);
  बफर.निवेशय(Real_विस्थापनम्, NewStr.begin(), NewStr.अन्तः());
  यदि (Orig_दीर्घता != NewStr.आकार_2())
    सङ्कलय_Replace_Delta(Orig_विस्थापनम्, NewStr.आकार_2() - Orig_दीर्घता);
}
