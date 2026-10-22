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

  यदि (Str.empty())
    प्रत्यागम std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  ध्रुव Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  प्रति (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    यदि (Part.empty())
      अनुवर्त;

    SmallVector<StringRef, 4> Matches;
    यदि (!IntervalRegex.match(Part, &Matches))
      प्रत्यागम createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    यदि (Matches[1].getAsInteger(10, Begin))
      प्रत्यागम createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    यदि (!Matches[3].empty()) {
      // Interval format "begin-end".
      यदि (Matches[3].getAsInteger(10, End))
        प्रत्यागम createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      यदि (Begin >= End)
        प्रत्यागम createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } अन्यथा
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    यदि (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      प्रत्यागम createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.ढकला_back(IntegerInclusiveInterval(Begin, End));
  }

  प्रत्यागम Intervals;
}

तार्किक contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t मूल्य) {
  प्रति (ध्रुव IntegerInclusiveInterval &It : Intervals) {
    यदि (It.contains(मूल्य))
      प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

शून्य printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    वर्ण Separator) {
  यदि (Intervals.empty()) {
    OS << "empty";
    प्रत्यागम;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  प्रति (ध्रुव IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  यदि (Intervals.empty())
    प्रत्यागम {};

  IntervalList Result;
  Result.ढकला_back(Intervals[0]);

  प्रति (ध्रुव IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = Result.back();
    // Check if current interval is adjacent to the last merged interval.
    यदि (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } अन्यथा {
      // Not adjacent, add as separate interval.
      Result.ढकला_back(Current);
    }
  }

  प्रत्यागम Result;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
