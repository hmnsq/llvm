//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/LineIterator.h"
#ضمن "llvm/Support/MemoryBuffer.h"

استعمال نطاق llvm;

ساكن منطقي is_At_سٽ_پڇاڙي(ثابت حرف *P) {
  إذا (*P == '\n')
    أرجع سچ;
  إذا (*P == '\r' && *(P + 1) == '\n')
    أرجع سچ;
  أرجع خطأ;
}

ساكن منطقي skip_If_At_سٽ_پڇاڙي(ثابت حرف *&P) {
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

سٽ_iterator::سٽ_iterator(ثابت يادگيري_بفر &بفر, منطقي SkipBlanks,
                             حرف CommentMarker)
    : سٽ_iterator(بفر.get_Mem_بفر_Ref(), SkipBlanks, CommentMarker) {}

سٽ_iterator::سٽ_iterator(ثابت يادگيري_بفر_Ref &بفر, منطقي SkipBlanks,
                             حرف CommentMarker)
    : بفر(بفر.get_بفر_سائيز() ? std::optional<يادگيري_بفر_Ref>(بفر)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_سٽ(بفر.get_بفر_سائيز() ? بفر.get_بفر_شروعات() : خالي_اشارو,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  إذا (بفر.get_بفر_سائيز()) {
    أكد(بفر.get_بفر_پڇاڙي()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    إذا (SkipBlanks || !is_At_سٽ_پڇاڙي(بفر.get_بفر_شروعات()))
      advance();
  }
}

فراغ سٽ_iterator::advance() {
  أكد(بفر && "Cannot advance past the end!");

  ثابت حرف *Pos = Current_سٽ.پڇاڙي();
  أكد(Pos == بفر->get_بفر_شروعات() || is_At_سٽ_پڇاڙي(Pos) || *Pos == '\0');

  إذا (skip_If_At_سٽ_پڇاڙي(Pos))
    ++سٽ_عدد;
  إذا (!SkipBlanks && is_At_سٽ_پڇاڙي(Pos)) {
    // Nothing to do for a blank line.
  } وإلا إذا (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    طالما (skip_If_At_سٽ_پڇاڙي(Pos))
      ++سٽ_عدد;
  } وإلا {
    // Skip comments and count line numbers, which is a bit more complex.
    لكل (;;) {
      إذا (is_At_سٽ_پڇاڙي(Pos) && !SkipBlanks)
        اكسر;
      إذا (*Pos == CommentMarker)
        افعل {
          ++Pos;
        } طالما (*Pos != '\0' && !is_At_سٽ_پڇاڙي(Pos));
      إذا (!skip_If_At_سٽ_پڇاڙي(Pos))
        اكسر;
      ++سٽ_عدد;
    }
  }

  إذا (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    بفر = std::nullopt;
    Current_سٽ = StringRef();
    أرجع;
  }

  // Measure the line.
  size_t ڊيگهائي = 0;
  طالما (Pos[ڊيگهائي] != '\0' && !is_At_سٽ_پڇاڙي(&Pos[ڊيگهائي])) {
    ++ڊيگهائي;
  }

  Current_سٽ = StringRef(Pos, ڊيگهائي);
}
