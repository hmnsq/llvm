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
  অক্ষর *P = Alloc.Allocate<অক্ষর>(S.আকার_3() + 1);
  যদি (!S.empty())
    memcpy(P, S.তথ্য(), S.আকার_3());
  P[S.আকার_3()] = '\0';
  ফেরত StringRef(P, S.আকার_3());
}

StringRef StringSaver::save(ধ্রুবক Twine &S) {
  SmallString<128> Storage;
  ফেরত save(S.toStringRef(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  auto R = Unique.ঢোকাও(S);
  যদি (R.second)                 // cache miss, need to actually save the string
    *R.first = Strings.save(S); // safe replacement with equal value
  ফেরত *R.first;
}

StringRef UniqueStringSaver::save(ধ্রুবক Twine &S) {
  SmallString<128> Storage;
  ফেরত save(S.toStringRef(Storage));
}
