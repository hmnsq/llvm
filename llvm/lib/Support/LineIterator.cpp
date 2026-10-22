//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/LineIterator.h"
#समावेश "llvm/Support/MemoryBuffer.h"

using namespace llvm;

अटल bool is_At_पंक्ति_End(स्थिर अक्षर *P) {
  अगर (*P == '\n')
    वापस true;
  अगर (*P == '\r' && *(P + 1) == '\n')
    वापस true;
  वापस false;
}

अटल bool skip_If_At_पंक्ति_End(स्थिर अक्षर *&P) {
  अगर (*P == '\n') {
    ++P;
    वापस true;
  }
  अगर (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    वापस true;
  }
  वापस false;
}

पंक्ति_iterator::पंक्ति_iterator(स्थिर Memory_बफर &बफर, bool SkipBlanks,
                             अक्षर CommentMarker)
    : पंक्ति_iterator(बफर.get_Mem_बफर_Ref(), SkipBlanks, CommentMarker) {}

पंक्ति_iterator::पंक्ति_iterator(स्थिर Memory_बफर_Ref &बफर, bool SkipBlanks,
                             अक्षर CommentMarker)
    : बफर(बफर.get_बफर_आकार() ? std::optional<Memory_बफर_Ref>(बफर)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_पंक्ति(बफर.get_बफर_आकार() ? बफर.get_बफर_Start() : nullptr,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  अगर (बफर.get_बफर_आकार()) {
    निश्चित(बफर.get_बफर_End()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    अगर (SkipBlanks || !is_At_पंक्ति_End(बफर.get_बफर_Start()))
      advance();
  }
}

व्योम पंक्ति_iterator::advance() {
  निश्चित(बफर && "Cannot advance past the end!");

  स्थिर अक्षर *Pos = Current_पंक्ति.end();
  निश्चित(Pos == बफर->get_बफर_Start() || is_At_पंक्ति_End(Pos) || *Pos == '\0');

  अगर (skip_If_At_पंक्ति_End(Pos))
    ++पंक्ति_संख्या;
  अगर (!SkipBlanks && is_At_पंक्ति_End(Pos)) {
    // Nothing to do for a blank line.
  } अन्यथा अगर (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    जबतक (skip_If_At_पंक्ति_End(Pos))
      ++पंक्ति_संख्या;
  } अन्यथा {
    // Skip comments and count line numbers, which is a bit more complex.
    क्रम (;;) {
      अगर (is_At_पंक्ति_End(Pos) && !SkipBlanks)
        अवरोध;
      अगर (*Pos == CommentMarker)
        करो {
          ++Pos;
        } जबतक (*Pos != '\0' && !is_At_पंक्ति_End(Pos));
      अगर (!skip_If_At_पंक्ति_End(Pos))
        अवरोध;
      ++पंक्ति_संख्या;
    }
  }

  अगर (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    बफर = std::nullopt;
    Current_पंक्ति = StringRef();
    वापस;
  }

  // Measure the line.
  माप_प्रकार लंबाई = 0;
  जबतक (Pos[लंबाई] != '\0' && !is_At_पंक्ति_End(&Pos[लंबाई])) {
    ++लंबाई;
  }

  Current_पंक्ति = StringRef(Pos, लंबाई);
}
