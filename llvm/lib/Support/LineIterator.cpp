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
  ܐܢ (*P == '\n')
    ܦܢܐ ܫܪܪܐ;
  ܐܢ (*P == '\r' && *(P + 1) == '\n')
    ܦܢܐ ܫܪܪܐ;
  ܦܢܐ ܕܓܠܐ;
}

ساكن منطقي skipIfAtLineEnd(ثابت حرف *&P) {
  ܐܢ (*P == '\n') {
    ++P;
    ܦܢܐ ܫܪܪܐ;
  }
  ܐܢ (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    ܦܢܐ ܫܪܪܐ;
  }
  ܦܢܐ ܕܓܠܐ;
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
  ܐܢ (Buffer.getBufferSize()) {
    أكد(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    ܐܢ (SkipBlanks || !isAtLineEnd(Buffer.getBufferStart()))
      advance();
  }
}

فراغ line_iterator::advance() {
  أكد(Buffer && "Cannot advance past the end!");

  ثابت حرف *Pos = CurrentLine.end();
  أكد(Pos == Buffer->getBufferStart() || isAtLineEnd(Pos) || *Pos == '\0');

  ܐܢ (skipIfAtLineEnd(Pos))
    ++LineNumber;
  ܐܢ (!SkipBlanks && isAtLineEnd(Pos)) {
    // Nothing to do for a blank line.
  } ܐܚܪܢܐ ܐܢ (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    ܟܡܐ (skipIfAtLineEnd(Pos))
      ++LineNumber;
  } ܐܚܪܢܐ {
    // Skip comments and count line numbers, which is a bit more complex.
    لكل (;;) {
      ܐܢ (isAtLineEnd(Pos) && !SkipBlanks)
        اكسر;
      ܐܢ (*Pos == CommentMarker)
        ܥܒܕ {
          ++Pos;
        } ܟܡܐ (*Pos != '\0' && !isAtLineEnd(Pos));
      ܐܢ (!skipIfAtLineEnd(Pos))
        اكسر;
      ++LineNumber;
    }
  }

  ܐܢ (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    CurrentLine = StringRef();
    ܦܢܐ;
  }

  // Measure the line.
  size_t Length = 0;
  ܟܡܐ (Pos[Length] != '\0' && !isAtLineEnd(&Pos[Length])) {
    ++Length;
  }

  CurrentLine = StringRef(Pos, Length);
}
