//===- OptionStrCmp.cpp - Option String Comparison --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/OptionStrCmp.h"
#அந்தர்பாவ "llvm/ADT/STLExtras.h"

using நாமாகாஶ llvm;

// Comparison function for Option strings (option names & prefixes).
// The ordering is *almost* case-insensitive lexicographic, with an exception.
// '\0' comes at the end of the alphabet instead of the beginning (thus options
// precede any other options which prefix them). Additionally, if two options
// are identical ignoring case, they are ordered according to case sensitive
// ordering if `FallbackCaseSensitive` is true.
முழு llvm::StrCmpOptionName(StringRef A, StringRef B,
                           தருக்க FallbackCaseSensitive) {
  size_t MinSize = std::min(A.size(), B.size());
  எனில் (முழு Res = A.substr(0, MinSize).compare_insensitive(B.substr(0, MinSize)))
    திரும்பு Res;

  // If they are identical ignoring case, use case sensitive ordering.
  எனில் (A.size() == B.size())
    திரும்பு FallbackCaseSensitive ? A.compare(B) : 0;

  திரும்பு (A.size() == MinSize) ? 1 /* A is a prefix of B. */
                               : -1 /* B is a prefix of A */;
}

// Comparison function for Option prefixes.
முழு llvm::StrCmpOptionPrefixes(ArrayRef<StringRef> APrefixes,
                               ArrayRef<StringRef> BPrefixes) {
  க்கு (மாறிலி auto &[APre, BPre] : zip(APrefixes, BPrefixes)) {
    எனில் (முழு Cmp = StrCmpOptionName(APre, BPre))
      திரும்பு Cmp;
  }
  // Both prefixes are identical.
  திரும்பு 0;
}
