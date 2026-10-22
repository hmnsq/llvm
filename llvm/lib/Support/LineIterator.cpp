//===- LineIterator.cpp - Implementation of line iteration ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/LineIterator.h"
#అన్తర్భావ "llvm/Support/MemoryBuffer.h"

ఉపయోగించి నామాకాశ llvm;

స్థిర తార్కిక is_At_పంక్తి_చివర(స్థిరరాశి అక్షరం *P) {
  ఒకవేళ (*P == '\n')
    తిరిగి నిజం;
  ఒకవేళ (*P == '\r' && *(P + 1) == '\n')
    తిరిగి నిజం;
  తిరిగి అబద్ధం;
}

స్థిర తార్కిక skip_If_At_పంక్తి_చివర(స్థిరరాశి అక్షరం *&P) {
  ఒకవేళ (*P == '\n') {
    ++P;
    తిరిగి నిజం;
  }
  ఒకవేళ (*P == '\r' && *(P + 1) == '\n') {
    P += 2;
    తిరిగి నిజం;
  }
  తిరిగి అబద్ధం;
}

పంక్తి_iterator::పంక్తి_iterator(స్థిరరాశి జ్ఞాపకం_బఫరు &బఫరు, తార్కిక SkipBlanks,
                             అక్షరం CommentMarker)
    : పంక్తి_iterator(బఫరు.get_Mem_బఫరు_Ref(), SkipBlanks, CommentMarker) {}

పంక్తి_iterator::పంక్తి_iterator(స్థిరరాశి జ్ఞాపకం_బఫరు_Ref &బఫరు, తార్కిక SkipBlanks,
                             అక్షరం CommentMarker)
    : బఫరు(బఫరు.get_బఫరు_కొలత() ? std::optional<జ్ఞాపకం_బఫరు_Ref>(బఫరు)
                                    : std::nullopt),
      CommentMarker(CommentMarker), SkipBlanks(SkipBlanks),
      Current_పంక్తి(బఫరు.get_బఫరు_కొలత() ? బఫరు.get_బఫరు_ప్రారంభం() : శూన్యసూచిక,
                  0) {
  // Ensure that if we are constructed on a non-empty memory buffer that it is
  // a null terminated buffer.
  ఒకవేళ (బఫరు.get_బఫరు_కొలత()) {
    ప్రతిజ్ఞా(బఫరు.get_బఫరు_చివర()[0] == '\0');
    // Make sure we don't skip a leading newline if we're keeping blanks
    ఒకవేళ (SkipBlanks || !is_At_పంక్తి_చివర(బఫరు.get_బఫరు_ప్రారంభం()))
      advance();
  }
}

శూన్య పంక్తి_iterator::advance() {
  ప్రతిజ్ఞా(బఫరు && "Cannot advance past the end!");

  స్థిరరాశి అక్షరం *Pos = Current_పంక్తి.చివర();
  ప్రతిజ్ఞా(Pos == బఫరు->get_బఫరు_ప్రారంభం() || is_At_పంక్తి_చివర(Pos) || *Pos == '\0');

  ఒకవేళ (skip_If_At_పంక్తి_చివర(Pos))
    ++పంక్తి_సంఖ్య;
  ఒకవేళ (!SkipBlanks && is_At_పంక్తి_చివర(Pos)) {
    // Nothing to do for a blank line.
  } లేకపోతే ఒకవేళ (CommentMarker == '\0') {
    // If we're not stripping comments, this is simpler.
    అయితే (skip_If_At_పంక్తి_చివర(Pos))
      ++పంక్తి_సంఖ్య;
  } లేకపోతే {
    // Skip comments and count line numbers, which is a bit more complex.
    కొరకు (;;) {
      ఒకవేళ (is_At_పంక్తి_చివర(Pos) && !SkipBlanks)
        ఆపు;
      ఒకవేళ (*Pos == CommentMarker)
        చేయి {
          ++Pos;
        } అయితే (*Pos != '\0' && !is_At_పంక్తి_చివర(Pos));
      ఒకవేళ (!skip_If_At_పంక్తి_చివర(Pos))
        ఆపు;
      ++పంక్తి_సంఖ్య;
    }
  }

  ఒకవేళ (*Pos == '\0') {
    // We've hit the end of the buffer, reset ourselves to the end state.
    బఫరు = std::nullopt;
    Current_పంక్తి = StringRef();
    తిరిగి;
  }

  // Measure the line.
  size_t నిడివి = 0;
  అయితే (Pos[నిడివి] != '\0' && !is_At_పంక్తి_చివర(&Pos[నిడివి])) {
    ++నిడివి;
  }

  Current_పంక్తి = StringRef(Pos, నిడివి);
}
