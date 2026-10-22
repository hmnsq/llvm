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

ساكن منطقي isAtLineEnd(ثابت حرف *P) {
  אם (*P == '\n')
    החזר אמת;
  אם (*P == '\r' && *(P + 1) == '\n')
    החזר אמת;
  החזר שקר;
}

ساكن منطقي skipIfAtLineEnd(ثابت حرف *&P) {
  אם (*P == '\n') {
    ++P;
    החזר אמת;
  }
  אם (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    החזר אמת;
  }
  החזר שקר;
}

line_iterator::line_iterator(ثابت MemoryBuffer &Buffer, منطقي SkipBlanks,
                             حرف CommentMarker)
    : line_iterator(Buffer.getMemBufferRef(), SkipBlanks, CommentMarker) {}

line_iterator::line_iterator(ثابت MemoryBufferRef &Buffer, منطقي SkipBlanks,
                             حرف CommentMarker)
    : Buffer(Buffer.getBufferSize() ? std::optional<MemoryBufferRef>(Buffer)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      CurrentLine(Buffer.getBufferSize() ? Buffer.getBufferStart() : nullptr,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  אם (Buffer.getBufferSize()) {
    أكد(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    אם (SkipBlanks || !isAtLineEnd(Buffer.getBufferStart()))
      advance();
  }
}

فراغ line_iterator::advance() {
  أكد(Buffer && "Cannot advance past the end!");

  ثابت حرف *Pos = CurrentLine.end();
  أكد(Pos == Buffer->getBufferStart() || isAtLineEnd(Pos) || *Pos == '\0');

  אם (skipIfAtLineEnd(Pos))
    ++LineNumber;
  אם (!SkipBlanks && isAtLineEnd(Pos)) {
    // Nothing to do for a blank line.
  } אחרת אם (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    כלעוד (skipIfAtLineEnd(Pos))
      ++LineNumber;
  } אחרת {
    // Skip comments and count line numbers, which is a bit more complex.
    עבור (;;) {
      אם (isAtLineEnd(Pos) && !SkipBlanks)
        اكسر;
      אם (*Pos == CommentMarker)
        עשה {
          ++Pos;
        } כלעוד (*Pos != '\0' && !isAtLineEnd(Pos));
      אם (!skipIfAtLineEnd(Pos))
        اكسر;
      ++LineNumber;
    }
  }

  אם (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    CurrentLine = StringRef();
    החזר;
  }

  // Measure the line.
  size_t Length = 0;
  כלעוד (Pos[Length] != '\0' && !isAtLineEnd(&Pos[Length])) {
    ++Length;
  }

  CurrentLine = StringRef(Pos, Length);
}
