//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/LineIterator.h"
#شامل "llvm/Support/MemoryBuffer.h"

استعمال نامخلا llvm;

ساکن منطقی is_At_سطر_انتہا(ثابت حرف *P) {
  اگر (*P == '\n')
    واپس سچ;
  اگر (*P == '\r' && *(P + 1) == '\n')
    واپس سچ;
  واپس جھوٹ;
}

ساکن منطقی skip_If_At_سطر_انتہا(ثابت حرف *&P) {
  اگر (*P == '\n') {
    ++P;
    واپس سچ;
  }
  اگر (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    واپس سچ;
  }
  واپس جھوٹ;
}

سطر_iterator::سطر_iterator(ثابت حافظہ_بفر &بفر, منطقی SkipBlanks,
                             حرف CommentMarker)
    : سطر_iterator(بفر.get_Mem_بفر_Ref(), SkipBlanks, CommentMarker) {}

سطر_iterator::سطر_iterator(ثابت حافظہ_بفر_Ref &بفر, منطقی SkipBlanks,
                             حرف CommentMarker)
    : بفر(بفر.get_بفر_سائز() ? std::optional<حافظہ_بفر_Ref>(بفر)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_سطر(بفر.get_بفر_سائز() ? بفر.get_بفر_آغاز() : خالی_اشاریہ,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  اگر (بفر.get_بفر_سائز()) {
    أكد(بفر.get_بفر_انتہا()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    اگر (SkipBlanks || !is_At_سطر_انتہا(بفر.get_بفر_آغاز()))
      advance();
  }
}

خالی سطر_iterator::advance() {
  أكد(بفر && "Cannot advance past the end!");

  ثابت حرف *Pos = Current_سطر.انتہا();
  أكد(Pos == بفر->get_بفر_آغاز() || is_At_سطر_انتہا(Pos) || *Pos == '\0');

  اگر (skip_If_At_سطر_انتہا(Pos))
    ++سطر_نمبر;
  اگر (!SkipBlanks && is_At_سطر_انتہا(Pos)) {
    // Nothing to do for a blank line.
  } ورنہ اگر (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    جب (skip_If_At_سطر_انتہا(Pos))
      ++سطر_نمبر;
  } ورنہ {
    // Skip comments and count line numbers, which is a bit more complex.
    برائے (;;) {
      اگر (is_At_سطر_انتہا(Pos) && !SkipBlanks)
        توڑو;
      اگر (*Pos == CommentMarker)
        کرو {
          ++Pos;
        } جب (*Pos != '\0' && !is_At_سطر_انتہا(Pos));
      اگر (!skip_If_At_سطر_انتہا(Pos))
        توڑو;
      ++سطر_نمبر;
    }
  }

  اگر (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    بفر = std::nullopt;
    Current_سطر = StringRef();
    واپس;
  }

  // Measure the line.
  size_t لمبائی = 0;
  جب (Pos[لمبائی] != '\0' && !is_At_سطر_انتہا(&Pos[لمبائی])) {
    ++لمبائی;
  }

  Current_سطر = StringRef(Pos, لمبائی);
}
