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

#அந்தர்பாவ "llvm/Support/IntegerInclusiveInterval.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/Support/Error.h"
#அந்தர்பாவ "llvm/Support/Regex.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <string>

using நாமாகாஶ llvm;

நாமாகாஶ llvm::IntegerInclusiveIntervalUtils {

Expected<IntervalList> parseIntervals(StringRef Str, எழுத்து Separator) {
  IntervalList Intervals;

  எனில் (Str.empty())
    திரும்பு std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  மாறிலி Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  க்கு (StringRef Part : llvm::split(Str, Separator)) {
    Part = Part.trim();
    எனில் (Part.empty())
      தொடர்;

    SmallVector<StringRef, 4> Matches;
    எனில் (!IntervalRegex.match(Part, &Matches))
      திரும்பு createStringError(std::errc::invalid_argument,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, End;
    எனில் (Matches[1].getAsInteger(10, Begin))
      திரும்பு createStringError(std::errc::invalid_argument,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    எனில் (!Matches[3].empty()) {
      // Interval format "begin-end".
      எனில் (Matches[3].getAsInteger(10, End))
        திரும்பு createStringError(std::errc::invalid_argument,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      எனில் (Begin >= End)
        திரும்பு createStringError(std::errc::invalid_argument,
                                 "Invalid interval: %lld >= %lld", Begin, End);
    } இல்லையெனில்
      // Single number.
      End = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    எனில் (!Intervals.empty() && Begin <= Intervals.back().getEnd())
      திரும்பு createStringError(
          std::errc::invalid_argument,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.back().getEnd());

    Intervals.தள்ளு_back(IntegerInclusiveInterval(Begin, End));
  }

  திரும்பு Intervals;
}

தருக்க contains(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t மதிப்பு) {
  க்கு (மாறிலி IntegerInclusiveInterval &It : Intervals) {
    எனில் (It.contains(மதிப்பு))
      திரும்பு உண்மை;
  }
  திரும்பு பொய்;
}

வெறுமை printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    எழுத்து Separator) {
  எனில் (Intervals.empty()) {
    OS << "empty";
    திரும்பு;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  க்கு (மாறிலி IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
mergeAdjacentIntervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  எனில் (Intervals.empty())
    திரும்பு {};

  IntervalList Result;
  Result.தள்ளு_back(Intervals[0]);

  க்கு (மாறிலி IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &Last = Result.back();
    // Check if current interval is adjacent to the last merged interval.
    எனில் (Current.getBegin() == Last.getEnd() + 1) {
      // Merge by extending the end of the last interval.
      Last.setEnd(Current.getEnd());
    } இல்லையெனில் {
      // Not adjacent, add as separate interval.
      Result.தள்ளு_back(Current);
    }
  }

  திரும்பு Result;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
