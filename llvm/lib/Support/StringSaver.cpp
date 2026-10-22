//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/StringSaver.h"

#অন্তর্ভাব "llvm/ADT/SmallString.h"

using নামাকাশ llvm;

StringRef StringSaver::save(StringRef S) {
  বর্ণ *P = Alloc.Allocate<বর্ণ>(S.size() + 1);
  যদি (!S.empty())
    memcpy(P, S.data(), S.size());
  P[S.size()] = '\0';
  প্রত্যাগম StringRef(P, S.size());
}

StringRef StringSaver::save(ধ্রুব Twine &S) {
  SmallString<128> Storage;
  প্রত্যাগম save(S.toStringRef(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  auto R = Unique.ঢোকাও(S);
  যদি (R.second)                 // cache miss, need to actually save the string
    *R.first = Strings.save(S); // safe replacement with equal value
  প্রত্যাগম *R.first;
}

StringRef UniqueStringSaver::save(ধ্রুব Twine &S) {
  SmallString<128> Storage;
  প্রত্যাগম save(S.toStringRef(Storage));
}
