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

#ಅನ್ತರ್ಭಾವ "llvm/Support/IntegerInclusiveInterval.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Regex.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <string>

using ನಾಮಾಕಾಶ llvm;

ನಾಮಾಕಾಶ llvm::IntegerInclusiveIntervalUtils {

Expected<IntervalList> parseIntervals(StringRef Str, ವರ್ಣ Separator) {
  IntervalList Intervals;

  ಯದಿ (Str.empty())
    ಪ್ರತ್ಯಾಗಮ std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  ಧ್ರುವ Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  ಪ್ರತಿ (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    ಯದಿ (Part.empty())
      ಅನುವರ್ತ;

    SmallVector<StringRef, 4> Matches;
    ಯದಿ (!IntervalRegex.match(Part, &Matches))
      ಪ್ರತ್ಯಾಗಮ createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    ಯದಿ (Matches[1].getAsInteger(10, Begin))
      ಪ್ರತ್ಯಾಗಮ createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    ಯದಿ (!Matches[3].empty()) {
      // Interval format "begin-end".
      ಯದಿ (Matches[3].getAsInteger(10, End))
        ಪ್ರತ್ಯಾಗಮ createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      ಯದಿ (Begin >= End)
        ಪ್ರತ್ಯಾಗಮ createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } ಅನ್ಯಥಾ
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    ಯದಿ (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      ಪ್ರತ್ಯಾಗಮ createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.ತಳ್ಳು_back(IntegerInclusiveInterval(Begin, End));
  }

  ಪ್ರತ್ಯಾಗಮ Intervals;
}

ತಾರ್ಕಿಕ contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t ಮೌಲ್ಯ) {
  ಪ್ರತಿ (ಧ್ರುವ IntegerInclusiveInterval &It : Intervals) {
    ಯದಿ (It.contains(ಮೌಲ್ಯ))
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ಶೂನ್ಯ printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    ವರ್ಣ Separator) {
  ಯದಿ (Intervals.empty()) {
    OS << "empty";
    ಪ್ರತ್ಯಾಗಮ;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  ಪ್ರತಿ (ಧ್ರುವ IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  ಯದಿ (Intervals.empty())
    ಪ್ರತ್ಯಾಗಮ {};

  IntervalList Result;
  Result.ತಳ್ಳು_back(Intervals[0]);

  ಪ್ರತಿ (ಧ್ರುವ IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = Result.back();
    // Check if current interval is adjacent to the last merged interval.
    ಯದಿ (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } ಅನ್ಯಥಾ {
      // Not adjacent, add as separate interval.
      Result.ತಳ್ಳು_back(Current);
    }
  }

  ಪ್ರತ್ಯಾಗಮ Result;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
