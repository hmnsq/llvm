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
  만약 (*P == '\n')
    반환 true;
  만약 (*P == '\r' && *(P + 1) == '\n')
    반환 true;
  반환 false;
}

static bool skipIfAtLineEnd(const char *&P) {
  만약 (*P == '\n') {
    ++P;
    반환 true;
  }
  만약 (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    반환 true;
  }
  반환 false;
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
  만약 (Buffer.getBufferSize()) {
    assert(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    만약 (SkipBlanks || !isAtLineEnd(Buffer.getBufferStart()))
      advance();
  }
}

void line_iterator::advance() {
  assert(Buffer && "Cannot advance past the end!");

  const char *Pos = CurrentLine.end();
  assert(Pos == Buffer->getBufferStart() || isAtLineEnd(Pos) || *Pos == '\0');

  만약 (skipIfAtLineEnd(Pos))
    ++LineNumber;
  만약 (!SkipBlanks && isAtLineEnd(Pos)) {
    // Nothing to do for a blank line.
  } 아니면 만약 (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    동안 (skipIfAtLineEnd(Pos))
      ++LineNumber;
  } 아니면 {
    // Skip comments and count line numbers, which is a bit more complex.
    위해 (;;) {
      만약 (isAtLineEnd(Pos) && !SkipBlanks)
        break;
      만약 (*Pos == CommentMarker)
        do {
          ++Pos;
        } 동안 (*Pos != '\0' && !isAtLineEnd(Pos));
      만약 (!skipIfAtLineEnd(Pos))
        break;
      ++LineNumber;
    }
  }

  만약 (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    CurrentLine = StringRef();
    반환;
  }

  // Measure the line.
  size_t Length = 0;
  동안 (Pos[Length] != '\0' && !isAtLineEnd(&Pos[Length])) {
    ++Length;
  }

  CurrentLine = StringRef(Pos, Length);
}
