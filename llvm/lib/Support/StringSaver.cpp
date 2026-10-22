//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/StringSaver.h"

#அந்தர்பாவ "llvm/ADT/SmallString.h"

using நாமாகாஶ llvm;

StringRef StringSaver::save(StringRef S) {
  எழுத்து *P = Alloc.Allocate<எழுத்து>(S.size() + 1);
  எனில் (!S.empty())
    memcpy(P, S.data(), S.size());
  P[S.size()] = '\0';
  திரும்பு StringRef(P, S.size());
}

StringRef StringSaver::save(மாறிலி Twine &S) {
  SmallString<128> Storage;
  திரும்பு save(S.toStringRef(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  auto R = Unique.செருகு(S);
  எனில் (R.second)                 // cache miss, need to actually save the string
    *R.first = Strings.save(S); // safe replacement with equal value
  திரும்பு *R.first;
}

StringRef UniqueStringSaver::save(மாறிலி Twine &S) {
  SmallString<128> Storage;
  திரும்பு save(S.toStringRef(Storage));
}
