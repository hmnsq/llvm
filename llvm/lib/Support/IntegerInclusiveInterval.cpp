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

  ຖ້າ (Str.empty())
    ສົ່ງຄືນ std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  const Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  ສຳລັບ (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    ຖ້າ (Part.empty())
      continue;

    SmallVector<StringRef, 4> Matches;
    ຖ້າ (!IntervalRegex.match(Part, &Matches))
      ສົ່ງຄືນ createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    ຖ້າ (Matches[1].getAsInteger(10, Begin))
      ສົ່ງຄືນ createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    ຖ້າ (!Matches[3].empty()) {
      // Interval format "begin-end".
      ຖ້າ (Matches[3].getAsInteger(10, End))
        ສົ່ງຄືນ createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      ຖ້າ (Begin >= End)
        ສົ່ງຄືນ createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } ບໍ່ດັ່ງນັ້ນ
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    ຖ້າ (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      ສົ່ງຄືນ createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.push_back(IntegerInclusiveInterval(Begin, End));
  }

  ສົ່ງຄືນ Intervals;
}

bool contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t Value) {
  ສຳລັບ (const IntegerInclusiveInterval &It : Intervals) {
    ຖ້າ (It.contains(Value))
      ສົ່ງຄືນ true;
  }
  ສົ່ງຄືນ false;
}

void printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    var_na Separator) {
  ຖ້າ (Intervals.empty()) {
    OS << "empty";
    ສົ່ງຄືນ;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  ສຳລັບ (const IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  ຖ້າ (Intervals.empty())
    ສົ່ງຄືນ {};

  IntervalList Result;
  Result.push_back(Intervals[0]);

  ສຳລັບ (const IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = Result.back();
    // Check if current interval is adjacent to the last merged interval.
    ຖ້າ (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } ບໍ່ດັ່ງນັ້ນ {
      // Not adjacent, add as separate interval.
      Result.push_back(Current);
    }
  }

  ສົ່ງຄືນ Result;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
