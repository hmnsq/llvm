//===- OptionStrCmp.cpp - Option String Comparison --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/Support/OptionStrCmp.h"
#অন্তর্ভাৱ "llvm/ADT/STLExtras.h"

using নামাকাশ llvm;

// Comparison function for Option strings (option names & prefixes).
// The ordering is *almost* case-insensitive lexicographic, with an exception.
// '\0' comes at the end of the alphabet instead of the beginning (thus options
// precede any other options which prefix them). Additionally, if two options
// are identical ignoring case, they are ordered according to case sensitive
// ordering if `FallbackCaseSensitive` is true.
পূর্ণাঙ্ক llvm::StrCmpOptionName(StringRef A, StringRef B,
                           তার্কিক FallbackCaseSensitive) {
  size_t MinSize = std::min(A.size(), B.size());
  যদি (পূর্ণাঙ্ক Res = A.substr(0, MinSize).compare_insensitive(B.substr(0, MinSize)))
    ঘূৰাই Res;

  // If they are identical ignoring case, use case sensitive ordering.
  যদি (A.size() == B.size())
    ঘূৰাই FallbackCaseSensitive ? A.compare(B) : 0;

  ঘূৰাই (A.size() == MinSize) ? 1 /* A is a prefix of B. */
                               : -1 /* B is a prefix of A */;
}

// Comparison function for Option prefixes.
পূর্ণাঙ্ক llvm::StrCmpOptionPrefixes(ArrayRef<StringRef> APrefixes,
                               ArrayRef<StringRef> BPrefixes) {
  বাবে (ধ্রুৱ auto &[APre, BPre] : zip(APrefixes, BPrefixes)) {
    যদি (পূর্ণাঙ্ক Cmp = StrCmpOptionName(APre, BPre))
      ঘূৰাই Cmp;
  }
  // Both prefixes are identical.
  ঘূৰাই 0;
}
