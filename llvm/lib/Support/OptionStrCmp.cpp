//===- OptionStrCmp.cpp - Option String Comparison --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/OptionStrCmp.h"
#ضمن "llvm/ADT/STLExtras.h"

استعمال نطاق llvm;

// Comparison function for Option strings (option names & prefixes).
// The ordering is *almost* case-insensitive lexicographic, with an exception.
// '\0' comes at the end of the alphabet instead of the beginning (thus options
// precede any other options which prefix them). Additionally, if two options
// are identical ignoring case, they are ordered according to case sensitive
// ordering if `FallbackCaseSensitive` is true.
صحيح llvm::StrCmpOptionName(StringRef A, StringRef B,
                           منطقي FallbackCaseSensitive) {
  size_t Min_سائيز = std::min(A.سائيز_2(), B.سائيز_2());
  إذا (صحيح Res = A.substr(0, Min_سائيز).compare_insensitive(B.substr(0, Min_سائيز)))
    أرجع Res;

  // If they are identical ignoring case, use case sensitive ordering.
  إذا (A.سائيز_2() == B.سائيز_2())
    أرجع FallbackCaseSensitive ? A.compare(B) : 0;

  أرجع (A.سائيز_2() == Min_سائيز) ? 1 /* A is a prefix of B. */
                               : -1 /* B is a prefix of A */;
}

// Comparison function for Option prefixes.
صحيح llvm::StrCmpOptionPrefixes(ArrayRef<StringRef> APrefixes,
                               ArrayRef<StringRef> BPrefixes) {
  لكل (ثابت خودڪار &[APre, BPre] : zip(APrefixes, BPrefixes)) {
    إذا (صحيح Cmp = StrCmpOptionName(APre, BPre))
      أرجع Cmp;
  }
  // Both prefixes are identical.
  أرجع 0;
}
