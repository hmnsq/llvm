//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/LineIterator.h"
#شامل "llvm/Support/MemoryBuffer.h"

using فضاینام llvm;

ایستا منطقی is_At_سطر_End(ثابت نویسه *P) {
  اگر (*P == '\n')
    بازگشت درست;
  اگر (*P == '\r' && *(P + 1) == '\n')
    بازگشت درست;
  بازگشت نادرست;
}

ایستا منطقی skip_If_At_سطر_End(ثابت نویسه *&P) {
  اگر (*P == '\n') {
    ++P;
    بازگشت درست;
  }
  اگر (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    بازگشت درست;
  }
  بازگشت نادرست;
}

سطر_iterator::سطر_iterator(ثابت MemoryBuffer &Buffer, منطقی SkipBlanks,
                             نویسه CommentMarker)
    : سطر_iterator(Buffer.getMemBufferRef(), SkipBlanks, CommentMarker) {}

سطر_iterator::سطر_iterator(ثابت MemoryBufferRef &Buffer, منطقی SkipBlanks,
                             نویسه CommentMarker)
    : Buffer(Buffer.getBufferSize() ? std::optional<MemoryBufferRef>(Buffer)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_سطر(Buffer.getBufferSize() ? Buffer.getBufferStart() : nullptr,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  اگر (Buffer.getBufferSize()) {
    أكد(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    اگر (SkipBlanks || !is_At_سطر_End(Buffer.getBufferStart()))
      advance();
  }
}

تهی سطر_iterator::advance() {
  أكد(Buffer && "Cannot advance past the end!");

  ثابت نویسه *Pos = Current_سطر.end();
  أكد(Pos == Buffer->getBufferStart() || is_At_سطر_End(Pos) || *Pos == '\0');

  اگر (skip_If_At_سطر_End(Pos))
    ++سطر_عدد;
  اگر (!SkipBlanks && is_At_سطر_End(Pos)) {
    // Nothing to do for a blank line.
  } وگرنه اگر (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    وقتی (skip_If_At_سطر_End(Pos))
      ++سطر_عدد;
  } وگرنه {
    // Skip comments and count line numbers, which is a bit more complex.
    برای (;;) {
      اگر (is_At_سطر_End(Pos) && !SkipBlanks)
        بشکن;
      اگر (*Pos == CommentMarker)
        انجامبده {
          ++Pos;
        } وقتی (*Pos != '\0' && !is_At_سطر_End(Pos));
      اگر (!skip_If_At_سطر_End(Pos))
        بشکن;
      ++سطر_عدد;
    }
  }

  اگر (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    Current_سطر = StringRef();
    بازگشت;
  }

  // Measure the line.
  size_t Length = 0;
  وقتی (Pos[Length] != '\0' && !is_At_سطر_End(&Pos[Length])) {
    ++Length;
  }

  Current_سطر = StringRef(Pos, Length);
}
