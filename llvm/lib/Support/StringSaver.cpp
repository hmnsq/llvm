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
  सचे (!S.empty())
    memcpy(P, S.data(), S.size());
  P[S.size()] = '\0';
  पटिनिवत्त StringRef(P, S.size());
}

StringRef StringSaver::save(ध्रुव Twine &S) {
  SmallString<128> Storage;
  पटिनिवत्त save(S.toStringRef(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  auto R = Unique.निवेशय(S);
  सचे (R.second)                 // cache miss, need to actually save the string
    *R.first = Strings.save(S); // safe replacement with equal value
  पटिनिवत्त *R.first;
}

StringRef UniqueStringSaver::save(ध्रुव Twine &S) {
  SmallString<128> Storage;
  पटिनिवत्त save(S.toStringRef(Storage));
}
