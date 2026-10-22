//===- IntegerInclusiveInterval.cpp -----------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements utilities for handling lists of inclusive integer
// intervals, such as parsing interval strings like "1-10,20-30,45", which are
// used in debugging and bisection tools.
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/IntegerInclusiveInterval.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/Support/Error.h"
#അന്തര്ഭാവ "llvm/Support/Regex.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <string>

using നാമാകാശ llvm;

നാമാകാശ llvm::IntegerInclusiveIntervalUtils {

Expected<IntervalList> parseIntervals(StringRef Str, അക്ഷരം Separator) {
  IntervalList Intervals;

  എങ്കിൽ (Str.empty())
    തിരികെ std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  സ്ഥിരം Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  വേണ്ടി (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    എങ്കിൽ (Part.empty())
      തുടരുക;

    SmallVector<StringRef, 4> Matches;
    എങ്കിൽ (!IntervalRegex.match(Part, &Matches))
      തിരികെ createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    എങ്കിൽ (Matches[1].getAsInteger(10, Begin))
      തിരികെ createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    എങ്കിൽ (!Matches[3].empty()) {
      // Interval format "begin-end".
      എങ്കിൽ (Matches[3].getAsInteger(10, End))
        തിരികെ createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      എങ്കിൽ (Begin >= End)
        തിരികെ createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } അല്ലെങ്കിൽ
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    എങ്കിൽ (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      തിരികെ createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.തള്ളുക_back(IntegerInclusiveInterval(Begin, End));
  }

  തിരികെ Intervals;
}

തർക്കം contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t മൂല്യം) {
  വേണ്ടി (സ്ഥിരം IntegerInclusiveInterval &It : Intervals) {
    എങ്കിൽ (It.contains(മൂല്യം))
      തിരികെ സത്യം;
  }
  തിരികെ അസത്യം;
}

ശൂന്യം printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    അക്ഷരം Separator) {
  എങ്കിൽ (Intervals.empty()) {
    OS << "empty";
    തിരികെ;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  വേണ്ടി (സ്ഥിരം IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  എങ്കിൽ (Intervals.empty())
    തിരികെ {};

  IntervalList Result;
  Result.തള്ളുക_back(Intervals[0]);

  വേണ്ടി (സ്ഥിരം IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = Result.back();
    // Check if current interval is adjacent to the last merged interval.
    എങ്കിൽ (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } അല്ലെങ്കിൽ {
      // Not adjacent, add as separate interval.
      Result.തള്ളുക_back(Current);
    }
  }

  തിരികെ Result;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
