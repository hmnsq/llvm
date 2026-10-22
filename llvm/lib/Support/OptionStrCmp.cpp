//===- OptionStrCmp.cpp - Option String Comparison --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/OptionStrCmp.h"
#અન્તર્ભાવ "llvm/ADT/STLExtras.h"

using નામાકાશ llvm;

// Comparison function for Option strings (option names & prefixes).
// The ordering is *almost* case-insensitive lexicographic, with an exception.
// '\0' comes at the end of the alphabet instead of the beginning (thus options
// precede any other options which prefix them). Additionally, if two options
// are identical ignoring case, they are ordered according to case sensitive
// ordering if `FallbackCaseSensitive` is true.
પૂર્ણાંક llvm::StrCmpOptionName(StringRef A, StringRef B,
                           તાર્કિક FallbackCaseSensitive) {
  size_t MinSize = std::min(A.size(), B.size());
  જો (પૂર્ણાંક Res = A.substr(0, MinSize).compare_insensitive(B.substr(0, MinSize)))
    પાછા Res;

  // If they are identical ignoring case, use case sensitive ordering.
  જો (A.size() == B.size())
    પાછા FallbackCaseSensitive ? A.compare(B) : 0;

  પાછા (A.size() == MinSize) ? 1 /* A is a prefix of B. */
                               : -1 /* B is a prefix of A */;
}

// Comparison function for Option prefixes.
પૂર્ણાંક llvm::StrCmpOptionPrefixes(ArrayRef<StringRef> APrefixes,
                               ArrayRef<StringRef> BPrefixes) {
  માટે (અચલ auto &[APre, BPre] : zip(APrefixes, BPrefixes)) {
    જો (પૂર્ણાંક Cmp = StrCmpOptionName(APre, BPre))
      પાછા Cmp;
  }
  // Both prefixes are identical.
  પાછા 0;
}
