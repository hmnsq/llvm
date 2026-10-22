//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/LineIterator.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBuffer.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರ ತಾರ್ಕಿಕ is_At_ಸಾಲು_ಕೊನೆ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *P) {
  ಆದರೆ (*P == '\n')
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಆದರೆ (*P == '\r' && *(P + 1) == '\n')
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ skip_If_At_ಸಾಲು_ಕೊನೆ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *&P) {
  ಆದರೆ (*P == '\n') {
    ++P;
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಆದರೆ (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ಸಾಲು_iterator::ಸಾಲು_iterator(ಸ್ಥಿರಾಂಕ ಸ್ಮರಣೆ_ಬಫರ್ &ಬಫರ್, ತಾರ್ಕಿಕ SkipBlanks,
                             ಅಕ್ಷರ CommentMarker)
    : ಸಾಲು_iterator(ಬಫರ್.get_Mem_ಬಫರ್_Ref(), SkipBlanks, CommentMarker) {}

ಸಾಲು_iterator::ಸಾಲು_iterator(ಸ್ಥಿರಾಂಕ ಸ್ಮರಣೆ_ಬಫರ್_Ref &ಬಫರ್, ತಾರ್ಕಿಕ SkipBlanks,
                             ಅಕ್ಷರ CommentMarker)
    : ಬಫರ್(ಬಫರ್.get_ಬಫರ್_ಗಾತ್ರ() ? std::optional<ಸ್ಮರಣೆ_ಬಫರ್_Ref>(ಬಫರ್)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_ಸಾಲು(ಬಫರ್.get_ಬಫರ್_ಗಾತ್ರ() ? ಬಫರ್.get_ಬಫರ್_ಆರಂಭ() : ಶೂನ್ಯಸೂಚಕ,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  ಆದರೆ (ಬಫರ್.get_ಬಫರ್_ಗಾತ್ರ()) {
    ಪ್ರತಿಜ್ಞಾ(ಬಫರ್.get_ಬಫರ್_ಕೊನೆ()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    ಆದರೆ (SkipBlanks || !is_At_ಸಾಲು_ಕೊನೆ(ಬಫರ್.get_ಬಫರ್_ಆರಂಭ()))
      advance();
  }
}

ಶೂನ್ಯ ಸಾಲು_iterator::advance() {
  ಪ್ರತಿಜ್ಞಾ(ಬಫರ್ && "Cannot advance past the end!");

  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Pos = Current_ಸಾಲು.ಕೊನೆ();
  ಪ್ರತಿಜ್ಞಾ(Pos == ಬಫರ್->get_ಬಫರ್_ಆರಂಭ() || is_At_ಸಾಲು_ಕೊನೆ(Pos) || *Pos == '\0');

  ಆದರೆ (skip_If_At_ಸಾಲು_ಕೊನೆ(Pos))
    ++ಸಾಲು_ಸಂಖ್ಯೆ;
  ಆದರೆ (!SkipBlanks && is_At_ಸಾಲು_ಕೊನೆ(Pos)) {
    // Nothing to do for a blank line.
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    ಆಗ (skip_If_At_ಸಾಲು_ಕೊನೆ(Pos))
      ++ಸಾಲು_ಸಂಖ್ಯೆ;
  } ಇಲ್ಲದಿದ್ದರೆ {
    // Skip comments and count line numbers, which is a bit more complex.
    ಗಾಗಿ (;;) {
      ಆದರೆ (is_At_ಸಾಲು_ಕೊನೆ(Pos) && !SkipBlanks)
        ನಿಲ್ಲಿಸು;
      ಆದರೆ (*Pos == CommentMarker)
        ಮಾಡು {
          ++Pos;
        } ಆಗ (*Pos != '\0' && !is_At_ಸಾಲು_ಕೊನೆ(Pos));
      ಆದರೆ (!skip_If_At_ಸಾಲು_ಕೊನೆ(Pos))
        ನಿಲ್ಲಿಸು;
      ++ಸಾಲು_ಸಂಖ್ಯೆ;
    }
  }

  ಆದರೆ (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    ಬಫರ್ = std::nullopt;
    Current_ಸಾಲು = StringRef();
    ಹಿಂತಿರುಗಿ;
  }

  // Measure the line.
  size_t ಉದ್ದಳತೆ = 0;
  ಆಗ (Pos[ಉದ್ದಳತೆ] != '\0' && !is_At_ಸಾಲು_ಕೊನೆ(&Pos[ಉದ್ದಳತೆ])) {
    ++ಉದ್ದಳತೆ;
  }

  Current_ಸಾಲು = StringRef(Pos, ಉದ್ದಳತೆ);
}
