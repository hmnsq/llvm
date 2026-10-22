//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/StringSaver.h"

#ضمن "llvm/ADT/SmallString.h"

استفاده نطاق llvm;

StringRef StringSaver::save(StringRef S) {
  حرف *P = Alloc.Allocate<حرف>(S.اندازه_2() + 1);
  إذا (!S.خالی())
    کپی_حافظه(P, S.داده(), S.اندازه_2());
  P[S.اندازه_2()] = '\0';
  أرجع StringRef(P, S.اندازه_2());
}

StringRef StringSaver::save(ثابت Twine &S) {
  SmallString<128> Storage;
  أرجع save(S.مقصد_String_Ref(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  خودکار R = Unique.درج(S);
  إذا (R.دومی_2)                 // cache miss, need to actually save the string
    *R.اولی_2 = رشتهها_2.save(S); // safe replacement with equal value
  أرجع *R.اولی_2;
}

StringRef UniqueStringSaver::save(ثابت Twine &S) {
  SmallString<128> Storage;
  أرجع save(S.مقصد_String_Ref(Storage));
}
