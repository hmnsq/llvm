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

#ضمن "llvm/Support/IntegerInclusiveInterval.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/Regex.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <string>

using نطاق llvm;

نطاق llvm::IntegerInclusiveIntervalUtils {

Expected<IntervalList> parseIntervals(StringRef Str, حرف Separator) {
  IntervalList Intervals;

  ܐܢ (Str.empty())
    ܦܢܐ std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  ثابت Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  لكل (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    ܐܢ (Part.empty())
      استمر;

    SmallVector<StringRef, 4> Matches;
    ܐܢ (!IntervalRegex.match(Part, &Matches))
      ܦܢܐ createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    ܐܢ (Matches[1].getAsInteger(10, Begin))
      ܦܢܐ createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    ܐܢ (!Matches[3].empty()) {
      // Interval format "begin-end".
      ܐܢ (Matches[3].getAsInteger(10, End))
        ܦܢܐ createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      ܐܢ (Begin >= End)
        ܦܢܐ createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } ܐܚܪܢܐ
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    ܐܢ (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      ܦܢܐ createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.push_back(IntegerInclusiveInterval(Begin, End));
  }

  ܦܢܐ Intervals;
}

منطقي contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t Value) {
  لكل (ثابت IntegerInclusiveInterval &It : Intervals) {
    ܐܢ (It.contains(Value))
      ܦܢܐ ܫܪܪܐ;
  }
  ܦܢܐ ܕܓܠܐ;
}

فراغ printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    حرف Separator) {
  ܐܢ (Intervals.empty()) {
    OS << "empty";
    ܦܢܐ;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  لكل (ثابت IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  ܐܢ (Intervals.empty())
    ܦܢܐ {};

  IntervalList Result;
  Result.push_back(Intervals[0]);

  لكل (ثابت IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = Result.back();
    // Check if current interval is adjacent to the last merged interval.
    ܐܢ (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } ܐܚܪܢܐ {
      // Not adjacent, add as separate interval.
      Result.push_back(Current);
    }
  }

  ܦܢܐ Result;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
