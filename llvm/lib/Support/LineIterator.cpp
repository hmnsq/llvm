//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/Support/LineIterator.h"
#অন্তর্ভাৱ "llvm/Support/MemoryBuffer.h"

using নামাকাশ llvm;

স্থির তার্কিক is_At_সাৰি_End(ধ্রুৱ ৱর্ণ *P) {
  যদি (*P == '\n')
    ঘূৰাই সঁচা;
  যদি (*P == '\r' && *(P + 1) == '\n')
    ঘূৰাই সঁচা;
  ঘূৰাই মিছা;
}

স্থির তার্কিক skip_If_At_সাৰি_End(ধ্রুৱ ৱর্ণ *&P) {
  যদি (*P == '\n') {
    ++P;
    ঘূৰাই সঁচা;
  }
  যদি (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    ঘূৰাই সঁচা;
  }
  ঘূৰাই মিছা;
}

সাৰি_iterator::সাৰি_iterator(ধ্রুৱ MemoryBuffer &Buffer, তার্কিক SkipBlanks,
                             ৱর্ণ CommentMarker)
    : সাৰি_iterator(Buffer.getMemBufferRef(), SkipBlanks, CommentMarker) {}

সাৰি_iterator::সাৰি_iterator(ধ্রুৱ MemoryBufferRef &Buffer, তার্কিক SkipBlanks,
                             ৱর্ণ CommentMarker)
    : Buffer(Buffer.getBufferSize() ? std::optional<MemoryBufferRef>(Buffer)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_সাৰি(Buffer.getBufferSize() ? Buffer.getBufferStart() : nullptr,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  যদি (Buffer.getBufferSize()) {
    প্রতিজ্ঞা(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    যদি (SkipBlanks || !is_At_সাৰি_End(Buffer.getBufferStart()))
      advance();
  }
}

শূন্য সাৰি_iterator::advance() {
  প্রতিজ্ঞা(Buffer && "Cannot advance past the end!");

  ধ্রুৱ ৱর্ণ *Pos = Current_সাৰি.end();
  প্রতিজ্ঞা(Pos == Buffer->getBufferStart() || is_At_সাৰি_End(Pos) || *Pos == '\0');

  যদি (skip_If_At_সাৰি_End(Pos))
    ++সাৰি_সংখ্যা;
  যদি (!SkipBlanks && is_At_সাৰি_End(Pos)) {
    // Nothing to do for a blank line.
  } অন্যথা যদি (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    যেতিয়ালৈ (skip_If_At_সাৰি_End(Pos))
      ++সাৰি_সংখ্যা;
  } অন্যথা {
    // Skip comments and count line numbers, which is a bit more complex.
    বাবে (;;) {
      যদি (is_At_সাৰি_End(Pos) && !SkipBlanks)
        ভঙ্গ;
      যদি (*Pos == CommentMarker)
        কৰা {
          ++Pos;
        } যেতিয়ালৈ (*Pos != '\0' && !is_At_সাৰি_End(Pos));
      যদি (!skip_If_At_সাৰি_End(Pos))
        ভঙ্গ;
      ++সাৰি_সংখ্যা;
    }
  }

  যদি (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    Current_সাৰি = StringRef();
    ঘূৰাই;
  }

  // Measure the line.
  size_t Length = 0;
  যেতিয়ালৈ (Pos[Length] != '\0' && !is_At_সাৰি_End(&Pos[Length])) {
    ++Length;
  }

  Current_সাৰি = StringRef(Pos, Length);
}
