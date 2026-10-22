//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/LineIterator.h"
#ضمن "llvm/Support/MemoryBuffer.h"

ورتوں نطاق llvm;

ساكن منطقي is_At_سطر_سرا(ثابت حرف *P) {
  إذا (*P == '\n')
    أرجع سچ;
  إذا (*P == '\r' && *(P + 1) == '\n')
    أرجع سچ;
  أرجع خطأ;
}

ساكن منطقي skip_If_At_سطر_سرا(ثابت حرف *&P) {
  إذا (*P == '\n') {
    ++P;
    أرجع سچ;
  }
  إذا (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    أرجع سچ;
  }
  أرجع خطأ;
}

سطر_iterator::سطر_iterator(ثابت حافظہ_بفر &بفر, منطقي SkipBlanks,
                             حرف CommentMarker)
    : سطر_iterator(بفر.get_Mem_بفر_Ref(), SkipBlanks, CommentMarker) {}

سطر_iterator::سطر_iterator(ثابت حافظہ_بفر_Ref &بفر, منطقي SkipBlanks,
                             حرف CommentMarker)
    : بفر(بفر.get_بفر_ناپ() ? std::optional<حافظہ_بفر_Ref>(بفر)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_سطر(بفر.get_بفر_ناپ() ? بفر.get_بفر_شروع() : خالی_سوچک,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  إذا (بفر.get_بفر_ناپ()) {
    أكد(بفر.get_بفر_سرا()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    إذا (SkipBlanks || !is_At_سطر_سرا(بفر.get_بفر_شروع()))
      advance();
  }
}

فراغ سطر_iterator::advance() {
  أكد(بفر && "Cannot advance past the end!");

  ثابت حرف *Pos = Current_سطر.سرا();
  أكد(Pos == بفر->get_بفر_شروع() || is_At_سطر_سرا(Pos) || *Pos == '\0');

  إذا (skip_If_At_سطر_سرا(Pos))
    ++سطر_نمبر;
  إذا (!SkipBlanks && is_At_سطر_سرا(Pos)) {
    // Nothing to do for a blank line.
  } وإلا إذا (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    طالما (skip_If_At_سطر_سرا(Pos))
      ++سطر_نمبر;
  } وإلا {
    // Skip comments and count line numbers, which is a bit more complex.
    لكل (;;) {
      إذا (is_At_سطر_سرا(Pos) && !SkipBlanks)
        اكسر;
      إذا (*Pos == CommentMarker)
        افعل {
          ++Pos;
        } طالما (*Pos != '\0' && !is_At_سطر_سرا(Pos));
      إذا (!skip_If_At_سطر_سرا(Pos))
        اكسر;
      ++سطر_نمبر;
    }
  }

  إذا (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    بفر = std::nullopt;
    Current_سطر = StringRef();
    أرجع;
  }

  // Measure the line.
  size_t لمائی = 0;
  طالما (Pos[لمائی] != '\0' && !is_At_سطر_سرا(&Pos[لمائی])) {
    ++لمائی;
  }

  Current_سطر = StringRef(Pos, لمائی);
}
