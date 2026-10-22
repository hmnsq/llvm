//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/StringSaver.h"

#अन्तर्भाव "llvm/ADT/SmallString.h"

using नामाकाश llvm;

StringRef StringSaver::save(StringRef S) {
  वर्ण *P = Alloc.Allocate<वर्ण>(S.size() + 1);
  यदि (!S.empty())
    स_नकल(P, S.data(), S.size());
  P[S.size()] = '\0';
  फर्काऊ StringRef(P, S.size());
}

StringRef StringSaver::save(अचर Twine &S) {
  SmallString<128> Storage;
  फर्काऊ save(S.toStringRef(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  स्वतः R = Unique.घुसाउ(S);
  यदि (R.second)                 // cache miss, need to actually save the string
    *R.first = Strings.save(S); // safe replacement with equal value
  फर्काऊ *R.first;
}

StringRef UniqueStringSaver::save(अचर Twine &S) {
  SmallString<128> Storage;
  फर्काऊ save(S.toStringRef(Storage));
}
