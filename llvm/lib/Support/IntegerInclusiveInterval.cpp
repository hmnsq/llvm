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

#अन्तर्भाव "llvm/Support/IntegerInclusiveInterval.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Support/Error.h"
#अन्तर्भाव "llvm/Support/Regex.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <string>

using नामाकाश llvm;

नामाकाश llvm::IntegerInclusiveIntervalUtils {

Expected<IntervalList> parseIntervals(StringRef Str, वर्ण Separator) {
  IntervalList Intervals;

  सचे (Str.empty())
    पटिनिवत्त std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  ध्रुव Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  अत्थाय (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    सचे (Part.empty())
      अनुवर्त;

    SmallVector<StringRef, 4> Matches;
    सचे (!IntervalRegex.match(Part, &Matches))
      पटिनिवत्त createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    सचे (Matches[1].getAsInteger(10, Begin))
      पटिनिवत्त createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    सचे (!Matches[3].empty()) {
      // Interval format "begin-end".
      सचे (Matches[3].getAsInteger(10, End))
        पटिनिवत्त createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      सचे (Begin >= End)
        पटिनिवत्त createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } अञ्ञथा
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    सचे (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      पटिनिवत्त createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.प्रक्षिप_back(IntegerInclusiveInterval(Begin, End));
  }

  पटिनिवत्त Intervals;
}

तार्किक contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t मानम्) {
  अत्थाय (ध्रुव IntegerInclusiveInterval &It : Intervals) {
    सचे (It.contains(मानम्))
      पटिनिवत्त सच्च;
  }
  पटिनिवत्त मिच्छा;
}

शून्य printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    वर्ण Separator) {
  सचे (Intervals.empty()) {
    OS << "empty";
    पटिनिवत्त;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  अत्थाय (ध्रुव IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  सचे (Intervals.empty())
    पटिनिवत्त {};

  IntervalList Result;
  Result.प्रक्षिप_back(Intervals[0]);

  अत्थाय (ध्रुव IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = Result.back();
    // Check if current interval is adjacent to the last merged interval.
    सचे (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } अञ्ञथा {
      // Not adjacent, add as separate interval.
      Result.प्रक्षिप_back(Current);
    }
  }

  पटिनिवत्त Result;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
