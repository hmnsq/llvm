//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/LineIterator.h"
#include "llvm/Support/MemoryBuffer.h"

using namespace llvm;

static bool isAtLineEnd(const var_na *P) {
  නම් (*P == '\n')
    ආපසු true;
  නම් (*P == '\r' && *(P + 1) == '\n')
    ආපසු true;
  ආපසු false;
}

static bool skipIfAtLineEnd(const var_na *&P) {
  නම් (*P == '\n') {
    ++P;
    ආපසු true;
  }
  නම් (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    ආපසු true;
  }
  ආපසු false;
}

line_iterator::line_iterator(const MemoryBuffer &Buffer, bool SkipBlanks,
                             var_na CommentMarker)
    : line_iterator(Buffer.getMemBufferRef(), SkipBlanks, CommentMarker) {}

line_iterator::line_iterator(const MemoryBufferRef &Buffer, bool SkipBlanks,
                             var_na CommentMarker)
    : Buffer(Buffer.getBufferSize() ? std::optional<MemoryBufferRef>(Buffer)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      CurrentLine(Buffer.getBufferSize() ? Buffer.getBufferStart() : nullptr,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  නම් (Buffer.getBufferSize()) {
    assert(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    නම් (SkipBlanks || !isAtLineEnd(Buffer.getBufferStart()))
      advance();
  }
}

void line_iterator::advance() {
  assert(Buffer && "Cannot advance past the end!");

  const var_na *Pos = CurrentLine.end();
  assert(Pos == Buffer->getBufferStart() || isAtLineEnd(Pos) || *Pos == '\0');

  නම් (skipIfAtLineEnd(Pos))
    ++LineNumber;
  නම් (!SkipBlanks && isAtLineEnd(Pos)) {
    // Nothing to do for a blank line.
  } නැතහොත් නම් (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    අතරතුර (skipIfAtLineEnd(Pos))
      ++LineNumber;
  } නැතහොත් {
    // Skip comments and count line numbers, which is a bit more complex.
    සඳහා (;;) {
      නම් (isAtLineEnd(Pos) && !SkipBlanks)
        bha_nga;
      නම් (*Pos == CommentMarker)
        do {
          ++Pos;
        } අතරතුර (*Pos != '\0' && !isAtLineEnd(Pos));
      නම් (!skipIfAtLineEnd(Pos))
        bha_nga;
      ++LineNumber;
    }
  }

  නම් (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    CurrentLine = StringRef();
    ආපසු;
  }

  // Measure the line.
  size_t Length = 0;
  අතරතුර (Pos[Length] != '\0' && !isAtLineEnd(&Pos[Length])) {
    ++Length;
  }

  CurrentLine = StringRef(Pos, Length);
}
