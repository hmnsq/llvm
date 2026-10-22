//===- OptionStrCmp.cpp - Option String Comparison --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/OptionStrCmp.h"
#समावेश "llvm/ADT/STLExtras.h"

using namespace llvm;

// Comparison function for Option strings (option names & prefixes).
// The ordering is *almost* case-insensitive lexicographic, with an exception.
// '\0' comes at the end of the alphabet instead of the beginning (thus options
// precede any other options which prefix them). Additionally, if two options
// are identical ignoring case, they are ordered according to case sensitive
// ordering if `FallbackCaseSensitive` is true.
पूर्णांक llvm::StrCmpOptionName(StringRef A, StringRef B,
                           bool FallbackCaseSensitive) {
  माप_प्रकार Min_आकार = std::min(A.आकार_2(), B.आकार_2());
  अगर (पूर्णांक Res = A.substr(0, Min_आकार).compare_insensitive(B.substr(0, Min_आकार)))
    वापस Res;

  // If they are identical ignoring case, use case sensitive ordering.
  अगर (A.आकार_2() == B.आकार_2())
    वापस FallbackCaseSensitive ? A.compare(B) : 0;

  वापस (A.आकार_2() == Min_आकार) ? 1 /* A is a prefix of B. */
                               : -1 /* B is a prefix of A */;
}

// Comparison function for Option prefixes.
पूर्णांक llvm::StrCmpOptionPrefixes(ArrayRef<StringRef> APrefixes,
                               ArrayRef<StringRef> BPrefixes) {
  क्रम (स्थिर स्वतः &[APre, BPre] : zip(APrefixes, BPrefixes)) {
    अगर (पूर्णांक Cmp = StrCmpOptionName(APre, BPre))
      वापस Cmp;
  }
  // Both prefixes are identical.
  वापस 0;
}
