//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/StringSaver.h"

#અન્તર્ભાવ "llvm/ADT/SmallString.h"

using નામાકાશ llvm;

StringRef StringSaver::save(StringRef S) {
  અક્ષર *P = Alloc.Allocate<અક્ષર>(S.size() + 1);
  જો (!S.empty())
    memcpy(P, S.data(), S.size());
  P[S.size()] = '\0';
  પાછા StringRef(P, S.size());
}

StringRef StringSaver::save(અચલ Twine &S) {
  SmallString<128> Storage;
  પાછા save(S.toStringRef(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  auto R = Unique.ઉમેરો(S);
  જો (R.second)                 // cache miss, need to actually save the string
    *R.first = Strings.save(S); // safe replacement with equal value
  પાછા *R.first;
}

StringRef UniqueStringSaver::save(અચલ Twine &S) {
  SmallString<128> Storage;
  પાછા save(S.toStringRef(Storage));
}
