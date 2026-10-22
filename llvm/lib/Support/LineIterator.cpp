//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/LineIterator.h"
#অন্তর্ভাব "llvm/Support/MemoryBuffer.h"

using নামাকাশ llvm;

স্থির যৌক্তিক is_At_সারি_End(ধ্রুবক অক্ষর *P) {
  যদি (*P == '\n')
    ফেরত সত্য;
  যদি (*P == '\r' && *(P + 1) == '\n')
    ফেরত সত্য;
  ফেরত মিথ্যা;
}

স্থির যৌক্তিক skip_If_At_সারি_End(ধ্রুবক অক্ষর *&P) {
  যদি (*P == '\n') {
    ++P;
    ফেরত সত্য;
  }
  যদি (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    ফেরত সত্য;
  }
  ফেরত মিথ্যা;
}

সারি_iterator::সারি_iterator(ধ্রুবক Memory_বাফার &বাফার, যৌক্তিক SkipBlanks,
                             অক্ষর CommentMarker)
    : সারি_iterator(বাফার.get_Mem_বাফার_Ref(), SkipBlanks, CommentMarker) {}

সারি_iterator::সারি_iterator(ধ্রুবক Memory_বাফার_Ref &বাফার, যৌক্তিক SkipBlanks,
                             অক্ষর CommentMarker)
    : বাফার(বাফার.get_বাফার_আকার() ? std::optional<Memory_বাফার_Ref>(বাফার)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_সারি(বাফার.get_বাফার_আকার() ? বাফার.get_বাফার_Start() : nullptr,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  যদি (বাফার.get_বাফার_আকার()) {
    প্রতিজ্ঞা(বাফার.get_বাফার_End()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    যদি (SkipBlanks || !is_At_সারি_End(বাফার.get_বাফার_Start()))
      advance();
  }
}

শূন্য সারি_iterator::advance() {
  প্রতিজ্ঞা(বাফার && "Cannot advance past the end!");

  ধ্রুবক অক্ষর *Pos = Current_সারি.end();
  প্রতিজ্ঞা(Pos == বাফার->get_বাফার_Start() || is_At_সারি_End(Pos) || *Pos == '\0');

  যদি (skip_If_At_সারি_End(Pos))
    ++সারি_সংখ্যা;
  যদি (!SkipBlanks && is_At_সারি_End(Pos)) {
    // Nothing to do for a blank line.
  } অন্যথা যদি (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    যখন (skip_If_At_সারি_End(Pos))
      ++সারি_সংখ্যা;
  } অন্যথা {
    // Skip comments and count line numbers, which is a bit more complex.
    জন্য (;;) {
      যদি (is_At_সারি_End(Pos) && !SkipBlanks)
        বিরতি;
      যদি (*Pos == CommentMarker)
        করো {
          ++Pos;
        } যখন (*Pos != '\0' && !is_At_সারি_End(Pos));
      যদি (!skip_If_At_সারি_End(Pos))
        বিরতি;
      ++সারি_সংখ্যা;
    }
  }

  যদি (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    বাফার = std::nullopt;
    Current_সারি = StringRef();
    ফেরত;
  }

  // Measure the line.
  size_t দৈর্ঘ্য = 0;
  যখন (Pos[দৈর্ঘ্য] != '\0' && !is_At_সারি_End(&Pos[দৈর্ঘ্য])) {
    ++দৈর্ঘ্য;
  }

  Current_সারি = StringRef(Pos, দৈর্ঘ্য);
}
