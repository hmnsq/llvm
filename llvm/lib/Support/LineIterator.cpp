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
  如果 (*P == '\n')
    返回 true;
  如果 (*P == '\r' && *(P + 1) == '\n')
    返回 true;
  返回 false;
}

static bool skipIfAtLineEnd(const char *&P) {
  如果 (*P == '\n') {
    ++P;
    返回 true;
  }
  如果 (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    返回 true;
  }
  返回 false;
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
  如果 (Buffer.getBufferSize()) {
    assert(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    如果 (SkipBlanks || !isAtLineEnd(Buffer.getBufferStart()))
      advance();
  }
}

void line_iterator::advance() {
  assert(Buffer && "Cannot advance past the end!");

  const char *Pos = CurrentLine.end();
  assert(Pos == Buffer->getBufferStart() || isAtLineEnd(Pos) || *Pos == '\0');

  如果 (skipIfAtLineEnd(Pos))
    ++LineNumber;
  如果 (!SkipBlanks && isAtLineEnd(Pos)) {
    // Nothing to do for a blank line.
  } 否则 如果 (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    当 (skipIfAtLineEnd(Pos))
      ++LineNumber;
  } 否则 {
    // Skip comments and count line numbers, which is a bit more complex.
    为 (;;) {
      如果 (isAtLineEnd(Pos) && !SkipBlanks)
        break;
      如果 (*Pos == CommentMarker)
        do {
          ++Pos;
        } 当 (*Pos != '\0' && !isAtLineEnd(Pos));
      如果 (!skipIfAtLineEnd(Pos))
        break;
      ++LineNumber;
    }
  }

  如果 (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    CurrentLine = StringRef();
    返回;
  }

  // Measure the line.
  size_t Length = 0;
  当 (Pos[Length] != '\0' && !isAtLineEnd(&Pos[Length])) {
    ++Length;
  }

  CurrentLine = StringRef(Pos, Length);
}
