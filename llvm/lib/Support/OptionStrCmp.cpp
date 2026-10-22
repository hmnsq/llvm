//===- OptionStrCmp.cpp - Option String Comparison --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/OptionStrCmp.h"
#ضمن "llvm/ADT/STLExtras.h"

using نطاق llvm;

// Comparison function for Option strings (option names & prefixes).
// The ordering is *almost* case-insensitive lexicographic, with an exception.
// '\0' comes at the end of the alphabet instead of the beginning (thus options
// precede any other options which prefix them). Additionally, if two options
// are identical ignoring case, they are ordered according to case sensitive
// ordering if `FallbackCaseSensitive` is true.
صحيح llvm::StrCmpOptionName(StringRef A, StringRef B,
                           منطقي FallbackCaseSensitive) {
  size_t MinSize = std::min(A.size(), B.size());
  𐡄𐡍 (صحيح Res = A.substr(0, MinSize).compare_insensitive(B.substr(0, MinSize)))
    أرجع Res;

  // If they are identical ignoring case, use case sensitive ordering.
  𐡄𐡍 (A.size() == B.size())
    أرجع FallbackCaseSensitive ? A.compare(B) : 0;

  أرجع (A.size() == MinSize) ? 1 /* A is a prefix of B. */
                               : -1 /* B is a prefix of A */;
}

// Comparison function for Option prefixes.
صحيح llvm::StrCmpOptionPrefixes(ArrayRef<StringRef> APrefixes,
                               ArrayRef<StringRef> BPrefixes) {
  لكل (ثابت auto &[APre, BPre] : zip(APrefixes, BPrefixes)) {
    𐡄𐡍 (صحيح Cmp = StrCmpOptionName(APre, BPre))
      أرجع Cmp;
  }
  // Both prefixes are identical.
  أرجع 0;
}
