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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/IntegerInclusiveInterval.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Error.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Regex.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <string>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਨਾਮਾਕਾਸ਼ llvm::IntegerInclusiveIntervalUtils {

Expected<IntervalList> parseIntervals(StringRef Str, ਅੱਖਰ Separator) {
  IntervalList Intervals;

  ਜੇ (Str.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  ਅਚਲ Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  ਲਈ (StringRef Part : llvm::ਵੰਡੋ(Str, Separator)) {
    Part = Part.trim();
    ਜੇ (Part.ਖਾਲੀ_ਥਾਵਾਂ())
      ਜਾਰੀ;

    Small_ਸਦਿਸ਼<StringRef, 4> Matches;
    ਜੇ (!IntervalRegex.ਮੇਲ(Part, &Matches))
      ਵਾਪਸ createStringError(std::errc::invalid_ਦਲੀਲ,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, ਸਿਰਾ_2;
    ਜੇ (Matches[1].getAsInteger(10, Begin))
      ਵਾਪਸ createStringError(std::errc::invalid_ਦਲੀਲ,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    ਜੇ (!Matches[3].ਖਾਲੀ_ਥਾਵਾਂ()) {
      // Interval format "begin-end".
      ਜੇ (Matches[3].getAsInteger(10, ਸਿਰਾ_2))
        ਵਾਪਸ createStringError(std::errc::invalid_ਦਲੀਲ,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      ਜੇ (Begin >= ਸਿਰਾ_2)
        ਵਾਪਸ createStringError(std::errc::invalid_ਦਲੀਲ,
                                 "Invalid interval: %lld >= %lld", Begin, ਸਿਰਾ_2);
    } ਨਹੀਂਤਾਂ
      // Single number.
      ਸਿਰਾ_2 = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    ਜੇ (!Intervals.ਖਾਲੀ_ਥਾਵਾਂ() && Begin <= Intervals.ਪਿੱਠ().get_ਸਿਰਾ())
      ਵਾਪਸ createStringError(
          std::errc::invalid_ਦਲੀਲ,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.ਪਿੱਠ().get_ਸਿਰਾ());

    Intervals.ਧੱਕੋ_ਪਿੱਠ(IntegerInclusiveInterval(Begin, ਸਿਰਾ_2));
  }

  ਵਾਪਸ Intervals;
}

ਤਾਰਕਿਕ ਸ਼ਾਮਲ_ਹੈ(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t ਮੁੱਲ) {
  ਲਈ (ਅਚਲ IntegerInclusiveInterval &It : Intervals) {
    ਜੇ (It.ਸ਼ਾਮਲ_ਹੈ(ਮੁੱਲ))
      ਵਾਪਸ ਸੱਚ;
  }
  ਵਾਪਸ ਝੂਠ;
}

ਖਾਲੀ printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    ਅੱਖਰ Separator) {
  ਜੇ (Intervals.ਖਾਲੀ_ਥਾਵਾਂ()) {
    OS << "empty";
    ਵਾਪਸ;
  }

  std::string Sep(1, Separator);
  ListSeparator LS(Sep);
  ਲਈ (ਅਚਲ IntegerInclusiveInterval &It : Intervals) {
    OS << LS;
    It.print(OS);
  }
}

IntervalList
ਮਿਲਾਓ_Adjacent_Intervals(ArrayRef<IntegerInclusiveInterval> Intervals) {
  ਜੇ (Intervals.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ {};

  IntervalList ਨਤੀਜਾ;
  ਨਤੀਜਾ.ਧੱਕੋ_ਪਿੱਠ(Intervals[0]);

  ਲਈ (ਅਚਲ IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &ਆਖਰੀ = ਨਤੀਜਾ.ਪਿੱਠ();
    // Check if current interval is adjacent to the last merged interval.
    ਜੇ (Current.getBegin() == ਆਖਰੀ.get_ਸਿਰਾ() + 1) {
      // Merge by extending the end of the last interval.
      ਆਖਰੀ.ਸੈੱਟ_ਸਿਰਾ(Current.get_ਸਿਰਾ());
    } ਨਹੀਂਤਾਂ {
      // Not adjacent, add as separate interval.
      ਨਤੀਜਾ.ਧੱਕੋ_ਪਿੱਠ(Current);
    }
  }

  ਵਾਪਸ ਨਤੀਜਾ;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
