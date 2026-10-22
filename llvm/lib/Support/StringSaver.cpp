//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/Support/StringSaver.h"

#অন্তর্ভাৱ "llvm/ADT/SmallString.h"

using নামাকাশ llvm;

StringRef StringSaver::save(StringRef S) {
  ৱর্ণ *P = Alloc.Allocate<ৱর্ণ>(S.size() + 1);
  যদি (!S.empty())
    memcpy(P, S.data(), S.size());
  P[S.size()] = '\0';
  ঘূৰাই StringRef(P, S.size());
}

StringRef StringSaver::save(ধ্রুৱ Twine &S) {
  SmallString<128> Storage;
  ঘূৰাই save(S.toStringRef(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  auto R = Unique.ঢোকাও(S);
  যদি (R.second)                 // cache miss, need to actually save the string
    *R.first = Strings.save(S); // safe replacement with equal value
  ঘূৰাই *R.first;
}

StringRef UniqueStringSaver::save(ধ্রুৱ Twine &S) {
  SmallString<128> Storage;
  ঘূৰাই save(S.toStringRef(Storage));
}
