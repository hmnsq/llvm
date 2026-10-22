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
पूर्णांक llvm::StrCmpOptionName(StringRef A, StringRef B,
                           तार्किक FallbackCaseSensitive) {
  माप_प्रकार MinSize = std::min(A.size(), B.size());
  जर (पूर्णांक Res = A.substr(0, MinSize).compare_insensitive(B.substr(0, MinSize)))
    परत Res;

  // If they are identical ignoring case, use case sensitive ordering.
  जर (A.size() == B.size())
    परत FallbackCaseSensitive ? A.compare(B) : 0;

  परत (A.size() == MinSize) ? 1 /* A is a prefix of B. */
                               : -1 /* B is a prefix of A */;
}

// Comparison function for Option prefixes.
पूर्णांक llvm::StrCmpOptionPrefixes(ArrayRef<StringRef> APrefixes,
                               ArrayRef<StringRef> BPrefixes) {
  साठी (स्थिरांक स्वतः &[APre, BPre] : zip(APrefixes, BPrefixes)) {
    जर (पूर्णांक Cmp = StrCmpOptionName(APre, BPre))
      परत Cmp;
  }
  // Both prefixes are identical.
  परत 0;
}
