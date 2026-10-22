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

स्थिर तार्किक is_At_ओळ_End(स्थिरांक अक्षर *P) {
  जर (*P == '\n')
    परत सत्य;
  जर (*P == '\r' && *(P + 1) == '\n')
    परत सत्य;
  परत असत्य;
}

स्थिर तार्किक skip_If_At_ओळ_End(स्थिरांक अक्षर *&P) {
  जर (*P == '\n') {
    ++P;
    परत सत्य;
  }
  जर (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    परत सत्य;
  }
  परत असत्य;
}

ओळ_iterator::ओळ_iterator(स्थिरांक MemoryBuffer &Buffer, तार्किक SkipBlanks,
                             अक्षर CommentMarker)
    : ओळ_iterator(Buffer.getMemBufferRef(), SkipBlanks, CommentMarker) {}

ओळ_iterator::ओळ_iterator(स्थिरांक MemoryBufferRef &Buffer, तार्किक SkipBlanks,
                             अक्षर CommentMarker)
    : Buffer(Buffer.getBufferSize() ? std::optional<MemoryBufferRef>(Buffer)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_ओळ(Buffer.getBufferSize() ? Buffer.getBufferStart() : nullptr,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  जर (Buffer.getBufferSize()) {
    प्रतिज्ञा(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    जर (SkipBlanks || !is_At_ओळ_End(Buffer.getBufferStart()))
      advance();
  }
}

शून्य ओळ_iterator::advance() {
  प्रतिज्ञा(Buffer && "Cannot advance past the end!");

  स्थिरांक अक्षर *Pos = Current_ओळ.end();
  प्रतिज्ञा(Pos == Buffer->getBufferStart() || is_At_ओळ_End(Pos) || *Pos == '\0');

  जर (skip_If_At_ओळ_End(Pos))
    ++ओळ_संख्या;
  जर (!SkipBlanks && is_At_ओळ_End(Pos)) {
    // Nothing to do for a blank line.
  } अन्यथा जर (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    पर्यंत (skip_If_At_ओळ_End(Pos))
      ++ओळ_संख्या;
  } अन्यथा {
    // Skip comments and count line numbers, which is a bit more complex.
    साठी (;;) {
      जर (is_At_ओळ_End(Pos) && !SkipBlanks)
        थांबा;
      जर (*Pos == CommentMarker)
        करा {
          ++Pos;
        } पर्यंत (*Pos != '\0' && !is_At_ओळ_End(Pos));
      जर (!skip_If_At_ओळ_End(Pos))
        थांबा;
      ++ओळ_संख्या;
    }
  }

  जर (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    Current_ओळ = StringRef();
    परत;
  }

  // Measure the line.
  माप_प्रकार Length = 0;
  पर्यंत (Pos[Length] != '\0' && !is_At_ओळ_End(&Pos[Length])) {
    ++Length;
  }

  Current_ओळ = StringRef(Pos, Length);
}
