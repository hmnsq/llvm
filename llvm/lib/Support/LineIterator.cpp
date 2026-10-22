//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/LineIterator.h"
#ضمن "llvm/Support/MemoryBuffer.h"

کارول نطاق llvm;

ساكن منطقي is_At_کرښه_پای_ټکی(ثابت حرف *P) {
  إذا (*P == '\n')
    أرجع ریښتیا;
  إذا (*P == '\r' && *(P + 1) == '\n')
    أرجع ریښتیا;
  أرجع خطأ;
}

ساكن منطقي skip_If_At_کرښه_پای_ټکی(ثابت حرف *&P) {
  إذا (*P == '\n') {
    ++P;
    أرجع ریښتیا;
  }
  إذا (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    أرجع ریښتیا;
  }
  أرجع خطأ;
}

کرښه_iterator::کرښه_iterator(ثابت حافظه_بفر &بفر, منطقي SkipBlanks,
                             حرف CommentMarker)
    : کرښه_iterator(بفر.get_Mem_بفر_Ref(), SkipBlanks, CommentMarker) {}

کرښه_iterator::کرښه_iterator(ثابت حافظه_بفر_Ref &بفر, منطقي SkipBlanks,
                             حرف CommentMarker)
    : بفر(بفر.get_بفر_اندازه() ? std::optional<حافظه_بفر_Ref>(بفر)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_کرښه(بفر.get_بفر_اندازه() ? بفر.get_بفر_پیل() : تش_اشاره,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  إذا (بفر.get_بفر_اندازه()) {
    أكد(بفر.get_بفر_پای_ټکی()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    إذا (SkipBlanks || !is_At_کرښه_پای_ټکی(بفر.get_بفر_پیل()))
      advance();
  }
}

فراغ کرښه_iterator::advance() {
  أكد(بفر && "Cannot advance past the end!");

  ثابت حرف *Pos = Current_کرښه.پای_ټکی();
  أكد(Pos == بفر->get_بفر_پیل() || is_At_کرښه_پای_ټکی(Pos) || *Pos == '\0');

  إذا (skip_If_At_کرښه_پای_ټکی(Pos))
    ++کرښه_شمېره;
  إذا (!SkipBlanks && is_At_کرښه_پای_ټکی(Pos)) {
    // Nothing to do for a blank line.
  } وإلا إذا (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    طالما (skip_If_At_کرښه_پای_ټکی(Pos))
      ++کرښه_شمېره;
  } وإلا {
    // Skip comments and count line numbers, which is a bit more complex.
    لكل (;;) {
      إذا (is_At_کرښه_پای_ټکی(Pos) && !SkipBlanks)
        اكسر;
      إذا (*Pos == CommentMarker)
        افعل {
          ++Pos;
        } طالما (*Pos != '\0' && !is_At_کرښه_پای_ټکی(Pos));
      إذا (!skip_If_At_کرښه_پای_ټکی(Pos))
        اكسر;
      ++کرښه_شمېره;
    }
  }

  إذا (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    بفر = std::nullopt;
    Current_کرښه = StringRef();
    أرجع;
  }

  // Measure the line.
  size_t اوږدتیا = 0;
  طالما (Pos[اوږدتیا] != '\0' && !is_At_کرښه_پای_ټکی(&Pos[اوږدتیا])) {
    ++اوږدتیا;
  }

  Current_کرښه = StringRef(Pos, اوږدتیا);
}
