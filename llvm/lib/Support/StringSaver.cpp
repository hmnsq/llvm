//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/StringSaver.h"

#include "llvm/ADT/SmallString.h"

using namespace llvm;

StringRef StringSaver::save(StringRef S) {
  var_na *P = Alloc.Allocate<var_na>(S.size() + 1);
  གལ་ཏེ (!S.empty())
    memcpy(P, S.data(), S.size());
  P[S.size()] = '\0';
  ཕྱིར StringRef(P, S.size());
}

StringRef StringSaver::save(const Twine &S) {
  SmallString<128> Storage;
  ཕྱིར save(S.toStringRef(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  auto R = Unique.insert(S);
  གལ་ཏེ (R.second)                 // cache miss, need to actually save the string
    *R.first = Strings.save(S); // safe replacement with equal value
  ཕྱིར *R.first;
}

StringRef UniqueStringSaver::save(const Twine &S) {
  SmallString<128> Storage;
  ཕྱིར save(S.toStringRef(Storage));
}
