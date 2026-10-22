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

using ನಾಮಾಕಾಶ llvm;

hash_code llvm::hash_ಮೌಲ್ಯ(ಧ್ರುವ DynamicAPInt &X) {
  ಯದಿ (X.isSmall())
    ಪ್ರತ್ಯಾಗಮ llvm::hash_ಮೌಲ್ಯ(X.getSmall());
  ಪ್ರತ್ಯಾಗಮ detail::hash_ಮೌಲ್ಯ(X.getLarge());
}

ಶೂನ್ಯ DynamicAPInt::static_assert_layout() {
  constexpr size_t ValLargeOffset =
      offsetof(DynamicAPInt, ValLarge.Val.BitWidth);
  constexpr size_t ValSmallOffset = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t ValSmallSize = ಪರಿಮಾಣ(ValSmall);
  static_assert(ValLargeOffset >= ValSmallOffset + ValSmallSize);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) ಧ್ರುವ {
  ಯದಿ (isSmall())
    ಪ್ರತ್ಯಾಗಮ OS << ValSmall;
  ಪ್ರತ್ಯಾಗಮ OS << ValLarge;
}

#ಯದಿ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ಶೂನ್ಯ DynamicAPInt::dump() ಧ್ರುವ { print(dbgs()); }
#endif
