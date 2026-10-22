//===- OptionStrCmp.cpp - Option String Comparison --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/OptionStrCmp.h"
#అన్తర్భావ "llvm/ADT/STLExtras.h"

ఉపయోగించి నామాకాశ llvm;

// Comparison function for Option strings (option names & prefixes).
// The ordering is *almost* case-insensitive lexicographic, with an exception.
// '\0' comes at the end of the alphabet instead of the beginning (thus options
// precede any other options which prefix them). Additionally, if two options
// are identical ignoring case, they are ordered according to case sensitive
// ordering if `FallbackCaseSensitive` is true.
పూర్ణాంకం llvm::StrCmpOptionName(StringRef A, StringRef B,
                           తార్కిక FallbackCaseSensitive) {
  size_t Min_కొలత = std::min(A.కొలత_2(), B.కొలత_2());
  ఒకవేళ (పూర్ణాంకం Res = A.substr(0, Min_కొలత).compare_insensitive(B.substr(0, Min_కొలత)))
    తిరిగి Res;

  // If they are identical ignoring case, use case sensitive ordering.
  ఒకవేళ (A.కొలత_2() == B.కొలత_2())
    తిరిగి FallbackCaseSensitive ? A.compare(B) : 0;

  తిరిగి (A.కొలత_2() == Min_కొలత) ? 1 /* A is a prefix of B. */
                               : -1 /* B is a prefix of A */;
}

// Comparison function for Option prefixes.
పూర్ణాంకం llvm::StrCmpOptionPrefixes(ArrayRef<StringRef> APrefixes,
                               ArrayRef<StringRef> BPrefixes) {
  కొరకు (స్థిరరాశి స్వయంచాలక &[APre, BPre] : zip(APrefixes, BPrefixes)) {
    ఒకవేళ (పూర్ణాంకం Cmp = StrCmpOptionName(APre, BPre))
      తిరిగి Cmp;
  }
  // Both prefixes are identical.
  తిరిగి 0;
}
