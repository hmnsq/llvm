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

Expected<IntervalList> parseIntervals(StringRef Str, char Separator) {
  IntervalList Intervals;

  만약 (Str.empty())
    반환 std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  const Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  위해 (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    만약 (Part.empty())
      continue;

    SmallVector<StringRef, 4> Matches;
    만약 (!IntervalRegex.match(Part, &Matches))
      반환 createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    만약 (Matches[1].getAsInteger(10, Begin))
      반환 createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    만약 (!Matches[3].empty()) {
      // Interval format "begin-end".
      만약 (Matches[3].getAsInteger(10, End))
        반환 createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      만약 (Begin >= End)
        반환 createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } 아니면
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    만약 (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      반환 createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.push_back(IntegerInclusiveInterval(Begin, End));
  }

  반환 Intervals;
}

bool contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t Value) {
  위해 (const IntegerInclusiveInterval &It : Intervals) {
    만약 (It.contains(Value))
      반환 true;
  }
  반환 false;
}

void printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    char Separator) {
  만약 (Intervals.empty()) {
    OS << "empty";
    반환;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  위해 (const IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  만약 (Intervals.empty())
    반환 {};

  IntervalList Result;
  Result.push_back(Intervals[0]);

  위해 (const IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = Result.back();
    // Check if current interval is adjacent to the last merged interval.
    만약 (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } 아니면 {
      // Not adjacent, add as separate interval.
      Result.push_back(Current);
    }
  }

  반환 Result;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
