//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/LineIterator.h"
#ضمن "llvm/Support/MemoryBuffer.h"

using نطاق llvm;

ساكن منطقي is_At_قۇر_End(ثابت حرف *P) {
  إذا (*P == '\n')
    أرجع true;
  إذا (*P == '\r' && *(P + 1) == '\n')
    أرجع true;
  أرجع خطأ;
}

ساكن منطقي skip_If_At_قۇر_End(ثابت حرف *&P) {
  إذا (*P == '\n') {
    ++P;
    أرجع true;
  }
  إذا (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    أرجع true;
  }
  أرجع خطأ;
}

قۇر_iterator::قۇر_iterator(ثابت MemoryBuffer &Buffer, منطقي SkipBlanks,
                             حرف CommentMarker)
    : قۇر_iterator(Buffer.getMemBufferRef(), SkipBlanks, CommentMarker) {}

قۇر_iterator::قۇر_iterator(ثابت MemoryBufferRef &Buffer, منطقي SkipBlanks,
                             حرف CommentMarker)
    : Buffer(Buffer.getBufferSize() ? std::optional<MemoryBufferRef>(Buffer)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_قۇر(Buffer.getBufferSize() ? Buffer.getBufferStart() : nullptr,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  إذا (Buffer.getBufferSize()) {
    أكد(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    إذا (SkipBlanks || !is_At_قۇر_End(Buffer.getBufferStart()))
      advance();
  }
}

فراغ قۇر_iterator::advance() {
  أكد(Buffer && "Cannot advance past the end!");

  ثابت حرف *Pos = Current_قۇر.end();
  أكد(Pos == Buffer->getBufferStart() || is_At_قۇر_End(Pos) || *Pos == '\0');

  إذا (skip_If_At_قۇر_End(Pos))
    ++قۇر_سان;
  إذا (!SkipBlanks && is_At_قۇر_End(Pos)) {
    // Nothing to do for a blank line.
  } وإلا إذا (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    طالما (skip_If_At_قۇر_End(Pos))
      ++قۇر_سان;
  } وإلا {
    // Skip comments and count line numbers, which is a bit more complex.
    لكل (;;) {
      إذا (is_At_قۇر_End(Pos) && !SkipBlanks)
        اكسر;
      إذا (*Pos == CommentMarker)
        افعل {
          ++Pos;
        } طالما (*Pos != '\0' && !is_At_قۇر_End(Pos));
      إذا (!skip_If_At_قۇر_End(Pos))
        اكسر;
      ++قۇر_سان;
    }
  }

  إذا (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    Current_قۇر = StringRef();
    أرجع;
  }

  // Measure the line.
  size_t Length = 0;
  طالما (Pos[Length] != '\0' && !is_At_قۇر_End(&Pos[Length])) {
    ++Length;
  }

  Current_قۇر = StringRef(Pos, Length);
}
