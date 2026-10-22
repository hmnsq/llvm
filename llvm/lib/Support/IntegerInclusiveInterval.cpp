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

उपयुज्य नामाकाश llvm;

नामाकाश llvm::IntegerInclusiveIntervalUtils {

Expected<IntervalList> parseIntervals(StringRef Str, वर्ण Separator) {
  IntervalList Intervals;

  यदि (Str.रिक्तानि())
    प्रत्यागम std::move(Intervals);

  // Regex to match either single number or interval "num1-num2".
  ध्रुव Regex IntervalRegex("^([0-9]+)(-([0-9]+))?$");

  प्रति (StringRef Part : llvm::विभज(Str, Separator)) {
    Part = Part.trim();
    यदि (Part.रिक्तानि())
      अनुवर्त;

    Small_सदिशम्<StringRef, 4> Matches;
    यदि (!IntervalRegex.मेलनम्(Part, &Matches))
      प्रत्यागम createStringError(std::errc::invalid_तर्कः,
                               "Invalid interval format: '%s'",
                               Part.str().c_str());

    int64_t Begin, अन्तः_2;
    यदि (Matches[1].getAsInteger(10, Begin))
      प्रत्यागम createStringError(std::errc::invalid_तर्कः,
                               "Failed to parse number: '%s'",
                               Matches[1].str().c_str());

    यदि (!Matches[3].रिक्तानि()) {
      // Interval format "begin-end".
      यदि (Matches[3].getAsInteger(10, अन्तः_2))
        प्रत्यागम createStringError(std::errc::invalid_तर्कः,
                                 "Failed to parse number: '%s'",
                                 Matches[3].str().c_str());
      यदि (Begin >= अन्तः_2)
        प्रत्यागम createStringError(std::errc::invalid_तर्कः,
                                 "Invalid interval: %lld >= %lld", Begin, अन्तः_2);
    } अन्यथा
      // Single number.
      अन्तः_2 = Begin;

    // Check ordering constraint (intervals must be in increasing order).
    यदि (!Intervals.रिक्तानि() && Begin <= Intervals.पृष्ठभागः().get_अन्तः())
      प्रत्यागम createStringError(
          std::errc::invalid_तर्कः,
          "Expected intervals to be in increasing order: %lld <= %lld", Begin,
          Intervals.पृष्ठभागः().get_अन्तः());

    Intervals.प्रक्षिप_पृष्ठभागः(IntegerInclusiveInterval(Begin, अन्तः_2));
  }

  प्रत्यागम Intervals;
}

तार्किक अन्तर्भवति(ArrayRef<IntegerInclusiveInterval> Intervals, int64_t मानम्) {
  प्रति (ध्रुव IntegerInclusiveInterval &It : Intervals) {
    यदि (It.अन्तर्भवति(मानम्))
      प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

शून्य printIntervals(raw_ostream &OS,
                    ArrayRef<IntegerInclusiveInterval> Intervals,
                    वर्ण Separator) {
  यदि (Intervals.रिक्तानि()) {
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
  यदि (Intervals.रिक्तानि())
    प्रत्यागम {};

  IntervalList परिणाम;
  परिणाम.प्रक्षिप_पृष्ठभागः(Intervals[0]);

  प्रति (ध्रुव IntegerInclusiveInterval &Current : Intervals.drop_front()) {
    IntegerInclusiveInterval &चरमः = परिणाम.पृष्ठभागः();
    // Check if current interval is adjacent to the last merged interval.
    यदि (Current.getBegin() == चरमः.get_अन्तः() + 1) {
      // Merge by extending the end of the last interval.
      चरमः.समुच्चयः_अन्तः(Current.get_अन्तः());
    } अन्यथा {
      // Not adjacent, add as separate interval.
      परिणाम.प्रक्षिप_पृष्ठभागः(Current);
    }
  }

  प्रत्यागम परिणाम;
}

} // end namespace llvm::IntegerInclusiveIntervalUtils
