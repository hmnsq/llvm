//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/StringSaver.h"

#अन्तर्भाव "llvm/ADT/SmallString.h"

उपयुज्य नामाकाश llvm;

StringRef StringSaver::save(StringRef S) {
  वर्ण *P = Alloc.Allocate<वर्ण>(S.आकार_2() + 1);
  यदि (!S.रिक्तानि())
    स्मृतिं_प्रतिलिख(P, S.दत्त(), S.आकार_2());
  P[S.आकार_2()] = '\0';
  प्रत्यागम StringRef(P, S.आकार_2());
}

StringRef StringSaver::save(ध्रुव Twine &S) {
  SmallString<128> Storage;
  प्रत्यागम save(S.गन्तव्यम्_String_Ref(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  स्वचालित R = Unique.निवेशय(S);
  यदि (R.second)                 // cache miss, need to actually save the string
    *R.first = वाक्यानि_2.save(S); // safe replacement with equal value
  प्रत्यागम *R.first;
}

StringRef UniqueStringSaver::save(ध्रुव Twine &S) {
  SmallString<128> Storage;
  प्रत्यागम save(S.गन्तव्यम्_String_Ref(Storage));
}
