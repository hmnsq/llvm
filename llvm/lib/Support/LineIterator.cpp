//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/LineIterator.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemoryBuffer.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਸਥਿਰ ਤਾਰਕਿਕ is_At_ਸਤਰ_ਸਿਰਾ(ਅਚਲ ਅੱਖਰ *P) {
  ਜੇ (*P == '\n')
    ਵਾਪਸ ਸੱਚ;
  ਜੇ (*P == '\r' && *(P + 1) == '\n')
    ਵਾਪਸ ਸੱਚ;
  ਵਾਪਸ ਝੂਠ;
}

ਸਥਿਰ ਤਾਰਕਿਕ skip_If_At_ਸਤਰ_ਸਿਰਾ(ਅਚਲ ਅੱਖਰ *&P) {
  ਜੇ (*P == '\n') {
    ++P;
    ਵਾਪਸ ਸੱਚ;
  }
  ਜੇ (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    ਵਾਪਸ ਸੱਚ;
  }
  ਵਾਪਸ ਝੂਠ;
}

ਸਤਰ_iterator::ਸਤਰ_iterator(ਅਚਲ ਸਮ੍ਰਿਤੀ_ਬਫਰ &ਬਫਰ, ਤਾਰਕਿਕ SkipBlanks,
                             ਅੱਖਰ CommentMarker)
    : ਸਤਰ_iterator(ਬਫਰ.get_Mem_ਬਫਰ_Ref(), SkipBlanks, CommentMarker) {}

ਸਤਰ_iterator::ਸਤਰ_iterator(ਅਚਲ ਸਮ੍ਰਿਤੀ_ਬਫਰ_Ref &ਬਫਰ, ਤਾਰਕਿਕ SkipBlanks,
                             ਅੱਖਰ CommentMarker)
    : ਬਫਰ(ਬਫਰ.get_ਬਫਰ_ਮਾਪ() ? std::optional<ਸਮ੍ਰਿਤੀ_ਬਫਰ_Ref>(ਬਫਰ)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_ਸਤਰ(ਬਫਰ.get_ਬਫਰ_ਮਾਪ() ? ਬਫਰ.get_ਬਫਰ_ਸ਼ੁਰੂ() : ਖਾਲੀ_ਸੂਚਕ,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  ਜੇ (ਬਫਰ.get_ਬਫਰ_ਮਾਪ()) {
    ਪ੍ਰਤਿਜ੍ਞਾ(ਬਫਰ.get_ਬਫਰ_ਸਿਰਾ()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    ਜੇ (SkipBlanks || !is_At_ਸਤਰ_ਸਿਰਾ(ਬਫਰ.get_ਬਫਰ_ਸ਼ੁਰੂ()))
      advance();
  }
}

ਖਾਲੀ ਸਤਰ_iterator::advance() {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਬਫਰ && "Cannot advance past the end!");

  ਅਚਲ ਅੱਖਰ *Pos = Current_ਸਤਰ.ਸਿਰਾ();
  ਪ੍ਰਤਿਜ੍ਞਾ(Pos == ਬਫਰ->get_ਬਫਰ_ਸ਼ੁਰੂ() || is_At_ਸਤਰ_ਸਿਰਾ(Pos) || *Pos == '\0');

  ਜੇ (skip_If_At_ਸਤਰ_ਸਿਰਾ(Pos))
    ++ਸਤਰ_ਸੰਖਿਆ;
  ਜੇ (!SkipBlanks && is_At_ਸਤਰ_ਸਿਰਾ(Pos)) {
    // Nothing to do for a blank line.
  } ਨਹੀਂਤਾਂ ਜੇ (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    ਜਦੋਂ (skip_If_At_ਸਤਰ_ਸਿਰਾ(Pos))
      ++ਸਤਰ_ਸੰਖਿਆ;
  } ਨਹੀਂਤਾਂ {
    // Skip comments and count line numbers, which is a bit more complex.
    ਲਈ (;;) {
      ਜੇ (is_At_ਸਤਰ_ਸਿਰਾ(Pos) && !SkipBlanks)
        ਰੋਕੋ;
      ਜੇ (*Pos == CommentMarker)
        ਕਰੋ {
          ++Pos;
        } ਜਦੋਂ (*Pos != '\0' && !is_At_ਸਤਰ_ਸਿਰਾ(Pos));
      ਜੇ (!skip_If_At_ਸਤਰ_ਸਿਰਾ(Pos))
        ਰੋਕੋ;
      ++ਸਤਰ_ਸੰਖਿਆ;
    }
  }

  ਜੇ (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    ਬਫਰ = std::nullopt;
    Current_ਸਤਰ = StringRef();
    ਵਾਪਸ;
  }

  // Measure the line.
  size_t ਲੰਮਾਈ = 0;
  ਜਦੋਂ (Pos[ਲੰਮਾਈ] != '\0' && !is_At_ਸਤਰ_ਸਿਰਾ(&Pos[ਲੰਮਾਈ])) {
    ++ਲੰਮਾਈ;
  }

  Current_ਸਤਰ = StringRef(Pos, ਲੰਮਾਈ);
}
