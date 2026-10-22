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
पूर्णाङ्क llvm::StrCmpOptionName(StringRef A, StringRef B,
                           तार्किक FallbackCaseSensitive) {
  size_t MinSize = std::min(A.size(), B.size());
  यदि (पूर्णाङ्क Res = A.substr(0, MinSize).compare_insensitive(B.substr(0, MinSize)))
    प्रत्यागम Res;

  // If they are identical ignoring case, use case sensitive ordering.
  यदि (A.size() == B.size())
    प्रत्यागम FallbackCaseSensitive ? A.compare(B) : 0;

  प्रत्यागम (A.size() == MinSize) ? 1 /* A is a prefix of B. */
                               : -1 /* B is a prefix of A */;
}

// Comparison function for Option prefixes.
पूर्णाङ्क llvm::StrCmpOptionPrefixes(ArrayRef<StringRef> APrefixes,
                               ArrayRef<StringRef> BPrefixes) {
  प्रति (ध्रुव auto &[APre, BPre] : zip(APrefixes, BPrefixes)) {
    यदि (पूर्णाङ्क Cmp = StrCmpOptionName(APre, BPre))
      प्रत्यागम Cmp;
  }
  // Both prefixes are identical.
  प्रत्यागम 0;
}
