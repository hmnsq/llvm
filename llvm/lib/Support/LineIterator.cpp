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
  𐤀𐤌 (*P == '\n')
    أرجع true;
  𐤀𐤌 (*P == '\r' && *(P + 1) == '\n')
    أرجع true;
  أرجع خطأ;
}

ساكن منطقي skipIfAtLineEnd(ثابت حرف *&P) {
  𐤀𐤌 (*P == '\n') {
    ++P;
    أرجع true;
  }
  𐤀𐤌 (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    أرجع true;
  }
  أرجع خطأ;
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
  𐤀𐤌 (Buffer.getBufferSize()) {
    أكد(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    𐤀𐤌 (SkipBlanks || !isAtLineEnd(Buffer.getBufferStart()))
      advance();
  }
}

فراغ line_iterator::advance() {
  أكد(Buffer && "Cannot advance past the end!");

  ثابت حرف *Pos = CurrentLine.end();
  أكد(Pos == Buffer->getBufferStart() || isAtLineEnd(Pos) || *Pos == '\0');

  𐤀𐤌 (skipIfAtLineEnd(Pos))
    ++LineNumber;
  𐤀𐤌 (!SkipBlanks && isAtLineEnd(Pos)) {
    // Nothing to do for a blank line.
  } وإلا 𐤀𐤌 (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    طالما (skipIfAtLineEnd(Pos))
      ++LineNumber;
  } وإلا {
    // Skip comments and count line numbers, which is a bit more complex.
    لكل (;;) {
      𐤀𐤌 (isAtLineEnd(Pos) && !SkipBlanks)
        اكسر;
      𐤀𐤌 (*Pos == CommentMarker)
        𐤐𐤏𐤋 {
          ++Pos;
        } طالما (*Pos != '\0' && !isAtLineEnd(Pos));
      𐤀𐤌 (!skipIfAtLineEnd(Pos))
        اكسر;
      ++LineNumber;
    }
  }

  𐤀𐤌 (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    CurrentLine = StringRef();
    أرجع;
  }

  // Measure the line.
  size_t Length = 0;
  طالما (Pos[Length] != '\0' && !isAtLineEnd(&Pos[Length])) {
    ++Length;
  }

  CurrentLine = StringRef(Pos, Length);
}
