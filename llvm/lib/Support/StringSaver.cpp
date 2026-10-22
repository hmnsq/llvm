//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/StringSaver.h"

#समावेश "llvm/ADT/SmallString.h"

using namespace llvm;

StringRef StringSaver::save(StringRef S) {
  अक्षर *P = Alloc.Allocate<अक्षर>(S.आकार_2() + 1);
  अगर (!S.empty())
    स_नकल(P, S.तथ्य(), S.आकार_2());
  P[S.आकार_2()] = '\0';
  वापस StringRef(P, S.आकार_2());
}

StringRef StringSaver::save(स्थिर Twine &S) {
  SmallString<128> Storage;
  वापस save(S.toStringRef(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  स्वतः R = Unique.जोड़ो(S);
  अगर (R.second)                 // cache miss, need to actually save the string
    *R.first = Strings.save(S); // safe replacement with equal value
  वापस *R.first;
}

StringRef UniqueStringSaver::save(स्थिर Twine &S) {
  SmallString<128> Storage;
  वापस save(S.toStringRef(Storage));
}
