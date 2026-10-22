//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/StringSaver.h"

#ضمن "llvm/ADT/SmallString.h"

استعمال نطاق llvm;

StringRef StringSaver::save(StringRef S) {
  حرف *P = Alloc.Allocate<حرف>(S.سائيز_2() + 1);
  إذا (!S.خالي_جايون())
    يادگيري_نقل(P, S.معلومات(), S.سائيز_2());
  P[S.سائيز_2()] = '\0';
  أرجع StringRef(P, S.سائيز_2());
}

StringRef StringSaver::save(ثابت Twine &S) {
  SmallString<128> Storage;
  أرجع save(S.منزل_String_Ref(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  خودڪار R = Unique.وجهو(S);
  إذا (R.ٻيو_2)                 // cache miss, need to actually save the string
    *R.پهريون_2 = لڙيون_2.save(S); // safe replacement with equal value
  أرجع *R.پهريون_2;
}

StringRef UniqueStringSaver::save(ثابت Twine &S) {
  SmallString<128> Storage;
  أرجع save(S.منزل_String_Ref(Storage));
}
