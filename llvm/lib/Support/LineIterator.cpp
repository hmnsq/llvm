//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/LineIterator.h"
#ضمن "llvm/Support/MemoryBuffer.h"

استفاده نطاق llvm;

ساكن منطقي is_At_سطر_انتها(ثابت حرف *P) {
  إذا (*P == '\n')
    أرجع درست;
  إذا (*P == '\r' && *(P + 1) == '\n')
    أرجع درست;
  أرجع خطأ;
}

ساكن منطقي skip_If_At_سطر_انتها(ثابت حرف *&P) {
  إذا (*P == '\n') {
    ++P;
    أرجع درست;
  }
  إذا (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    أرجع درست;
  }
  أرجع خطأ;
}

سطر_iterator::سطر_iterator(ثابت حافظه_میانگیر &میانگیر, منطقي SkipBlanks,
                             حرف CommentMarker)
    : سطر_iterator(میانگیر.get_Mem_میانگیر_Ref(), SkipBlanks, CommentMarker) {}

سطر_iterator::سطر_iterator(ثابت حافظه_میانگیر_Ref &میانگیر, منطقي SkipBlanks,
                             حرف CommentMarker)
    : میانگیر(میانگیر.get_میانگیر_اندازه() ? std::optional<حافظه_میانگیر_Ref>(میانگیر)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_سطر(میانگیر.get_میانگیر_اندازه() ? میانگیر.get_میانگیر_شروع() : اشاره_تهی,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  إذا (میانگیر.get_میانگیر_اندازه()) {
    أكد(میانگیر.get_میانگیر_انتها()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    إذا (SkipBlanks || !is_At_سطر_انتها(میانگیر.get_میانگیر_شروع()))
      advance();
  }
}

فراغ سطر_iterator::advance() {
  أكد(میانگیر && "Cannot advance past the end!");

  ثابت حرف *Pos = Current_سطر.انتها();
  أكد(Pos == میانگیر->get_میانگیر_شروع() || is_At_سطر_انتها(Pos) || *Pos == '\0');

  إذا (skip_If_At_سطر_انتها(Pos))
    ++سطر_عدد;
  إذا (!SkipBlanks && is_At_سطر_انتها(Pos)) {
    // Nothing to do for a blank line.
  } وإلا إذا (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    طالما (skip_If_At_سطر_انتها(Pos))
      ++سطر_عدد;
  } وإلا {
    // Skip comments and count line numbers, which is a bit more complex.
    لكل (;;) {
      إذا (is_At_سطر_انتها(Pos) && !SkipBlanks)
        اكسر;
      إذا (*Pos == CommentMarker)
        افعل {
          ++Pos;
        } طالما (*Pos != '\0' && !is_At_سطر_انتها(Pos));
      إذا (!skip_If_At_سطر_انتها(Pos))
        اكسر;
      ++سطر_عدد;
    }
  }

  إذا (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    میانگیر = std::nullopt;
    Current_سطر = StringRef();
    أرجع;
  }

  // Measure the line.
  size_t درازی = 0;
  طالما (Pos[درازی] != '\0' && !is_At_سطر_انتها(&Pos[درازی])) {
    ++درازی;
  }

  Current_سطر = StringRef(Pos, درازی);
}
