//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/StringSaver.h"

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallString.h"

using ନାମାକାଶ llvm;

StringRef StringSaver::save(StringRef S) {
  ଅକ୍ଷର *P = Alloc.Allocate<ଅକ୍ଷର>(S.size() + 1);
  ଯଦି (!S.empty())
    memcpy(P, S.data(), S.size());
  P[S.size()] = '\0';
  ଫେରନ୍ତୁ StringRef(P, S.size());
}

StringRef StringSaver::save(ଅଚଳ Twine &S) {
  SmallString<128> Storage;
  ଫେରନ୍ତୁ save(S.toStringRef(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  auto R = Unique.ଭର୍ତ୍ତି(S);
  ଯଦି (R.second)                 // cache miss, need to actually save the string
    *R.first = Strings.save(S); // safe replacement with equal value
  ଫେରନ୍ତୁ *R.first;
}

StringRef UniqueStringSaver::save(ଅଚଳ Twine &S) {
  SmallString<128> Storage;
  ଫେରନ୍ତୁ save(S.toStringRef(Storage));
}
