//===-- StringSaver.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/StringSaver.h"

#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

StringRef StringSaver::save(StringRef S) {
  ಅಕ್ಷರ *P = Alloc.Allocate<ಅಕ್ಷರ>(S.ಗಾತ್ರ_3() + 1);
  ಆದರೆ (!S.ಖಾಲಿಗಳು())
    ಸ್ಮರಣೆ_ನಕಲು(P, S.ದತ್ತಾಂಶ(), S.ಗಾತ್ರ_3());
  P[S.ಗಾತ್ರ_3()] = '\0';
  ಹಿಂತಿರುಗಿ StringRef(P, S.ಗಾತ್ರ_3());
}

StringRef StringSaver::save(ಸ್ಥಿರಾಂಕ Twine &S) {
  SmallString<128> Storage;
  ಹಿಂತಿರುಗಿ save(S.ಗಮ್ಯ_String_Ref(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  ಸ್ವಯಂಚಾಲಿತ R = Unique.ಸೇರಿಸು(S);
  ಆದರೆ (R.ಎರಡನೇ_2)                 // cache miss, need to actually save the string
    *R.ಮೊದಲ_2 = ಅಕ್ಷರಸರಣಿಗಳು_2.save(S); // safe replacement with equal value
  ಹಿಂತಿರುಗಿ *R.ಮೊದಲ_2;
}

StringRef UniqueStringSaver::save(ಸ್ಥಿರಾಂಕ Twine &S) {
  SmallString<128> Storage;
  ಹಿಂತಿರುಗಿ save(S.ಗಮ್ಯ_String_Ref(Storage));
}
