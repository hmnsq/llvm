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
  ຖ້າ (*P == '\n')
    ສົ່ງຄືນ true;
  ຖ້າ (*P == '\r' && *(P + 1) == '\n')
    ສົ່ງຄືນ true;
  ສົ່ງຄືນ false;
}

static bool skipIfAtLineEnd(const var_na *&P) {
  ຖ້າ (*P == '\n') {
    ++P;
    ສົ່ງຄືນ true;
  }
  ຖ້າ (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    ສົ່ງຄືນ true;
  }
  ສົ່ງຄືນ false;
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
  ຖ້າ (Buffer.getBufferSize()) {
    assert(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    ຖ້າ (SkipBlanks || !isAtLineEnd(Buffer.getBufferStart()))
      advance();
  }
}

void line_iterator::advance() {
  assert(Buffer && "Cannot advance past the end!");

  const var_na *Pos = CurrentLine.end();
  assert(Pos == Buffer->getBufferStart() || isAtLineEnd(Pos) || *Pos == '\0');

  ຖ້າ (skipIfAtLineEnd(Pos))
    ++LineNumber;
  ຖ້າ (!SkipBlanks && isAtLineEnd(Pos)) {
    // Nothing to do for a blank line.
  } ບໍ່ດັ່ງນັ້ນ ຖ້າ (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    ໃນຂະນະທີ່ (skipIfAtLineEnd(Pos))
      ++LineNumber;
  } ບໍ່ດັ່ງນັ້ນ {
    // Skip comments and count line numbers, which is a bit more complex.
    ສຳລັບ (;;) {
      ຖ້າ (isAtLineEnd(Pos) && !SkipBlanks)
        bha_nga;
      ຖ້າ (*Pos == CommentMarker)
        do {
          ++Pos;
        } ໃນຂະນະທີ່ (*Pos != '\0' && !isAtLineEnd(Pos));
      ຖ້າ (!skipIfAtLineEnd(Pos))
        bha_nga;
      ++LineNumber;
    }
  }

  ຖ້າ (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    CurrentLine = StringRef();
    ສົ່ງຄືນ;
  }

  // Measure the line.
  size_t Length = 0;
  ໃນຂະນະທີ່ (Pos[Length] != '\0' && !isAtLineEnd(&Pos[Length])) {
    ++Length;
  }

  CurrentLine = StringRef(Pos, Length);
}
