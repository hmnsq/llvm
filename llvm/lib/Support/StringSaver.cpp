//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/StringSaver.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallString.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

StringRef StringSaver::save(StringRef S) {
  ਅੱਖਰ *P = Alloc.Allocate<ਅੱਖਰ>(S.ਮਾਪ_2() + 1);
  ਜੇ (!S.ਖਾਲੀ_ਥਾਵਾਂ())
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(P, S.ਅੰਕੜੇ(), S.ਮਾਪ_2());
  P[S.ਮਾਪ_2()] = '\0';
  ਵਾਪਸ StringRef(P, S.ਮਾਪ_2());
}

StringRef StringSaver::save(ਅਚਲ Twine &S) {
  SmallString<128> Storage;
  ਵਾਪਸ save(S.ਮੰਜ਼ਿਲ_String_Ref(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  ਸਵੈਚਾਲਿਤ R = Unique.ਪਾਓ(S);
  ਜੇ (R.ਦੂਜਾ_2)                 // cache miss, need to actually save the string
    *R.ਪਹਿਲਾ_2 = ਲੜੀਆਂ_2.save(S); // safe replacement with equal value
  ਵਾਪਸ *R.ਪਹਿਲਾ_2;
}

StringRef UniqueStringSaver::save(ਅਚਲ Twine &S) {
  SmallString<128> Storage;
  ਵਾਪਸ save(S.ਮੰਜ਼ਿਲ_String_Ref(Storage));
}
