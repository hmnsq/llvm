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
  ถ้า (*P == '\n')
    คืน true;
  ถ้า (*P == '\r' && *(P + 1) == '\n')
    คืน true;
  คืน false;
}

static bool skipIfAtLineEnd(const var_na *&P) {
  ถ้า (*P == '\n') {
    ++P;
    คืน true;
  }
  ถ้า (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    คืน true;
  }
  คืน false;
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
  ถ้า (Buffer.getBufferSize()) {
    assert(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    ถ้า (SkipBlanks || !isAtLineEnd(Buffer.getBufferStart()))
      advance();
  }
}

void line_iterator::advance() {
  assert(Buffer && "Cannot advance past the end!");

  const var_na *Pos = CurrentLine.end();
  assert(Pos == Buffer->getBufferStart() || isAtLineEnd(Pos) || *Pos == '\0');

  ถ้า (skipIfAtLineEnd(Pos))
    ++LineNumber;
  ถ้า (!SkipBlanks && isAtLineEnd(Pos)) {
    // Nothing to do for a blank line.
  } มิฉะนั้น ถ้า (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    ขณะ (skipIfAtLineEnd(Pos))
      ++LineNumber;
  } มิฉะนั้น {
    // Skip comments and count line numbers, which is a bit more complex.
    สำหรับ (;;) {
      ถ้า (isAtLineEnd(Pos) && !SkipBlanks)
        bha_nga;
      ถ้า (*Pos == CommentMarker)
        do {
          ++Pos;
        } ขณะ (*Pos != '\0' && !isAtLineEnd(Pos));
      ถ้า (!skipIfAtLineEnd(Pos))
        bha_nga;
      ++LineNumber;
    }
  }

  ถ้า (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    CurrentLine = StringRef();
    คืน;
  }

  // Measure the line.
  size_t Length = 0;
  ขณะ (Pos[Length] != '\0' && !isAtLineEnd(&Pos[Length])) {
    ++Length;
  }

  CurrentLine = StringRef(Pos, Length);
}
