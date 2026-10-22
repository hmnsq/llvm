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

static bool isAtLineEnd(const char *P) {
  もし (*P == '\n')
    戻る true;
  もし (*P == '\r' && *(P + 1) == '\n')
    戻る true;
  戻る false;
}

static bool skipIfAtLineEnd(const char *&P) {
  もし (*P == '\n') {
    ++P;
    戻る true;
  }
  もし (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    戻る true;
  }
  戻る false;
}

line_iterator::line_iterator(const MemoryBuffer &Buffer, bool SkipBlanks,
                             char CommentMarker)
    : line_iterator(Buffer.getMemBufferRef(), SkipBlanks, CommentMarker) {}

line_iterator::line_iterator(const MemoryBufferRef &Buffer, bool SkipBlanks,
                             char CommentMarker)
    : Buffer(Buffer.getBufferSize() ? std::optional<MemoryBufferRef>(Buffer)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      CurrentLine(Buffer.getBufferSize() ? Buffer.getBufferStart() : nullptr,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  もし (Buffer.getBufferSize()) {
    assert(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    もし (SkipBlanks || !isAtLineEnd(Buffer.getBufferStart()))
      advance();
  }
}

void line_iterator::advance() {
  assert(Buffer && "Cannot advance past the end!");

  const char *Pos = CurrentLine.end();
  assert(Pos == Buffer->getBufferStart() || isAtLineEnd(Pos) || *Pos == '\0');

  もし (skipIfAtLineEnd(Pos))
    ++LineNumber;
  もし (!SkipBlanks && isAtLineEnd(Pos)) {
    // Nothing to do for a blank line.
  } 否则 もし (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    ながら (skipIfAtLineEnd(Pos))
      ++LineNumber;
  } 否则 {
    // Skip comments and count line numbers, which is a bit more complex.
    ために (;;) {
      もし (isAtLineEnd(Pos) && !SkipBlanks)
        break;
      もし (*Pos == CommentMarker)
        do {
          ++Pos;
        } ながら (*Pos != '\0' && !isAtLineEnd(Pos));
      もし (!skipIfAtLineEnd(Pos))
        break;
      ++LineNumber;
    }
  }

  もし (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    CurrentLine = StringRef();
    戻る;
  }

  // Measure the line.
  size_t Length = 0;
  ながら (Pos[Length] != '\0' && !isAtLineEnd(&Pos[Length])) {
    ++Length;
  }

  CurrentLine = StringRef(Pos, Length);
}
