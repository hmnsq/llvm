//===- OptionStrCmp.cpp - Option String Comparison --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/OptionStrCmp.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/STLExtras.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

// Comparison function for Option strings (option names & prefixes).
// The ordering is *almost* case-insensitive lexicographic, with an exception.
// '\0' comes at the end of the alphabet instead of the beginning (thus options
// precede any other options which prefix them). Additionally, if two options
// are identical ignoring case, they are ordered according to case sensitive
// ordering if `FallbackCaseSensitive` is true.
ਪੂਰਨ llvm::StrCmpOptionName(StringRef A, StringRef B,
                           ਤਾਰਕਿਕ FallbackCaseSensitive) {
  size_t Min_ਮਾਪ = std::min(A.ਮਾਪ_2(), B.ਮਾਪ_2());
  ਜੇ (ਪੂਰਨ Res = A.substr(0, Min_ਮਾਪ).compare_insensitive(B.substr(0, Min_ਮਾਪ)))
    ਵਾਪਸ Res;

  // If they are identical ignoring case, use case sensitive ordering.
  ਜੇ (A.ਮਾਪ_2() == B.ਮਾਪ_2())
    ਵਾਪਸ FallbackCaseSensitive ? A.compare(B) : 0;

  ਵਾਪਸ (A.ਮਾਪ_2() == Min_ਮਾਪ) ? 1 /* A is a prefix of B. */
                               : -1 /* B is a prefix of A */;
}

// Comparison function for Option prefixes.
ਪੂਰਨ llvm::StrCmpOptionPrefixes(ArrayRef<StringRef> APrefixes,
                               ArrayRef<StringRef> BPrefixes) {
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &[APre, BPre] : zip(APrefixes, BPrefixes)) {
    ਜੇ (ਪੂਰਨ Cmp = StrCmpOptionName(APre, BPre))
      ਵਾਪਸ Cmp;
  }
  // Both prefixes are identical.
  ਵਾਪਸ 0;
}
