//===- OptionStrCmp.cpp - Option String Comparison --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/OptionStrCmp.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"

using नामाकाश llvm;

// Comparison function for Option strings (option names & prefixes).
// The ordering is *almost* case-insensitive lexicographic, with an exception.
// '\0' comes at the end of the alphabet instead of the beginning (thus options
// precede any other options which prefix them). Additionally, if two options
// are identical ignoring case, they are ordered according to case sensitive
// ordering if `FallbackCaseSensitive` is true.
पूर्णाङ्क llvm::StrCmpOptionName(StringRef A, StringRef B,
                           तार्किक FallbackCaseSensitive) {
  size_t MinSize = std::min(A.size(), B.size());
  सचे (पूर्णाङ्क Res = A.substr(0, MinSize).compare_insensitive(B.substr(0, MinSize)))
    पटिनिवत्त Res;

  // If they are identical ignoring case, use case sensitive ordering.
  सचे (A.size() == B.size())
    पटिनिवत्त FallbackCaseSensitive ? A.compare(B) : 0;

  पटिनिवत्त (A.size() == MinSize) ? 1 /* A is a prefix of B. */
                               : -1 /* B is a prefix of A */;
}

// Comparison function for Option prefixes.
पूर्णाङ्क llvm::StrCmpOptionPrefixes(ArrayRef<StringRef> APrefixes,
                               ArrayRef<StringRef> BPrefixes) {
  अत्थाय (ध्रुव auto &[APre, BPre] : zip(APrefixes, BPrefixes)) {
    सचे (पूर्णाङ्क Cmp = StrCmpOptionName(APre, BPre))
      पटिनिवत्त Cmp;
  }
  // Both prefixes are identical.
  पटिनिवत्त 0;
}
