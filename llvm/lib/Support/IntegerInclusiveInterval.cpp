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

Expected<IntervalList> parseIntervals(StringRef Str, अक्षर Separator) {
  IntervalList Intervals;

  जर (Str.empty())
    परत std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  स्थिरांक Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  साठी (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    जर (Part.empty())
      सुरूठेवा;

    SmallVector<StringRef, 4> Matches;
    जर (!IntervalRegex.match(Part, &Matches))
      परत createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    जर (Matches[1].getAsInteger(10, Begin))
      परत createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    जर (!Matches[3].empty()) {
      // Interval format "begin-end".
      जर (Matches[3].getAsInteger(10, End))
        परत createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      जर (Begin >= End)
        परत createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } अन्यथा
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    जर (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      परत createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.ढकला_back(IntegerInclusiveInterval(Begin, End));
  }

  परत Intervals;
}

तार्किक contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t मूल्य) {
  साठी (स्थिरांक IntegerInclusiveInterval &It : Intervals) {
    जर (It.contains(मूल्य))
      परत सत्य;
  }
  परत असत्य;
}

शून्य printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    अक्षर Separator) {
  जर (Intervals.empty()) {
    OS << "empty";
    परत;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  साठी (स्थिरांक IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  जर (Intervals.empty())
    परत {};

  IntervalList Result;
  Result.ढकला_back(Intervals[0]);

  साठी (स्थिरांक IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = Result.back();
    // Check if current interval is adjacent to the last merged interval.
    जर (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } अन्यथा {
      // Not adjacent, add as separate interval.
      Result.ढकला_back(Current);
    }
  }

  परत Result;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
