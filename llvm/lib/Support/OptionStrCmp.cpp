//===- OptionStrCmp.cpp - Option String Comparison --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/OptionStrCmp.h"
#অন্তর্ভাব "llvm/ADT/STLExtras.h"

using নামাকাশ llvm;

// Comparison function for Option strings (option names & prefixes).
// The ordering is *almost* case-insensitive lexicographic, with an exception.
// '\0' comes at the end of the alphabet instead of the beginning (thus options
// precede any other options which prefix them). Additionally, if two options
// are identical ignoring case, they are ordered according to case sensitive
// ordering if `FallbackCaseSensitive` is true.
পূর্ণসংখ্যা llvm::StrCmpOptionName(StringRef A, StringRef B,
                           যৌক্তিক FallbackCaseSensitive) {
  size_t Min_আকার = std::min(A.আকার_3(), B.আকার_3());
  যদি (পূর্ণসংখ্যা Res = A.substr(0, Min_আকার).compare_insensitive(B.substr(0, Min_আকার)))
    ফেরত Res;

  // If they are identical ignoring case, use case sensitive ordering.
  যদি (A.আকার_3() == B.আকার_3())
    ফেরত FallbackCaseSensitive ? A.compare(B) : 0;

  ফেরত (A.আকার_3() == Min_আকার) ? 1 /* A is a prefix of B. */
                               : -1 /* B is a prefix of A */;
}

// Comparison function for Option prefixes.
পূর্ণসংখ্যা llvm::StrCmpOptionPrefixes(ArrayRef<StringRef> APrefixes,
                               ArrayRef<StringRef> BPrefixes) {
  জন্য (ধ্রুবক auto &[APre, BPre] : zip(APrefixes, BPrefixes)) {
    যদি (পূর্ণসংখ্যা Cmp = StrCmpOptionName(APre, BPre))
      ফেরত Cmp;
  }
  // Both prefixes are identical.
  ফেরত 0;
}
