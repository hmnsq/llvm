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

استفاده نطاق llvm;

نطاق llvm::IntegerInclusiveIntervalUtils {

Expected<IntervalList> parseIntervals(StringRef Str, حرف Separator) {
  IntervalList Intervals;

  إذا (Str.خالی())
    أرجع std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  ثابت Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  لكل (StringRef Part : llvm::تقسیم(Str, Separator)) {
    Part = Part.trim();
    إذا (Part.خالی())
      استمر;

    Small_وکتور<StringRef, 4> Matches;
    إذا (!IntervalRegex.تطابق(Part, &Matches))
      أرجع createStringError(std::errc::invalid_آرگومان,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, انتها_2;
    إذا (Matches[1].getAsInteger(10, Begin))
      أرجع createStringError(std::errc::invalid_آرگومان,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    إذا (!Matches[3].خالی()) {
      // Interval format "begin-end".
      إذا (Matches[3].getAsInteger(10, انتها_2))
        أرجع createStringError(std::errc::invalid_آرگومان,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      إذا (Begin >= انتها_2)
        أرجع createStringError(std::errc::invalid_آرگومان,
                                 "Invalid interval: %lld >= %lld", Begin, انتها_2);
    } وإلا
      // Single number.
      انتها_2 = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    إذا (!Intervals.خالی() && Begin <= Intervals.پشت().get_انتها())
      أرجع createStringError(
          std::errc::invalid_آرگومان,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.پشت().get_انتها());

    Intervals.افزودن_پشت(IntegerInclusiveInterval(Begin, انتها_2));
  }

  أرجع Intervals;
}

منطقي شامل_است(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t مقدار) {
  لكل (ثابت IntegerInclusiveInterval &It : Intervals) {
    إذا (It.شامل_است(مقدار))
      أرجع درست;
  }
  أرجع خطأ;
}

فراغ printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    حرف Separator) {
  إذا (Intervals.خالی()) {
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
ادغام_Adjacent_Intervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  إذا (Intervals.خالی())
    أرجع {};

  IntervalList نتیجه;
  نتیجه.افزودن_پشت(Intervals[0]);

  لكل (ثابت IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &آخرین = نتیجه.پشت();
    // Check if current interval is adjacent to the last merged interval.
    إذا (Current.getBegin() == آخرین.get_انتها() + 1) {
      // Merge by extending the end of the last interval.
      آخرین.مجموعه_انتها(Current.get_انتها());
    } وإلا {
      // Not adjacent, add as separate interval.
      نتیجه.افزودن_پشت(Current);
    }
  }

  أرجع نتیجه;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
