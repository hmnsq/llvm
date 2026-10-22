//===- DynamicAPInt.cpp - DynamicAPInt Implementation -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ಅನ್ತರ್ಭಾವ "llvm/ADT/DynamicAPInt.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Hashing.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

hash_ಸಂಕೇತ llvm::hash_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ DynamicAPInt &X) {
  ಆದರೆ (X.isSmall())
    ಹಿಂತಿರುಗಿ llvm::hash_ಮೌಲ್ಯ(X.getSmall());
  ಹಿಂತಿರುಗಿ detail::hash_ಮೌಲ್ಯ(X.getLarge());
}

ಶೂನ್ಯ DynamicAPInt::static_assert_layout() {
  constexpr size_t Val_Large_ಸ್ಥಾನಾಂತರ =
      offsetof(DynamicAPInt, ValLarge.Val.ಬಿಟ್_ಅಗಲ_2);
  constexpr size_t Val_Small_ಸ್ಥಾನಾಂತರ = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t Val_Small_ಗಾತ್ರ = ಗಾತ್ರ(ValSmall);
  static_assert(Val_Large_ಸ್ಥಾನಾಂತರ >= Val_Small_ಸ್ಥಾನಾಂತರ + Val_Small_ಗಾತ್ರ);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (isSmall())
    ಹಿಂತಿರುಗಿ OS << ValSmall;
  ಹಿಂತಿರುಗಿ OS << ValLarge;
}

#ಆದರೆ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ವಿಧಾನ ಶೂನ್ಯ DynamicAPInt::dump() ಸ್ಥಿರಾಂಕ { print(dbgs()); }
#ಆದರೆ_ಅಂತ್ಯ
