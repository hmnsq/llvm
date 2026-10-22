//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/LineIterator.h"
#அந்தர்பாவ "llvm/Support/MemoryBuffer.h"

using நாமாகாஶ llvm;

நிலையான தருக்க is_At_வரி_End(மாறிலி எழுத்து *P) {
  எனில் (*P == '\n')
    திரும்பு உண்மை;
  எனில் (*P == '\r' && *(P + 1) == '\n')
    திரும்பு உண்மை;
  திரும்பு பொய்;
}

நிலையான தருக்க skip_If_At_வரி_End(மாறிலி எழுத்து *&P) {
  எனில் (*P == '\n') {
    ++P;
    திரும்பு உண்மை;
  }
  எனில் (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    திரும்பு உண்மை;
  }
  திரும்பு பொய்;
}

வரி_iterator::வரி_iterator(மாறிலி MemoryBuffer &Buffer, தருக்க SkipBlanks,
                             எழுத்து CommentMarker)
    : வரி_iterator(Buffer.getMemBufferRef(), SkipBlanks, CommentMarker) {}

வரி_iterator::வரி_iterator(மாறிலி MemoryBufferRef &Buffer, தருக்க SkipBlanks,
                             எழுத்து CommentMarker)
    : Buffer(Buffer.getBufferSize() ? std::optional<MemoryBufferRef>(Buffer)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_வரி(Buffer.getBufferSize() ? Buffer.getBufferStart() : nullptr,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  எனில் (Buffer.getBufferSize()) {
    ப்ரதிஜ்ஞா(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    எனில் (SkipBlanks || !is_At_வரி_End(Buffer.getBufferStart()))
      advance();
  }
}

வெறுமை வரி_iterator::advance() {
  ப்ரதிஜ்ஞா(Buffer && "Cannot advance past the end!");

  மாறிலி எழுத்து *Pos = Current_வரி.end();
  ப்ரதிஜ்ஞா(Pos == Buffer->getBufferStart() || is_At_வரி_End(Pos) || *Pos == '\0');

  எனில் (skip_If_At_வரி_End(Pos))
    ++வரி_எண்;
  எனில் (!SkipBlanks && is_At_வரி_End(Pos)) {
    // Nothing to do for a blank line.
  } இல்லையெனில் எனில் (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    போது (skip_If_At_வரி_End(Pos))
      ++வரி_எண்;
  } இல்லையெனில் {
    // Skip comments and count line numbers, which is a bit more complex.
    க்கு (;;) {
      எனில் (is_At_வரி_End(Pos) && !SkipBlanks)
        நிறுத்து;
      எனில் (*Pos == CommentMarker)
        செய் {
          ++Pos;
        } போது (*Pos != '\0' && !is_At_வரி_End(Pos));
      எனில் (!skip_If_At_வரி_End(Pos))
        நிறுத்து;
      ++வரி_எண்;
    }
  }

  எனில் (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    Current_வரி = StringRef();
    திரும்பு;
  }

  // Measure the line.
  size_t Length = 0;
  போது (Pos[Length] != '\0' && !is_At_வரி_End(&Pos[Length])) {
    ++Length;
  }

  Current_வரி = StringRef(Pos, Length);
}
