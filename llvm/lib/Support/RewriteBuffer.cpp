//===- RewriteBuffer.h - Buffer rewriting interface -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/ADT/RewriteBuffer.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"

ఉపయోగించి నామాకాశ llvm;

raw_ostream &Rewrite_బఫరు::లిఖ(raw_ostream &ప్రవాహం) స్థిరరాశి {
  // Walk RewriteRope chunks efficiently using MoveToNextPiece() instead of the
  // character iterator.
  కొరకు (RopePieceBTreeIterator I = begin(), E = చివర(); I != E;
       I.Move_గమ్యం_తదుపరి_Piece())
    ప్రవాహం << I.piece();
  తిరిగి ప్రవాహం;
}

/// Return true if this character is non-new-line whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\r'.
స్థిర అంతఃవరుస తార్కిక isWhitespaceExceptNL(గుర్తులేని అక్షరం c) {
  తిరిగి c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

శూన్య Rewrite_బఫరు::Remove_పాఠ్యం(గుర్తులేని Orig_స్థానభ్రంశం, గుర్తులేని కొలత,
                               తార్కిక remove_పంక్తి_If_ఖాళీలు) {
  // Nothing to remove, exit early.
  ఒకవేళ (కొలత == 0)
    తిరిగి;

  గుర్తులేని Real_స్థానభ్రంశం = get_Mapped_స్థానభ్రంశం(Orig_స్థానభ్రంశం, నిజం);
  ప్రతిజ్ఞా(Real_స్థానభ్రంశం + కొలత <= బఫరు.కొలత_2() && "Invalid location");

  // Remove the dead characters.
  బఫరు.erase(Real_స్థానభ్రంశం, కొలత);

  // Add a delta so that future changes are offset correctly.
  కూడు_Replace_Delta(Orig_స్థానభ్రంశం, -కొలత);

  ఒకవేళ (remove_పంక్తి_If_ఖాళీలు) {
    // Find the line that the remove occurred and if it is completely empty
    // remove the line as well.

    iterator cur_పంక్తి_ప్రారంభం = begin();
    గుర్తులేని cur_పంక్తి_ప్రారంభం_Offs = 0;
    iterator posI = begin();
    కొరకు (గుర్తులేని i = 0; i != Real_స్థానభ్రంశం; ++i) {
      ఒకవేళ (*posI == '\n') {
        cur_పంక్తి_ప్రారంభం = posI;
        ++cur_పంక్తి_ప్రారంభం;
        cur_పంక్తి_ప్రారంభం_Offs = i + 1;
      }
      ++posI;
    }

    గుర్తులేని పంక్తి_కొలత = 0;
    posI = cur_పంక్తి_ప్రారంభం;
    అయితే (posI != చివర() && isWhitespaceExceptNL(*posI)) {
      ++posI;
      ++పంక్తి_కొలత;
    }
    ఒకవేళ (posI != చివర() && *posI == '\n') {
      బఫరు.erase(cur_పంక్తి_ప్రారంభం_Offs, పంక్తి_కొలత + 1 /* + '\n'*/);
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
      కూడు_Replace_Delta(cur_పంక్తి_ప్రారంభం_Offs, -(పంక్తి_కొలత + 1 /* + '\n'*/));
    }
  }
}

శూన్య Rewrite_బఫరు::చొప్పించు_పాఠ్యం_2(గుర్తులేని Orig_స్థానభ్రంశం, StringRef Str,
                               తార్కిక చొప్పించు_After) {
  // Nothing to insert, exit early.
  ఒకవేళ (Str.ఖాళీలు())
    తిరిగి;

  గుర్తులేని Real_స్థానభ్రంశం = get_Mapped_స్థానభ్రంశం(Orig_స్థానభ్రంశం, చొప్పించు_After);
  బఫరు.చొప్పించు(Real_స్థానభ్రంశం, Str.begin(), Str.చివర());

  // Add a delta so that future changes are offset correctly.
  కూడు_చొప్పించు_Delta(Orig_స్థానభ్రంశం, Str.కొలత_2());
}

/// ReplaceText - This method replaces a range of characters in the input
/// buffer with a new string.  This is effectively a combined "remove+insert"
/// operation.
శూన్య Rewrite_బఫరు::Replace_పాఠ్యం(గుర్తులేని Orig_స్థానభ్రంశం, గుర్తులేని Orig_నిడివి,
                                StringRef NewStr) {
  గుర్తులేని Real_స్థానభ్రంశం = get_Mapped_స్థానభ్రంశం(Orig_స్థానభ్రంశం, నిజం);
  బఫరు.erase(Real_స్థానభ్రంశం, Orig_నిడివి);
  బఫరు.చొప్పించు(Real_స్థానభ్రంశం, NewStr.begin(), NewStr.చివర());
  ఒకవేళ (Orig_నిడివి != NewStr.కొలత_2())
    కూడు_Replace_Delta(Orig_స్థానభ్రంశం, NewStr.కొలత_2() - Orig_నిడివి);
}
