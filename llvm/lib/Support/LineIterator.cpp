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

স্থির তার্কিক is_At_সারি_End(ধ্রুব বর্ণ *P) {
  যদি (*P == '\n')
    প্রত্যাগম সত্য;
  যদি (*P == '\r' && *(P + 1) == '\n')
    প্রত্যাগম সত্য;
  প্রত্যাগম অসত্য;
}

স্থির তার্কিক skip_If_At_সারি_End(ধ্রুব বর্ণ *&P) {
  যদি (*P == '\n') {
    ++P;
    প্রত্যাগম সত্য;
  }
  যদি (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    প্রত্যাগম সত্য;
  }
  প্রত্যাগম অসত্য;
}

সারি_iterator::সারি_iterator(ধ্রুব MemoryBuffer &Buffer, তার্কিক SkipBlanks,
                             বর্ণ CommentMarker)
    : সারি_iterator(Buffer.getMemBufferRef(), SkipBlanks, CommentMarker) {}

সারি_iterator::সারি_iterator(ধ্রুব MemoryBufferRef &Buffer, তার্কিক SkipBlanks,
                             বর্ণ CommentMarker)
    : Buffer(Buffer.getBufferSize() ? std::optional<MemoryBufferRef>(Buffer)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_সারি(Buffer.getBufferSize() ? Buffer.getBufferStart() : nullptr,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  যদি (Buffer.getBufferSize()) {
    প্রতিজ্ঞা(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    যদি (SkipBlanks || !is_At_সারি_End(Buffer.getBufferStart()))
      advance();
  }
}

শূন্য সারি_iterator::advance() {
  প্রতিজ্ঞা(Buffer && "Cannot advance past the end!");

  ধ্রুব বর্ণ *Pos = Current_সারি.end();
  প্রতিজ্ঞা(Pos == Buffer->getBufferStart() || is_At_সারি_End(Pos) || *Pos == '\0');

  যদি (skip_If_At_সারি_End(Pos))
    ++সারি_সংখ্যা;
  যদি (!SkipBlanks && is_At_সারি_End(Pos)) {
    // Nothing to do for a blank line.
  } অন্যথা যদি (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    যাবত্ (skip_If_At_সারি_End(Pos))
      ++সারি_সংখ্যা;
  } অন্যথা {
    // Skip comments and count line numbers, which is a bit more complex.
    প্রতি (;;) {
      যদি (is_At_সারি_End(Pos) && !SkipBlanks)
        ভঙ্গ;
      যদি (*Pos == CommentMarker)
        কুরু {
          ++Pos;
        } যাবত্ (*Pos != '\0' && !is_At_সারি_End(Pos));
      যদি (!skip_If_At_সারি_End(Pos))
        ভঙ্গ;
      ++সারি_সংখ্যা;
    }
  }

  যদি (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    Current_সারি = StringRef();
    প্রত্যাগম;
  }

  // Measure the line.
  size_t Length = 0;
  যাবত্ (Pos[Length] != '\0' && !is_At_সারি_End(&Pos[Length])) {
    ++Length;
  }

  Current_সারি = StringRef(Pos, Length);
}
