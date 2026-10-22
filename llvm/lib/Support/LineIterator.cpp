//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/LineIterator.h"
#અન્તર્ભાવ "llvm/Support/MemoryBuffer.h"

using નામાકાશ llvm;

સ્થિર તાર્કિક is_At_લીટી_End(અચલ અક્ષર *P) {
  જો (*P == '\n')
    પાછા સાચું;
  જો (*P == '\r' && *(P + 1) == '\n')
    પાછા સાચું;
  પાછા ખોટું;
}

સ્થિર તાર્કિક skip_If_At_લીટી_End(અચલ અક્ષર *&P) {
  જો (*P == '\n') {
    ++P;
    પાછા સાચું;
  }
  જો (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    પાછા સાચું;
  }
  પાછા ખોટું;
}

લીટી_iterator::લીટી_iterator(અચલ MemoryBuffer &Buffer, તાર્કિક SkipBlanks,
                             અક્ષર CommentMarker)
    : લીટી_iterator(Buffer.getMemBufferRef(), SkipBlanks, CommentMarker) {}

લીટી_iterator::લીટી_iterator(અચલ MemoryBufferRef &Buffer, તાર્કિક SkipBlanks,
                             અક્ષર CommentMarker)
    : Buffer(Buffer.getBufferSize() ? std::optional<MemoryBufferRef>(Buffer)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_લીટી(Buffer.getBufferSize() ? Buffer.getBufferStart() : nullptr,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  જો (Buffer.getBufferSize()) {
    પ્રતિજ્ઞા(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    જો (SkipBlanks || !is_At_લીટી_End(Buffer.getBufferStart()))
      advance();
  }
}

શૂન્ય લીટી_iterator::advance() {
  પ્રતિજ્ઞા(Buffer && "Cannot advance past the end!");

  અચલ અક્ષર *Pos = Current_લીટી.end();
  પ્રતિજ્ઞા(Pos == Buffer->getBufferStart() || is_At_લીટી_End(Pos) || *Pos == '\0');

  જો (skip_If_At_લીટી_End(Pos))
    ++લીટી_સંખ્યા;
  જો (!SkipBlanks && is_At_લીટી_End(Pos)) {
    // Nothing to do for a blank line.
  } નહીંતર જો (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    જ્યારે (skip_If_At_લીટી_End(Pos))
      ++લીટી_સંખ્યા;
  } નહીંતર {
    // Skip comments and count line numbers, which is a bit more complex.
    માટે (;;) {
      જો (is_At_લીટી_End(Pos) && !SkipBlanks)
        તોડો;
      જો (*Pos == CommentMarker)
        કરો {
          ++Pos;
        } જ્યારે (*Pos != '\0' && !is_At_લીટી_End(Pos));
      જો (!skip_If_At_લીટી_End(Pos))
        તોડો;
      ++લીટી_સંખ્યા;
    }
  }

  જો (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    Current_લીટી = StringRef();
    પાછા;
  }

  // Measure the line.
  size_t Length = 0;
  જ્યારે (Pos[Length] != '\0' && !is_At_લીટી_End(&Pos[Length])) {
    ++Length;
  }

  Current_લીટી = StringRef(Pos, Length);
}
