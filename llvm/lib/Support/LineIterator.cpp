//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/LineIterator.h"
#അന്തര്ഭാവ "llvm/Support/MemoryBuffer.h"

using നാമാകാശ llvm;

സ്ഥിതം തർക്കം is_At_വരി_End(സ്ഥിരം അക്ഷരം *P) {
  എങ്കിൽ (*P == '\n')
    തിരികെ സത്യം;
  എങ്കിൽ (*P == '\r' && *(P + 1) == '\n')
    തിരികെ സത്യം;
  തിരികെ അസത്യം;
}

സ്ഥിതം തർക്കം skip_If_At_വരി_End(സ്ഥിരം അക്ഷരം *&P) {
  എങ്കിൽ (*P == '\n') {
    ++P;
    തിരികെ സത്യം;
  }
  എങ്കിൽ (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    തിരികെ സത്യം;
  }
  തിരികെ അസത്യം;
}

വരി_iterator::വരി_iterator(സ്ഥിരം MemoryBuffer &Buffer, തർക്കം SkipBlanks,
                             അക്ഷരം CommentMarker)
    : വരി_iterator(Buffer.getMemBufferRef(), SkipBlanks, CommentMarker) {}

വരി_iterator::വരി_iterator(സ്ഥിരം MemoryBufferRef &Buffer, തർക്കം SkipBlanks,
                             അക്ഷരം CommentMarker)
    : Buffer(Buffer.getBufferSize() ? std::optional<MemoryBufferRef>(Buffer)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_വരി(Buffer.getBufferSize() ? Buffer.getBufferStart() : nullptr,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  എങ്കിൽ (Buffer.getBufferSize()) {
    പ്രതിജ്ഞാ(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    എങ്കിൽ (SkipBlanks || !is_At_വരി_End(Buffer.getBufferStart()))
      advance();
  }
}

ശൂന്യം വരി_iterator::advance() {
  പ്രതിജ്ഞാ(Buffer && "Cannot advance past the end!");

  സ്ഥിരം അക്ഷരം *Pos = Current_വരി.end();
  പ്രതിജ്ഞാ(Pos == Buffer->getBufferStart() || is_At_വരി_End(Pos) || *Pos == '\0');

  എങ്കിൽ (skip_If_At_വരി_End(Pos))
    ++വരി_സംഖ്യ;
  എങ്കിൽ (!SkipBlanks && is_At_വരി_End(Pos)) {
    // Nothing to do for a blank line.
  } അല്ലെങ്കിൽ എങ്കിൽ (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    ആകുമ്പോൾ (skip_If_At_വരി_End(Pos))
      ++വരി_സംഖ്യ;
  } അല്ലെങ്കിൽ {
    // Skip comments and count line numbers, which is a bit more complex.
    വേണ്ടി (;;) {
      എങ്കിൽ (is_At_വരി_End(Pos) && !SkipBlanks)
        നിർത്തുക;
      എങ്കിൽ (*Pos == CommentMarker)
        ചെയ്യുക {
          ++Pos;
        } ആകുമ്പോൾ (*Pos != '\0' && !is_At_വരി_End(Pos));
      എങ്കിൽ (!skip_If_At_വരി_End(Pos))
        നിർത്തുക;
      ++വരി_സംഖ്യ;
    }
  }

  എങ്കിൽ (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    Current_വരി = StringRef();
    തിരികെ;
  }

  // Measure the line.
  size_t Length = 0;
  ആകുമ്പോൾ (Pos[Length] != '\0' && !is_At_വരി_End(&Pos[Length])) {
    ++Length;
  }

  Current_വരി = StringRef(Pos, Length);
}
