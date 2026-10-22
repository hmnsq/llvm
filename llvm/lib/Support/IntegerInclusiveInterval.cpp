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

#ضمن "llvm/Support/IntegerInclusiveInterval.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/Regex.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <string>

ورتوں نطاق llvm;

نطاق llvm::IntegerInclusiveIntervalUtils {

Expected<IntervalList> parseIntervals(StringRef Str, حرف Separator) {
  IntervalList Intervals;

  إذا (Str.خالی_تھاواں())
    أرجع std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  ثابت Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  لكل (StringRef Part : llvm::ونڈو(Str, Separator)) {
    Part = Part.trim();
    إذا (Part.خالی_تھاواں())
      استمر;

    Small_سمتیہ<StringRef, 4> Matches;
    إذا (!IntervalRegex.میل(Part, &Matches))
      أرجع createStringError(std::errc::invalid_دلیل,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, سرا_2;
    إذا (Matches[1].getAsInteger(10, Begin))
      أرجع createStringError(std::errc::invalid_دلیل,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    إذا (!Matches[3].خالی_تھاواں()) {
      // Interval format "begin-end".
      إذا (Matches[3].getAsInteger(10, سرا_2))
        أرجع createStringError(std::errc::invalid_دلیل,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      إذا (Begin >= سرا_2)
        أرجع createStringError(std::errc::invalid_دلیل,
                                 "Invalid interval: %lld >= %lld", Begin, سرا_2);
    } وإلا
      // Single number.
      سرا_2 = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    إذا (!Intervals.خالی_تھاواں() && Begin <= Intervals.پٹھ().get_سرا())
      أرجع createStringError(
          std::errc::invalid_دلیل,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.پٹھ().get_سرا());

    Intervals.ڈالو_پٹھ(IntegerInclusiveInterval(Begin, سرا_2));
  }

  أرجع Intervals;
}

منطقي شامل_اے(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t قدر) {
  لكل (ثابت IntegerInclusiveInterval &It : Intervals) {
    إذا (It.شامل_اے(قدر))
      أرجع سچ;
  }
  أرجع خطأ;
}

فراغ printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    حرف Separator) {
  إذا (Intervals.خالی_تھاواں()) {
    OS << "empty";
    أرجع;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  لكل (ثابت IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
ملاؤ_Adjacent_Intervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  إذا (Intervals.خالی_تھاواں())
    أرجع {};

  IntervalList نتیجہ;
  نتیجہ.ڈالو_پٹھ(Intervals[0]);

  لكل (ثابت IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &آخری = نتیجہ.پٹھ();
    // Check if current interval is adjacent to the last merged interval.
    إذا (Current.getBegin() == آخری.get_سرا() + 1) {
      // Merge by extending the end of the last interval.
      آخری.سیٹ_سرا(Current.get_سرا());
    } وإلا {
      // Not adjacent, add as separate interval.
      نتیجہ.ڈالو_پٹھ(Current);
    }
  }

  أرجع نتیجہ;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
