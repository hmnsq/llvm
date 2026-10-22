//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/StringSaver.h"

#ضمن "llvm/ADT/SmallString.h"

using نطاق llvm;

StringRef StringSaver::save(StringRef S) {
  حرف *P = Alloc.Allocate<حرف>(S.size() + 1);
  𐡄𐡍 (!S.empty())
    memcpy(P, S.data(), S.size());
  P[S.size()] = '\0';
  أرجع StringRef(P, S.size());
}

StringRef StringSaver::save(ثابت Twine &S) {
  SmallString<128> Storage;
  أرجع save(S.toStringRef(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  auto R = Unique.insert(S);
  𐡄𐡍 (R.second)                 // cache miss, need to actually save the string
    *R.first = Strings.save(S); // safe replacement with equal value
  أرجع *R.first;
}

StringRef UniqueStringSaver::save(ثابت Twine &S) {
  SmallString<128> Storage;
  أرجع save(S.toStringRef(Storage));
}
