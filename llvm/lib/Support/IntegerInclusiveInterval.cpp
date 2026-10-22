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

Expected<IntervalList> parseIntervals(StringRef Str, অক্ষর Separator) {
  IntervalList Intervals;

  যদি (Str.empty())
    ফেরত std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  ধ্রুবক Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  জন্য (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    যদি (Part.empty())
      চালু;

    SmallVector<StringRef, 4> Matches;
    যদি (!IntervalRegex.match(Part, &Matches))
      ফেরত createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    যদি (Matches[1].getAsInteger(10, Begin))
      ফেরত createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    যদি (!Matches[3].empty()) {
      // Interval format "begin-end".
      যদি (Matches[3].getAsInteger(10, End))
        ফেরত createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      যদি (Begin >= End)
        ফেরত createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } অন্যথা
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    যদি (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      ফেরত createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.ঠেলো_back(IntegerInclusiveInterval(Begin, End));
  }

  ফেরত Intervals;
}

যৌক্তিক contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t মান) {
  জন্য (ধ্রুবক IntegerInclusiveInterval &It : Intervals) {
    যদি (It.contains(মান))
      ফেরত সত্য;
  }
  ফেরত মিথ্যা;
}

শূন্য printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    অক্ষর Separator) {
  যদি (Intervals.empty()) {
    OS << "empty";
    ফেরত;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  জন্য (ধ্রুবক IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  যদি (Intervals.empty())
    ফেরত {};

  IntervalList ফলাফল;
  ফলাফল.ঠেলো_back(Intervals[0]);

  জন্য (ধ্রুবক IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = ফলাফল.back();
    // Check if current interval is adjacent to the last merged interval.
    যদি (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } অন্যথা {
      // Not adjacent, add as separate interval.
      ফলাফল.ঠেলো_back(Current);
    }
  }

  ফেরত ফলাফল;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
