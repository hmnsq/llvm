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

استعمال نطاق llvm;

نطاق llvm::IntegerInclusiveIntervalUtils {

Expected<IntervalList> parseIntervals(StringRef Str, حرف Separator) {
  IntervalList Intervals;

  إذا (Str.خالي_جايون())
    أرجع std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  ثابت Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  لكل (StringRef Part : llvm::ورهايو(Str, Separator)) {
    Part = Part.trim();
    إذا (Part.خالي_جايون())
      استمر;

    Small_ويڪٽر<StringRef, 4> Matches;
    إذا (!IntervalRegex.ميلاپ(Part, &Matches))
      أرجع createStringError(std::errc::invalid_دليل,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, پڇاڙي_2;
    إذا (Matches[1].getAsInteger(10, Begin))
      أرجع createStringError(std::errc::invalid_دليل,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    إذا (!Matches[3].خالي_جايون()) {
      // Interval format "begin-end".
      إذا (Matches[3].getAsInteger(10, پڇاڙي_2))
        أرجع createStringError(std::errc::invalid_دليل,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      إذا (Begin >= پڇاڙي_2)
        أرجع createStringError(std::errc::invalid_دليل,
                                 "Invalid interval: %lld >= %lld", Begin, پڇاڙي_2);
    } وإلا
      // Single number.
      پڇاڙي_2 = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    إذا (!Intervals.خالي_جايون() && Begin <= Intervals.پٺ().get_پڇاڙي())
      أرجع createStringError(
          std::errc::invalid_دليل,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.پٺ().get_پڇاڙي());

    Intervals.ڌڪو_پٺ(IntegerInclusiveInterval(Begin, پڇاڙي_2));
  }

  أرجع Intervals;
}

منطقي شامل_آهي(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t قدر) {
  لكل (ثابت IntegerInclusiveInterval &It : Intervals) {
    إذا (It.شامل_آهي(قدر))
      أرجع سچ;
  }
  أرجع خطأ;
}

فراغ printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    حرف Separator) {
  إذا (Intervals.خالي_جايون()) {
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
ملايو_Adjacent_Intervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  إذا (Intervals.خالي_جايون())
    أرجع {};

  IntervalList نتيجو;
  نتيجو.ڌڪو_پٺ(Intervals[0]);

  لكل (ثابت IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &آخري = نتيجو.پٺ();
    // Check if current interval is adjacent to the last merged interval.
    إذا (Current.getBegin() == آخري.get_پڇاڙي() + 1) {
      // Merge by extending the end of the last interval.
      آخري.سيٽ_پڇاڙي(Current.get_پڇاڙي());
    } وإلا {
      // Not adjacent, add as separate interval.
      نتيجو.ڌڪو_پٺ(Current);
    }
  }

  أرجع نتيجو;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
