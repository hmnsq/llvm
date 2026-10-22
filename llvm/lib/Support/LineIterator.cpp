//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/LineIterator.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MemoryBuffer.h"

using ନାମାକାଶ llvm;

ସ୍ଥିର ତାର୍କିକ is_At_ଧାଡ଼ି_End(ଅଚଳ ଅକ୍ଷର *P) {
  ଯଦି (*P == '\n')
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଯଦି (*P == '\r' && *(P + 1) == '\n')
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ସ୍ଥିର ତାର୍କିକ skip_If_At_ଧାଡ଼ି_End(ଅଚଳ ଅକ୍ଷର *&P) {
  ଯଦି (*P == '\n') {
    ++P;
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ଯଦି (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ଧାଡ଼ି_iterator::ଧାଡ଼ି_iterator(ଅଚଳ MemoryBuffer &Buffer, ତାର୍କିକ SkipBlanks,
                             ଅକ୍ଷର CommentMarker)
    : ଧାଡ଼ି_iterator(Buffer.getMemBufferRef(), SkipBlanks, CommentMarker) {}

ଧାଡ଼ି_iterator::ଧାଡ଼ି_iterator(ଅଚଳ MemoryBufferRef &Buffer, ତାର୍କିକ SkipBlanks,
                             ଅକ୍ଷର CommentMarker)
    : Buffer(Buffer.getBufferSize() ? std::optional<MemoryBufferRef>(Buffer)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_ଧାଡ଼ି(Buffer.getBufferSize() ? Buffer.getBufferStart() : nullptr,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  ଯଦି (Buffer.getBufferSize()) {
    ପ୍ରତିଜ୍ଞା(Buffer.getBufferEnd()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    ଯଦି (SkipBlanks || !is_At_ଧାଡ଼ି_End(Buffer.getBufferStart()))
      advance();
  }
}

ଶୂନ୍ୟ ଧାଡ଼ି_iterator::advance() {
  ପ୍ରତିଜ୍ଞା(Buffer && "Cannot advance past the end!");

  ଅଚଳ ଅକ୍ଷର *Pos = Current_ଧାଡ଼ି.end();
  ପ୍ରତିଜ୍ଞା(Pos == Buffer->getBufferStart() || is_At_ଧାଡ଼ି_End(Pos) || *Pos == '\0');

  ଯଦି (skip_If_At_ଧାଡ଼ି_End(Pos))
    ++ଧାଡ଼ି_ସଂଖ୍ୟା;
  ଯଦି (!SkipBlanks && is_At_ଧାଡ଼ି_End(Pos)) {
    // Nothing to do for a blank line.
  } ଅନ୍ୟଥା ଯଦି (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    ଯେତେବେଳେ (skip_If_At_ଧାଡ଼ି_End(Pos))
      ++ଧାଡ଼ି_ସଂଖ୍ୟା;
  } ଅନ୍ୟଥା {
    // Skip comments and count line numbers, which is a bit more complex.
    ପାଇଁ (;;) {
      ଯଦି (is_At_ଧାଡ଼ି_End(Pos) && !SkipBlanks)
        ଭାଙ୍ଗ;
      ଯଦି (*Pos == CommentMarker)
        କର {
          ++Pos;
        } ଯେତେବେଳେ (*Pos != '\0' && !is_At_ଧାଡ଼ି_End(Pos));
      ଯଦି (!skip_If_At_ଧାଡ଼ି_End(Pos))
        ଭାଙ୍ଗ;
      ++ଧାଡ଼ି_ସଂଖ୍ୟା;
    }
  }

  ଯଦି (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    Buffer = std::nullopt;
    Current_ଧାଡ଼ି = StringRef();
    ଫେରନ୍ତୁ;
  }

  // Measure the line.
  size_t Length = 0;
  ଯେତେବେଳେ (Pos[Length] != '\0' && !is_At_ଧାଡ଼ି_End(&Pos[Length])) {
    ++Length;
  }

  Current_ଧାଡ଼ି = StringRef(Pos, Length);
}
