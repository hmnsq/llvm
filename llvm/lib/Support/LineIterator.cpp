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

स्थिर तार्किक is_At_पङ्क्ति_End(अचर वर्ण *P) {
  यदि (*P == '\n')
    फर्काऊ सत्य;
  यदि (*P == '\r' && *(P + 1) == '\n')
    फर्काऊ सत्य;
  फर्काऊ असत्य;
}

स्थिर तार्किक skip_If_At_पङ्क्ति_End(अचर वर्ण *&P) {
  यदि (*P == '\n') {
    ++P;
    फर्काऊ सत्य;
  }
  यदि (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    फर्काऊ सत्य;
  }
  फर्काऊ असत्य;
}

पङ्क्ति_iterator::पङ्क्ति_iterator(अचर MemoryBuffer &Buffer, तार्किक SkipBlanks,
                             वर्ण CommentMarker)
    : पङ्क्ति_iterator(Buffer.getMemBufferRef(), SkipBlanks, CommentMarker) {}

पङ्क्ति_iterator::पङ्क्ति_iterator(अचर MemoryBufferRef &Buffer, तार्किक SkipBlanks,
                             वर्ण CommentMarker)
    : Buffer(Buffer.getBufferSize() ? std::optional<MemoryBufferRef>(Buffer)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_पङ्क्ति(Buffer.getBufferSize() ? Buffer.getBufferStart() : nullptr,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  यदि (Buffer.getBufferSize()) {
    प्रतिज्ञा(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    यदि (SkipBlanks || !is_At_पङ्क्ति_End(Buffer.getBufferStart()))
      advance();
  }
}

शून्य पङ्क्ति_iterator::advance() {
  प्रतिज्ञा(Buffer && "Cannot advance past the end!");

  अचर वर्ण *Pos = Current_पङ्क्ति.end();
  प्रतिज्ञा(Pos == Buffer->getBufferStart() || is_At_पङ्क्ति_End(Pos) || *Pos == '\0');

  यदि (skip_If_At_पङ्क्ति_End(Pos))
    ++पङ्क्ति_सङ्ख्या;
  यदि (!SkipBlanks && is_At_पङ्क्ति_End(Pos)) {
    // Nothing to do for a blank line.
  } अन्यथा यदि (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    जबसम्म (skip_If_At_पङ्क्ति_End(Pos))
      ++पङ्क्ति_सङ्ख्या;
  } अन्यथा {
    // Skip comments and count line numbers, which is a bit more complex.
    लागि (;;) {
      यदि (is_At_पङ्क्ति_End(Pos) && !SkipBlanks)
        रोक;
      यदि (*Pos == CommentMarker)
        गर {
          ++Pos;
        } जबसम्म (*Pos != '\0' && !is_At_पङ्क्ति_End(Pos));
      यदि (!skip_If_At_पङ्क्ति_End(Pos))
        रोक;
      ++पङ्क्ति_सङ्ख्या;
    }
  }

  यदि (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    Current_पङ्क्ति = StringRef();
    फर्काऊ;
  }

  // Measure the line.
  माप_प्रकार Length = 0;
  जबसम्म (Pos[Length] != '\0' && !is_At_पङ्क्ति_End(&Pos[Length])) {
    ++Length;
  }

  Current_पङ्क्ति = StringRef(Pos, Length);
}
