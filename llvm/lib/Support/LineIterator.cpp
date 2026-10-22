//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/LineIterator.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBuffer.h"

using ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರ ತಾರ್ಕಿಕ is_At_ಸಾಲು_End(ಧ್ರುವ ವರ್ಣ *P) {
  ಯದಿ (*P == '\n')
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಯದಿ (*P == '\r' && *(P + 1) == '\n')
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ skip_If_At_ಸಾಲು_End(ಧ್ರುವ ವರ್ಣ *&P) {
  ಯದಿ (*P == '\n') {
    ++P;
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಯದಿ (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ಸಾಲು_iterator::ಸಾಲು_iterator(ಧ್ರುವ MemoryBuffer &Buffer, ತಾರ್ಕಿಕ SkipBlanks,
                             ವರ್ಣ CommentMarker)
    : ಸಾಲು_iterator(Buffer.getMemBufferRef(), SkipBlanks, CommentMarker) {}

ಸಾಲು_iterator::ಸಾಲು_iterator(ಧ್ರುವ MemoryBufferRef &Buffer, ತಾರ್ಕಿಕ SkipBlanks,
                             ವರ್ಣ CommentMarker)
    : Buffer(Buffer.getBufferSize() ? std::optional<MemoryBufferRef>(Buffer)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_ಸಾಲು(Buffer.getBufferSize() ? Buffer.getBufferStart() : nullptr,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  ಯದಿ (Buffer.getBufferSize()) {
    ಪ್ರತಿಜ್ಞಾ(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    ಯದಿ (SkipBlanks || !is_At_ಸಾಲು_End(Buffer.getBufferStart()))
      advance();
  }
}

ಶೂನ್ಯ ಸಾಲು_iterator::advance() {
  ಪ್ರತಿಜ್ಞಾ(Buffer && "Cannot advance past the end!");

  ಧ್ರುವ ವರ್ಣ *Pos = Current_ಸಾಲು.end();
  ಪ್ರತಿಜ್ಞಾ(Pos == Buffer->getBufferStart() || is_At_ಸಾಲು_End(Pos) || *Pos == '\0');

  ಯದಿ (skip_If_At_ಸಾಲು_End(Pos))
    ++ಸಾಲು_ಸಂಖ್ಯೆ;
  ಯದಿ (!SkipBlanks && is_At_ಸಾಲು_End(Pos)) {
    // Nothing to do for a blank line.
  } ಅನ್ಯಥಾ ಯದಿ (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    ಯಾವತ್ (skip_If_At_ಸಾಲು_End(Pos))
      ++ಸಾಲು_ಸಂಖ್ಯೆ;
  } ಅನ್ಯಥಾ {
    // Skip comments and count line numbers, which is a bit more complex.
    ಪ್ರತಿ (;;) {
      ಯದಿ (is_At_ಸಾಲು_End(Pos) && !SkipBlanks)
        ಭಙ್ಗ;
      ಯದಿ (*Pos == CommentMarker)
        ಕುರು {
          ++Pos;
        } ಯಾವತ್ (*Pos != '\0' && !is_At_ಸಾಲು_End(Pos));
      ಯದಿ (!skip_If_At_ಸಾಲು_End(Pos))
        ಭಙ್ಗ;
      ++ಸಾಲು_ಸಂಖ್ಯೆ;
    }
  }

  ಯದಿ (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    Current_ಸಾಲು = StringRef();
    ಪ್ರತ್ಯಾಗಮ;
  }

  // Measure the line.
  size_t Length = 0;
  ಯಾವತ್ (Pos[Length] != '\0' && !is_At_ಸಾಲು_End(&Pos[Length])) {
    ++Length;
  }

  Current_ಸಾಲು = StringRef(Pos, Length);
}
