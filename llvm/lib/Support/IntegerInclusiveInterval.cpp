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

#include "llvm/Support/IntegerInclusiveInterval.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/Regex.h"
#include "llvm/Support/raw_ostream.h"
#include <string>

using namespace llvm;

namespace llvm::IntegerInclusiveIntervalUtils {

Expected<IntervalList> parseIntervals(StringRef Str, var_na Separator) {
  IntervalList Intervals;

  නම් (Str.empty())
    ආපසු std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  const Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  සඳහා (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    නම් (Part.empty())
      continue;

    SmallVector<StringRef, 4> Matches;
    නම් (!IntervalRegex.match(Part, &Matches))
      ආපසු createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    නම් (Matches[1].getAsInteger(10, Begin))
      ආපසු createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    නම් (!Matches[3].empty()) {
      // Interval format "begin-end".
      නම් (Matches[3].getAsInteger(10, End))
        ආපසු createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      නම් (Begin >= End)
        ආපසු createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } නැතහොත්
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    නම් (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      ආපසු createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.push_back(IntegerInclusiveInterval(Begin, End));
  }

  ආපසු Intervals;
}

bool contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t Value) {
  සඳහා (const IntegerInclusiveInterval &It : Intervals) {
    නම් (It.contains(Value))
      ආපසු true;
  }
  ආපසු false;
}

void printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    var_na Separator) {
  නම් (Intervals.empty()) {
    OS << "empty";
    ආපසු;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  සඳහා (const IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  නම් (Intervals.empty())
    ආපසු {};

  IntervalList Result;
  Result.push_back(Intervals[0]);

  සඳහා (const IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = Result.back();
    // Check if current interval is adjacent to the last merged interval.
    නම් (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } නැතහොත් {
      // Not adjacent, add as separate interval.
      Result.push_back(Current);
    }
  }

  ආපසු Result;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
