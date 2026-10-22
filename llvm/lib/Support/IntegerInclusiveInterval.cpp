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

#અન્તર્ભાવ "llvm/Support/IntegerInclusiveInterval.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/Support/Error.h"
#અન્તર્ભાવ "llvm/Support/Regex.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <string>

using નામાકાશ llvm;

નામાકાશ llvm::IntegerInclusiveIntervalUtils {

Expected<IntervalList> parseIntervals(StringRef Str, અક્ષર Separator) {
  IntervalList Intervals;

  જો (Str.empty())
    પાછા std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  અચલ Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  માટે (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    જો (Part.empty())
      ચાલુ;

    SmallVector<StringRef, 4> Matches;
    જો (!IntervalRegex.match(Part, &Matches))
      પાછા createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    જો (Matches[1].getAsInteger(10, Begin))
      પાછા createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    જો (!Matches[3].empty()) {
      // Interval format "begin-end".
      જો (Matches[3].getAsInteger(10, End))
        પાછા createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      જો (Begin >= End)
        પાછા createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } નહીંતર
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    જો (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      પાછા createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.ધકેલો_back(IntegerInclusiveInterval(Begin, End));
  }

  પાછા Intervals;
}

તાર્કિક contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t કિંમત) {
  માટે (અચલ IntegerInclusiveInterval &It : Intervals) {
    જો (It.contains(કિંમત))
      પાછા સાચું;
  }
  પાછા ખોટું;
}

શૂન્ય printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    અક્ષર Separator) {
  જો (Intervals.empty()) {
    OS << "empty";
    પાછા;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  માટે (અચલ IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  જો (Intervals.empty())
    પાછા {};

  IntervalList Result;
  Result.ધકેલો_back(Intervals[0]);

  માટે (અચલ IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = Result.back();
    // Check if current interval is adjacent to the last merged interval.
    જો (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } નહીંતર {
      // Not adjacent, add as separate interval.
      Result.ધકેલો_back(Current);
    }
  }

  પાછા Result;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
