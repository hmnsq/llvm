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
ཧྲིལ llvm::StrCmpOptionName(StringRef A, StringRef B,
                           bool FallbackCaseSensitive) {
  size_t MinSize = std::min(A.size(), B.size());
  གལ་སྲིད (ཧྲིལ Res = A.substr(0, MinSize).compare_insensitive(B.substr(0, MinSize)))
    ལོག Res;

  // If they are identical ignoring case, use case sensitive ordering.
  གལ་སྲིད (A.size() == B.size())
    ལོག FallbackCaseSensitive ? A.compare(B) : 0;

  ལོག (A.size() == MinSize) ? 1 /* A is a prefix of B. */
                               : -1 /* B is a prefix of A */;
}

// Comparison function for Option prefixes.
ཧྲིལ llvm::StrCmpOptionPrefixes(ArrayRef<StringRef> APrefixes,
                               ArrayRef<StringRef> BPrefixes) {
  དོན་ལུ (const auto &[APre, BPre] : zip(APrefixes, BPrefixes)) {
    གལ་སྲིད (ཧྲིལ Cmp = StrCmpOptionName(APre, BPre))
      ལོག Cmp;
  }
  // Both prefixes are identical.
  ལོག 0;
}
