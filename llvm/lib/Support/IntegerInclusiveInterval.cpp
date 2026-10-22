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

#شامل "llvm/Support/IntegerInclusiveInterval.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Support/Error.h"
#شامل "llvm/Support/Regex.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <string>

استعمال نامخلا llvm;

نامخلا llvm::IntegerInclusiveIntervalUtils {

Expected<IntervalList> parseIntervals(StringRef Str, حرف Separator) {
  IntervalList Intervals;

  اگر (Str.خالی_خانے())
    واپس std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  ثابت Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  برائے (StringRef Part : llvm::بانٹو(Str, Separator)) {
    Part = Part.trim();
    اگر (Part.خالی_خانے())
      جاری;

    Small_سمتیہ<StringRef, 4> Matches;
    اگر (!IntervalRegex.مطابقت(Part, &Matches))
      واپس createStringError(std::errc::invalid_دلیل,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, انتہا_2;
    اگر (Matches[1].getAsInteger(10, Begin))
      واپس createStringError(std::errc::invalid_دلیل,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    اگر (!Matches[3].خالی_خانے()) {
      // Interval format "begin-end".
      اگر (Matches[3].getAsInteger(10, انتہا_2))
        واپس createStringError(std::errc::invalid_دلیل,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      اگر (Begin >= انتہا_2)
        واپس createStringError(std::errc::invalid_دلیل,
                                 "Invalid interval: %lld >= %lld", Begin, انتہا_2);
    } ورنہ
      // Single number.
      انتہا_2 = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    اگر (!Intervals.خالی_خانے() && Begin <= Intervals.عقب().get_انتہا())
      واپس createStringError(
          std::errc::invalid_دلیل,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.عقب().get_انتہا());

    Intervals.ڈالو_عقب(IntegerInclusiveInterval(Begin, انتہا_2));
  }

  واپس Intervals;
}

منطقی شامل_ہے(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t قدر) {
  برائے (ثابت IntegerInclusiveInterval &It : Intervals) {
    اگر (It.شامل_ہے(قدر))
      واپس سچ;
  }
  واپس جھوٹ;
}

خالی printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    حرف Separator) {
  اگر (Intervals.خالی_خانے()) {
    OS << "empty";
    واپس;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  برائے (ثابت IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
ملاؤ_Adjacent_Intervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  اگر (Intervals.خالی_خانے())
    واپس {};

  IntervalList نتیجہ;
  نتیجہ.ڈالو_عقب(Intervals[0]);

  برائے (ثابت IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &آخری = نتیجہ.عقب();
    // Check if current interval is adjacent to the last merged interval.
    اگر (Current.getBegin() == آخری.get_انتہا() + 1) {
      // Merge by extending the end of the last interval.
      آخری.سیٹ_انتہا(Current.get_انتہا());
    } ورنہ {
      // Not adjacent, add as separate interval.
      نتیجہ.ڈالو_عقب(Current);
    }
  }

  واپس نتیجہ;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
