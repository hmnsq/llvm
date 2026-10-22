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

#समावेश "llvm/Support/IntegerInclusiveInterval.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/Support/Error.h"
#समावेश "llvm/Support/Regex.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <string>

using namespace llvm;

namespace llvm::IntegerInclusiveIntervalUtils {

Expected<IntervalList> parseIntervals(StringRef Str, अक्षर Separator) {
  IntervalList Intervals;

  अगर (Str.empty())
    वापस std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  स्थिर Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  क्रम (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    अगर (Part.empty())
      जारी;

    SmallVector<StringRef, 4> Matches;
    अगर (!IntervalRegex.match(Part, &Matches))
      वापस createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    अगर (Matches[1].getAsInteger(10, Begin))
      वापस createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    अगर (!Matches[3].empty()) {
      // Interval format "begin-end".
      अगर (Matches[3].getAsInteger(10, End))
        वापस createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      अगर (Begin >= End)
        वापस createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } अन्यथा
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    अगर (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      वापस createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.धकेलो_back(IntegerInclusiveInterval(Begin, End));
  }

  वापस Intervals;
}

bool contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t मूल्य) {
  क्रम (स्थिर IntegerInclusiveInterval &It : Intervals) {
    अगर (It.contains(मूल्य))
      वापस true;
  }
  वापस false;
}

व्योम printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    अक्षर Separator) {
  अगर (Intervals.empty()) {
    OS << "empty";
    वापस;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  क्रम (स्थिर IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  अगर (Intervals.empty())
    वापस {};

  IntervalList परिणाम;
  परिणाम.धकेलो_back(Intervals[0]);

  क्रम (स्थिर IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = परिणाम.back();
    // Check if current interval is adjacent to the last merged interval.
    अगर (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } अन्यथा {
      // Not adjacent, add as separate interval.
      परिणाम.धकेलो_back(Current);
    }
  }

  वापस परिणाम;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
