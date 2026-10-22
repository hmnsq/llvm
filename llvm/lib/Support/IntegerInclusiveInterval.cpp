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

#অন্তর্ভাব "llvm/Support/IntegerInclusiveInterval.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/Support/Error.h"
#অন্তর্ভাব "llvm/Support/Regex.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব <string>

using নামাকাশ llvm;

নামাকাশ llvm::IntegerInclusiveIntervalUtils {

Expected<IntervalList> parseIntervals(StringRef Str, বর্ণ Separator) {
  IntervalList Intervals;

  যদি (Str.empty())
    প্রত্যাগম std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  ধ্রুব Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  প্রতি (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    যদি (Part.empty())
      অনুবর্ত;

    SmallVector<StringRef, 4> Matches;
    যদি (!IntervalRegex.match(Part, &Matches))
      প্রত্যাগম createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    যদি (Matches[1].getAsInteger(10, Begin))
      প্রত্যাগম createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    যদি (!Matches[3].empty()) {
      // Interval format "begin-end".
      যদি (Matches[3].getAsInteger(10, End))
        প্রত্যাগম createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      যদি (Begin >= End)
        প্রত্যাগম createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } অন্যথা
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    যদি (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      প্রত্যাগম createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.ঠেলো_back(IntegerInclusiveInterval(Begin, End));
  }

  প্রত্যাগম Intervals;
}

তার্কিক contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t মান) {
  প্রতি (ধ্রুব IntegerInclusiveInterval &It : Intervals) {
    যদি (It.contains(মান))
      প্রত্যাগম সত্য;
  }
  প্রত্যাগম অসত্য;
}

শূন্য printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    বর্ণ Separator) {
  যদি (Intervals.empty()) {
    OS << "empty";
    প্রত্যাগম;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  প্রতি (ধ্রুব IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  যদি (Intervals.empty())
    প্রত্যাগম {};

  IntervalList Result;
  Result.ঠেলো_back(Intervals[0]);

  প্রতি (ধ্রুব IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = Result.back();
    // Check if current interval is adjacent to the last merged interval.
    যদি (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } অন্যথা {
      // Not adjacent, add as separate interval.
      Result.ঠেলো_back(Current);
    }
  }

  প্রত্যাগম Result;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
