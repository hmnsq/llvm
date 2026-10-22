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

  ᱡᱩᱫᱤ (Str.empty())
    ᱨᱩᱣᱟᱹᱲ std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  const Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  for (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    ᱡᱩᱫᱤ (Part.empty())
      continue;

    SmallVector<StringRef, 4> Matches;
    ᱡᱩᱫᱤ (!IntervalRegex.match(Part, &Matches))
      ᱨᱩᱣᱟᱹᱲ createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    ᱡᱩᱫᱤ (Matches[1].getAsInteger(10, Begin))
      ᱨᱩᱣᱟᱹᱲ createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    ᱡᱩᱫᱤ (!Matches[3].empty()) {
      // Interval format "begin-end".
      ᱡᱩᱫᱤ (Matches[3].getAsInteger(10, End))
        ᱨᱩᱣᱟᱹᱲ createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      ᱡᱩᱫᱤ (Begin >= End)
        ᱨᱩᱣᱟᱹᱲ createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } ᱵᱟᱝ
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    ᱡᱩᱫᱤ (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      ᱨᱩᱣᱟᱹᱲ createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.push_back(IntegerInclusiveInterval(Begin, End));
  }

  ᱨᱩᱣᱟᱹᱲ Intervals;
}

bool contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t Value) {
  for (const IntegerInclusiveInterval &It : Intervals) {
    ᱡᱩᱫᱤ (It.contains(Value))
      ᱨᱩᱣᱟᱹᱲ true;
  }
  ᱨᱩᱣᱟᱹᱲ false;
}

void printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    var_na Separator) {
  ᱡᱩᱫᱤ (Intervals.empty()) {
    OS << "empty";
    ᱨᱩᱣᱟᱹᱲ;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  for (const IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  ᱡᱩᱫᱤ (Intervals.empty())
    ᱨᱩᱣᱟᱹᱲ {};

  IntervalList Result;
  Result.push_back(Intervals[0]);

  for (const IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = Result.back();
    // Check if current interval is adjacent to the last merged interval.
    ᱡᱩᱫᱤ (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } ᱵᱟᱝ {
      // Not adjacent, add as separate interval.
      Result.push_back(Current);
    }
  }

  ᱨᱩᱣᱟᱹᱲ Result;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
