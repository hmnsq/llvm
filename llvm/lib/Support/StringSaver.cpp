//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/StringSaver.h"

#അന്തര്ഭാവ "llvm/ADT/SmallString.h"

using നാമാകാശ llvm;

StringRef StringSaver::save(StringRef S) {
  അക്ഷരം *P = Alloc.Allocate<അക്ഷരം>(S.size() + 1);
  എങ്കിൽ (!S.empty())
    memcpy(P, S.data(), S.size());
  P[S.size()] = '\0';
  തിരികെ StringRef(P, S.size());
}

StringRef StringSaver::save(സ്ഥിരം Twine &S) {
  SmallString<128> Storage;
  തിരികെ save(S.toStringRef(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  auto R = Unique.തിരുകുക(S);
  എങ്കിൽ (R.second)                 // cache miss, need to actually save the string
    *R.first = Strings.save(S); // safe replacement with equal value
  തിരികെ *R.first;
}

StringRef UniqueStringSaver::save(സ്ഥിരം Twine &S) {
  SmallString<128> Storage;
  തിരികെ save(S.toStringRef(Storage));
}
