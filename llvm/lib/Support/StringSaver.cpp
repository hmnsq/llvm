//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/StringSaver.h"

#شامل "llvm/ADT/SmallString.h"

استعمال نامخلا llvm;

StringRef StringSaver::save(StringRef S) {
  حرف *P = Alloc.Allocate<حرف>(S.سائز_2() + 1);
  اگر (!S.خالی_خانے())
    حافظہ_نقل(P, S.ڈیٹا(), S.سائز_2());
  P[S.سائز_2()] = '\0';
  واپس StringRef(P, S.سائز_2());
}

StringRef StringSaver::save(ثابت Twine &S) {
  SmallString<128> Storage;
  واپس save(S.مقصد_String_Ref(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  خودکار R = Unique.داخل_کرو(S);
  اگر (R.دوسرا_2)                 // cache miss, need to actually save the string
    *R.پہلا_2 = لڑیاں_2.save(S); // safe replacement with equal value
  واپس *R.پہلا_2;
}

StringRef UniqueStringSaver::save(ثابت Twine &S) {
  SmallString<128> Storage;
  واپس save(S.مقصد_String_Ref(Storage));
}
