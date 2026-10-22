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

#شامل "llvm/Support/IntegerInclusiveInterval.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Support/Error.h"
#شامل "llvm/Support/Regex.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <string>

using فضاینام llvm;

فضاینام llvm::IntegerInclusiveIntervalUtils {

Expected<IntervalList> parseIntervals(StringRef Str, نویسه Separator) {
  IntervalList Intervals;

  اگر (Str.empty())
    بازگشت std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  ثابت Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  برای (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    اگر (Part.empty())
      ادامه;

    SmallVector<StringRef, 4> Matches;
    اگر (!IntervalRegex.match(Part, &Matches))
      بازگشت createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    اگر (Matches[1].getAsInteger(10, Begin))
      بازگشت createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    اگر (!Matches[3].empty()) {
      // Interval format "begin-end".
      اگر (Matches[3].getAsInteger(10, End))
        بازگشت createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      اگر (Begin >= End)
        بازگشت createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } وگرنه
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    اگر (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      بازگشت createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.افزودن_back(IntegerInclusiveInterval(Begin, End));
  }

  بازگشت Intervals;
}

منطقی contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t مقدار) {
  برای (ثابت IntegerInclusiveInterval &It : Intervals) {
    اگر (It.contains(مقدار))
      بازگشت درست;
  }
  بازگشت نادرست;
}

تهی printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    نویسه Separator) {
  اگر (Intervals.empty()) {
    OS << "empty";
    بازگشت;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  برای (ثابت IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  اگر (Intervals.empty())
    بازگشت {};

  IntervalList Result;
  Result.افزودن_back(Intervals[0]);

  برای (ثابت IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = Result.back();
    // Check if current interval is adjacent to the last merged interval.
    اگر (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } وگرنه {
      // Not adjacent, add as separate interval.
      Result.افزودن_back(Current);
    }
  }

  بازگشت Result;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
