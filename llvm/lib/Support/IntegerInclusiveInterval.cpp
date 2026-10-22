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

  ถ้า (Str.empty())
    คืน std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  const Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  สำหรับ (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    ถ้า (Part.empty())
      continue;

    SmallVector<StringRef, 4> Matches;
    ถ้า (!IntervalRegex.match(Part, &Matches))
      คืน createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    ถ้า (Matches[1].getAsInteger(10, Begin))
      คืน createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    ถ้า (!Matches[3].empty()) {
      // Interval format "begin-end".
      ถ้า (Matches[3].getAsInteger(10, End))
        คืน createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      ถ้า (Begin >= End)
        คืน createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } มิฉะนั้น
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    ถ้า (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      คืน createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.push_back(IntegerInclusiveInterval(Begin, End));
  }

  คืน Intervals;
}

bool contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t Value) {
  สำหรับ (const IntegerInclusiveInterval &It : Intervals) {
    ถ้า (It.contains(Value))
      คืน true;
  }
  คืน false;
}

void printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    var_na Separator) {
  ถ้า (Intervals.empty()) {
    OS << "empty";
    คืน;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  สำหรับ (const IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  ถ้า (Intervals.empty())
    คืน {};

  IntervalList Result;
  Result.push_back(Intervals[0]);

  สำหรับ (const IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = Result.back();
    // Check if current interval is adjacent to the last merged interval.
    ถ้า (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } มิฉะนั้น {
      // Not adjacent, add as separate interval.
      Result.push_back(Current);
    }
  }

  คืน Result;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
