//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/StringSaver.h"

#ضمن "llvm/ADT/SmallString.h"

ورتوں نطاق llvm;

StringRef StringSaver::save(StringRef S) {
  حرف *P = Alloc.Allocate<حرف>(S.ناپ_2() + 1);
  إذا (!S.خالی_تھاواں())
    حافظہ_نقل(P, S.انکڑے(), S.ناپ_2());
  P[S.ناپ_2()] = '\0';
  أرجع StringRef(P, S.ناپ_2());
}

StringRef StringSaver::save(ثابت Twine &S) {
  SmallString<128> Storage;
  أرجع save(S.منزل_String_Ref(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  خودکار R = Unique.داخل_کرو(S);
  إذا (R.دوجا_2)                 // cache miss, need to actually save the string
    *R.پہلا_2 = لڑیاں_2.save(S); // safe replacement with equal value
  أرجع *R.پہلا_2;
}

StringRef UniqueStringSaver::save(ثابت Twine &S) {
  SmallString<128> Storage;
  أرجع save(S.منزل_String_Ref(Storage));
}
