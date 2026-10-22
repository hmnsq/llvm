//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/LineIterator.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"

using नामाकाश llvm;

स्थिर तार्किक is_At_पङ्क्तिः_End(ध्रुव वर्ण *P) {
  सचे (*P == '\n')
    पटिनिवत्त सच्च;
  सचे (*P == '\r' && *(P + 1) == '\n')
    पटिनिवत्त सच्च;
  पटिनिवत्त मिच्छा;
}

स्थिर तार्किक skip_If_At_पङ्क्तिः_End(ध्रुव वर्ण *&P) {
  सचे (*P == '\n') {
    ++P;
    पटिनिवत्त सच्च;
  }
  सचे (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    पटिनिवत्त सच्च;
  }
  पटिनिवत्त मिच्छा;
}

पङ्क्तिः_iterator::पङ्क्तिः_iterator(ध्रुव MemoryBuffer &Buffer, तार्किक SkipBlanks,
                             वर्ण CommentMarker)
    : पङ्क्तिः_iterator(Buffer.getMemBufferRef(), SkipBlanks, CommentMarker) {}

पङ्क्तिः_iterator::पङ्क्तिः_iterator(ध्रुव MemoryBufferRef &Buffer, तार्किक SkipBlanks,
                             वर्ण CommentMarker)
    : Buffer(Buffer.getBufferSize() ? std::optional<MemoryBufferRef>(Buffer)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_पङ्क्तिः(Buffer.getBufferSize() ? Buffer.getBufferStart() : nullptr,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  सचे (Buffer.getBufferSize()) {
    प्रतिज्ञा(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    सचे (SkipBlanks || !is_At_पङ्क्तिः_End(Buffer.getBufferStart()))
      advance();
  }
}

शून्य पङ्क्तिः_iterator::advance() {
  प्रतिज्ञा(Buffer && "Cannot advance past the end!");

  ध्रुव वर्ण *Pos = Current_पङ्क्तिः.end();
  प्रतिज्ञा(Pos == Buffer->getBufferStart() || is_At_पङ्क्तिः_End(Pos) || *Pos == '\0');

  सचे (skip_If_At_पङ्क्तिः_End(Pos))
    ++पङ्क्तिः_सङ्ख्या;
  सचे (!SkipBlanks && is_At_पङ्क्तिः_End(Pos)) {
    // Nothing to do for a blank line.
  } अञ्ञथा सचे (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    याव (skip_If_At_पङ्क्तिः_End(Pos))
      ++पङ्क्तिः_सङ्ख्या;
  } अञ्ञथा {
    // Skip comments and count line numbers, which is a bit more complex.
    अत्थाय (;;) {
      सचे (is_At_पङ्क्तिः_End(Pos) && !SkipBlanks)
        भङ्ग;
      सचे (*Pos == CommentMarker)
        करोहि {
          ++Pos;
        } याव (*Pos != '\0' && !is_At_पङ्क्तिः_End(Pos));
      सचे (!skip_If_At_पङ्क्तिः_End(Pos))
        भङ्ग;
      ++पङ्क्तिः_सङ्ख्या;
    }
  }

  सचे (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    Current_पङ्क्तिः = StringRef();
    पटिनिवत्त;
  }

  // Measure the line.
  size_t Length = 0;
  याव (Pos[Length] != '\0' && !is_At_पङ्क्तिः_End(&Pos[Length])) {
    ++Length;
  }

  Current_पङ्क्तिः = StringRef(Pos, Length);
}
