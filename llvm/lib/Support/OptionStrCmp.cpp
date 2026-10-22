//===- OptionStrCmp.cpp - Option String Comparison --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/OptionStrCmp.h"
#അന്തര്ഭാവ "llvm/ADT/STLExtras.h"

using നാമാകാശ llvm;

// Comparison function for Option strings (option names & prefixes).
// The ordering is *almost* case-insensitive lexicographic, with an exception.
// '\0' comes at the end of the alphabet instead of the beginning (thus options
// precede any other options which prefix them). Additionally, if two options
// are identical ignoring case, they are ordered according to case sensitive
// ordering if `FallbackCaseSensitive` is true.
പൂർണ്ണസംഖ്യ llvm::StrCmpOptionName(StringRef A, StringRef B,
                           തർക്കം FallbackCaseSensitive) {
  size_t MinSize = std::min(A.size(), B.size());
  എങ്കിൽ (പൂർണ്ണസംഖ്യ Res = A.substr(0, MinSize).compare_insensitive(B.substr(0, MinSize)))
    തിരികെ Res;

  // If they are identical ignoring case, use case sensitive ordering.
  എങ്കിൽ (A.size() == B.size())
    തിരികെ FallbackCaseSensitive ? A.compare(B) : 0;

  തിരികെ (A.size() == MinSize) ? 1 /* A is a prefix of B. */
                               : -1 /* B is a prefix of A */;
}

// Comparison function for Option prefixes.
പൂർണ്ണസംഖ്യ llvm::StrCmpOptionPrefixes(ArrayRef<StringRef> APrefixes,
                               ArrayRef<StringRef> BPrefixes) {
  വേണ്ടി (സ്ഥിരം auto &[APre, BPre] : zip(APrefixes, BPrefixes)) {
    എങ്കിൽ (പൂർണ്ണസംഖ്യ Cmp = StrCmpOptionName(APre, BPre))
      തിരികെ Cmp;
  }
  // Both prefixes are identical.
  തിരികെ 0;
}
