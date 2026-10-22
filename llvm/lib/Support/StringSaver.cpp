//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/StringSaver.h"

#شامل "llvm/ADT/SmallString.h"

using فضاینام llvm;

StringRef StringSaver::save(StringRef S) {
  نویسه *P = Alloc.Allocate<نویسه>(S.size() + 1);
  اگر (!S.empty())
    memcpy(P, S.data(), S.size());
  P[S.size()] = '\0';
  بازگشت StringRef(P, S.size());
}

StringRef StringSaver::save(ثابت Twine &S) {
  SmallString<128> Storage;
  بازگشت save(S.toStringRef(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  auto R = Unique.درج(S);
  اگر (R.second)                 // cache miss, need to actually save the string
    *R.first = Strings.save(S); // safe replacement with equal value
  بازگشت *R.first;
}

StringRef UniqueStringSaver::save(ثابت Twine &S) {
  SmallString<128> Storage;
  بازگشت save(S.toStringRef(Storage));
}
