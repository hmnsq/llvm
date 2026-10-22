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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/IntegerInclusiveInterval.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Error.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Regex.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <string>

using ନାମାକାଶ llvm;

ନାମାକାଶ llvm::IntegerInclusiveIntervalUtils {

Expected<IntervalList> parseIntervals(StringRef Str, ଅକ୍ଷର Separator) {
  IntervalList Intervals;

  ଯଦି (Str.empty())
    ଫେରନ୍ତୁ std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  ଅଚଳ Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  ପାଇଁ (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    ଯଦି (Part.empty())
      ଜାରି;

    SmallVector<StringRef, 4> Matches;
    ଯଦି (!IntervalRegex.match(Part, &Matches))
      ଫେରନ୍ତୁ createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    ଯଦି (Matches[1].getAsInteger(10, Begin))
      ଫେରନ୍ତୁ createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    ଯଦି (!Matches[3].empty()) {
      // Interval format "begin-end".
      ଯଦି (Matches[3].getAsInteger(10, End))
        ଫେରନ୍ତୁ createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      ଯଦି (Begin >= End)
        ଫେରନ୍ତୁ createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } ଅନ୍ୟଥା
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    ଯଦି (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      ଫେରନ୍ତୁ createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.ଠେଲ_back(IntegerInclusiveInterval(Begin, End));
  }

  ଫେରନ୍ତୁ Intervals;
}

ତାର୍କିକ contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t ମୂଲ୍ୟ) {
  ପାଇଁ (ଅଚଳ IntegerInclusiveInterval &It : Intervals) {
    ଯଦି (It.contains(ମୂଲ୍ୟ))
      ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ଶୂନ୍ୟ printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    ଅକ୍ଷର Separator) {
  ଯଦି (Intervals.empty()) {
    OS << "empty";
    ଫେରନ୍ତୁ;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  ପାଇଁ (ଅଚଳ IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  ଯଦି (Intervals.empty())
    ଫେରନ୍ତୁ {};

  IntervalList Result;
  Result.ଠେଲ_back(Intervals[0]);

  ପାଇଁ (ଅଚଳ IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = Result.back();
    // Check if current interval is adjacent to the last merged interval.
    ଯଦି (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } ଅନ୍ୟଥା {
      // Not adjacent, add as separate interval.
      Result.ଠେଲ_back(Current);
    }
  }

  ଫେରନ୍ତୁ Result;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
