//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/StringSaver.h"

#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"

using ನಾಮಾಕಾಶ llvm;

StringRef StringSaver::save(StringRef S) {
  ವರ್ಣ *P = Alloc.Allocate<ವರ್ಣ>(S.size() + 1);
  ಯದಿ (!S.empty())
    memcpy(P, S.data(), S.size());
  P[S.size()] = '\0';
  ಪ್ರತ್ಯಾಗಮ StringRef(P, S.size());
}

StringRef StringSaver::save(ಧ್ರುವ Twine &S) {
  SmallString<128> Storage;
  ಪ್ರತ್ಯಾಗಮ save(S.toStringRef(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  auto R = Unique.ಸೇರಿಸು(S);
  ಯದಿ (R.second)                 // cache miss, need to actually save the string
    *R.first = Strings.save(S); // safe replacement with equal value
  ಪ್ರತ್ಯಾಗಮ *R.first;
}

StringRef UniqueStringSaver::save(ಧ್ರುವ Twine &S) {
  SmallString<128> Storage;
  ಪ್ರತ್ಯಾಗಮ save(S.toStringRef(Storage));
}
