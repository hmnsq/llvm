//===- OptionStrCmp.cpp - Option String Comparison --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/OptionStrCmp.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"

using ನಾಮಾಕಾಶ llvm;

// Comparison function for Option strings (option names & prefixes).
// The ordering is *almost* case-insensitive lexicographic, with an exception.
// '\0' comes at the end of the alphabet instead of the beginning (thus options
// precede any other options which prefix them). Additionally, if two options
// are identical ignoring case, they are ordered according to case sensitive
// ordering if `FallbackCaseSensitive` is true.
ಪೂರ್ಣಾಙ್ಕ llvm::StrCmpOptionName(StringRef A, StringRef B,
                           ತಾರ್ಕಿಕ FallbackCaseSensitive) {
  size_t MinSize = std::min(A.size(), B.size());
  ಯದಿ (ಪೂರ್ಣಾಙ್ಕ Res = A.substr(0, MinSize).compare_insensitive(B.substr(0, MinSize)))
    ಪ್ರತ್ಯಾಗಮ Res;

  // If they are identical ignoring case, use case sensitive ordering.
  ಯದಿ (A.size() == B.size())
    ಪ್ರತ್ಯಾಗಮ FallbackCaseSensitive ? A.compare(B) : 0;

  ಪ್ರತ್ಯಾಗಮ (A.size() == MinSize) ? 1 /* A is a prefix of B. */
                               : -1 /* B is a prefix of A */;
}

// Comparison function for Option prefixes.
ಪೂರ್ಣಾಙ್ಕ llvm::StrCmpOptionPrefixes(ArrayRef<StringRef> APrefixes,
                               ArrayRef<StringRef> BPrefixes) {
  ಪ್ರತಿ (ಧ್ರುವ auto &[APre, BPre] : zip(APrefixes, BPrefixes)) {
    ಯದಿ (ಪೂರ್ಣಾಙ್ಕ Cmp = StrCmpOptionName(APre, BPre))
      ಪ್ರತ್ಯಾಗಮ Cmp;
  }
  // Both prefixes are identical.
  ಪ್ರತ್ಯಾಗಮ 0;
}
