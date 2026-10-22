//===- OptionStrCmp.cpp - Option String Comparison --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/OptionStrCmp.h"
#include "llvm/ADT/STLExtras.h"

using namespace llvm;

// Comparison function for Option strings (option names & prefixes).
// The ordering is *almost* case-insensitive lexicographic, with an exception.
// '\0' comes at the end of the alphabet instead of the beginning (thus options
// precede any other options which prefix them). Additionally, if two options
// are identical ignoring case, they are ordered according to case sensitive
// ordering if `FallbackCaseSensitive` is true.
정수 llvm::StrCmpOptionName(StringRef A, StringRef B,
                           bool FallbackCaseSensitive) {
  size_t MinSize = std::min(A.size(), B.size());
  만약 (정수 Res = A.substr(0, MinSize).compare_insensitive(B.substr(0, MinSize)))
    반환 Res;

  // If they are identical ignoring case, use case sensitive ordering.
  만약 (A.size() == B.size())
    반환 FallbackCaseSensitive ? A.compare(B) : 0;

  반환 (A.size() == MinSize) ? 1 /* A is a prefix of B. */
                               : -1 /* B is a prefix of A */;
}

// Comparison function for Option prefixes.
정수 llvm::StrCmpOptionPrefixes(ArrayRef<StringRef> APrefixes,
                               ArrayRef<StringRef> BPrefixes) {
  위해 (const auto &[APre, BPre] : zip(APrefixes, BPrefixes)) {
    만약 (정수 Cmp = StrCmpOptionName(APre, BPre))
      반환 Cmp;
  }
  // Both prefixes are identical.
  반환 0;
}
