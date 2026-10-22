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
  अक्षर *P = Alloc.Allocate<अक्षर>(S.size() + 1);
  जर (!S.empty())
    स_नकल(P, S.data(), S.size());
  P[S.size()] = '\0';
  परत StringRef(P, S.size());
}

StringRef StringSaver::save(स्थिरांक Twine &S) {
  SmallString<128> Storage;
  परत save(S.toStringRef(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  स्वतः R = Unique.घाला(S);
  जर (R.second)                 // cache miss, need to actually save the string
    *R.first = Strings.save(S); // safe replacement with equal value
  परत *R.first;
}

StringRef UniqueStringSaver::save(स्थिरांक Twine &S) {
  SmallString<128> Storage;
  परत save(S.toStringRef(Storage));
}
