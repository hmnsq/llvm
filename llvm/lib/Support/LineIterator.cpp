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
  nếu (*P == '\n')
    trả_về true;
  nếu (*P == '\r' && *(P + 1) == '\n')
    trả_về true;
  trả_về false;
}

static bool skipIfAtLineEnd(const char *&P) {
  nếu (*P == '\n') {
    ++P;
    trả_về true;
  }
  nếu (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    trả_về true;
  }
  trả_về false;
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
  nếu (Buffer.getBufferSize()) {
    assert(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    nếu (SkipBlanks || !isAtLineEnd(Buffer.getBufferStart()))
      advance();
  }
}

void line_iterator::advance() {
  assert(Buffer && "Cannot advance past the end!");

  const char *Pos = CurrentLine.end();
  assert(Pos == Buffer->getBufferStart() || isAtLineEnd(Pos) || *Pos == '\0');

  nếu (skipIfAtLineEnd(Pos))
    ++LineNumber;
  nếu (!SkipBlanks && isAtLineEnd(Pos)) {
    // Nothing to do for a blank line.
  } không_thì nếu (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    trong_khi (skipIfAtLineEnd(Pos))
      ++LineNumber;
  } không_thì {
    // Skip comments and count line numbers, which is a bit more complex.
    cho (;;) {
      nếu (isAtLineEnd(Pos) && !SkipBlanks)
        break;
      nếu (*Pos == CommentMarker)
        do {
          ++Pos;
        } trong_khi (*Pos != '\0' && !isAtLineEnd(Pos));
      nếu (!skipIfAtLineEnd(Pos))
        break;
      ++LineNumber;
    }
  }

  nếu (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    CurrentLine = StringRef();
    trả_về;
  }

  // Measure the line.
  size_t Length = 0;
  trong_khi (Pos[Length] != '\0' && !isAtLineEnd(&Pos[Length])) {
    ++Length;
  }

  CurrentLine = StringRef(Pos, Length);
}
