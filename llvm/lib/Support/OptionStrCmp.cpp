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
ຈຳນວນເຕັມ llvm::StrCmpOptionName(StringRef A, StringRef B,
                           bool FallbackCaseSensitive) {
  size_t MinSize = std::min(A.size(), B.size());
  ຖ້າ (ຈຳນວນເຕັມ Res = A.substr(0, MinSize).compare_insensitive(B.substr(0, MinSize)))
    ສົ່ງຄືນ Res;

  // If they are identical ignoring case, use case sensitive ordering.
  ຖ້າ (A.size() == B.size())
    ສົ່ງຄືນ FallbackCaseSensitive ? A.compare(B) : 0;

  ສົ່ງຄືນ (A.size() == MinSize) ? 1 /* A is a prefix of B. */
                               : -1 /* B is a prefix of A */;
}

// Comparison function for Option prefixes.
ຈຳນວນເຕັມ llvm::StrCmpOptionPrefixes(ArrayRef<StringRef> APrefixes,
                               ArrayRef<StringRef> BPrefixes) {
  ສຳລັບ (const auto &[APre, BPre] : zip(APrefixes, BPrefixes)) {
    ຖ້າ (ຈຳນວນເຕັມ Cmp = StrCmpOptionName(APre, BPre))
      ສົ່ງຄືນ Cmp;
  }
  // Both prefixes are identical.
  ສົ່ງຄືນ 0;
}
