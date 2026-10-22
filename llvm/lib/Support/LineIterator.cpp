//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/LineIterator.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"

उपयुज्य नामाकाश llvm;

स्थिर तार्किक is_At_पङ्क्तिः_अन्तः(ध्रुव वर्ण *P) {
  यदि (*P == '\n')
    प्रत्यागम सत्य;
  यदि (*P == '\r' && *(P + 1) == '\n')
    प्रत्यागम सत्य;
  प्रत्यागम असत्य;
}

स्थिर तार्किक skip_If_At_पङ्क्तिः_अन्तः(ध्रुव वर्ण *&P) {
  यदि (*P == '\n') {
    ++P;
    प्रत्यागम सत्य;
  }
  यदि (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

पङ्क्तिः_iterator::पङ्क्तिः_iterator(ध्रुव स्मृतिः_बफर &बफर, तार्किक SkipBlanks,
                             वर्ण CommentMarker)
    : पङ्क्तिः_iterator(बफर.get_Mem_बफर_Ref(), SkipBlanks, CommentMarker) {}

पङ्क्तिः_iterator::पङ्क्तिः_iterator(ध्रुव स्मृतिः_बफर_Ref &बफर, तार्किक SkipBlanks,
                             वर्ण CommentMarker)
    : बफर(बफर.get_बफर_आकार() ? std::optional<स्मृतिः_बफर_Ref>(बफर)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_पङ्क्तिः(बफर.get_बफर_आकार() ? बफर.get_बफर_Start() : शून्यसूचकः,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  यदि (बफर.get_बफर_आकार()) {
    प्रतिज्ञा(बफर.get_बफर_अन्तः()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    यदि (SkipBlanks || !is_At_पङ्क्तिः_अन्तः(बफर.get_बफर_Start()))
      advance();
  }
}

शून्य पङ्क्तिः_iterator::advance() {
  प्रतिज्ञा(बफर && "Cannot advance past the end!");

  ध्रुव वर्ण *Pos = Current_पङ्क्तिः.अन्तः();
  प्रतिज्ञा(Pos == बफर->get_बफर_Start() || is_At_पङ्क्तिः_अन्तः(Pos) || *Pos == '\0');

  यदि (skip_If_At_पङ्क्तिः_अन्तः(Pos))
    ++पङ्क्तिः_सङ्ख्या;
  यदि (!SkipBlanks && is_At_पङ्क्तिः_अन्तः(Pos)) {
    // Nothing to do for a blank line.
  } अन्यथा यदि (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    यावत् (skip_If_At_पङ्क्तिः_अन्तः(Pos))
      ++पङ्क्तिः_सङ्ख्या;
  } अन्यथा {
    // Skip comments and count line numbers, which is a bit more complex.
    प्रति (;;) {
      यदि (is_At_पङ्क्तिः_अन्तः(Pos) && !SkipBlanks)
        भङ्ग;
      यदि (*Pos == CommentMarker)
        कुरु {
          ++Pos;
        } यावत् (*Pos != '\0' && !is_At_पङ्क्तिः_अन्तः(Pos));
      यदि (!skip_If_At_पङ्क्तिः_अन्तः(Pos))
        भङ्ग;
      ++पङ्क्तिः_सङ्ख्या;
    }
  }

  यदि (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    बफर = std::nullopt;
    Current_पङ्क्तिः = StringRef();
    प्रत्यागम;
  }

  // Measure the line.
  size_t दीर्घता = 0;
  यावत् (Pos[दीर्घता] != '\0' && !is_At_पङ्क्तिः_अन्तः(&Pos[दीर्घता])) {
    ++दीर्घता;
  }

  Current_पङ्क्तिः = StringRef(Pos, दीर्घता);
}
