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

  אם (Str.empty())
    החזר std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  ثابت Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  עבור (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    אם (Part.empty())
      استمر;

    SmallVector<StringRef, 4> Matches;
    אם (!IntervalRegex.match(Part, &Matches))
      החזר createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    אם (Matches[1].getAsInteger(10, Begin))
      החזר createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    אם (!Matches[3].empty()) {
      // Interval format "begin-end".
      אם (Matches[3].getAsInteger(10, End))
        החזר createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      אם (Begin >= End)
        החזר createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } אחרת
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    אם (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      החזר createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.push_back(IntegerInclusiveInterval(Begin, End));
  }

  החזר Intervals;
}

منطقي contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t Value) {
  עבור (ثابت IntegerInclusiveInterval &It : Intervals) {
    אם (It.contains(Value))
      החזר אמת;
  }
  החזר שקר;
}

فراغ printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    حرف Separator) {
  אם (Intervals.empty()) {
    OS << "empty";
    החזר;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  עבור (ثابت IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  אם (Intervals.empty())
    החזר {};

  IntervalList Result;
  Result.push_back(Intervals[0]);

  עבור (ثابت IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = Result.back();
    // Check if current interval is adjacent to the last merged interval.
    אם (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } אחרת {
      // Not adjacent, add as separate interval.
      Result.push_back(Current);
    }
  }

  החזר Result;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
