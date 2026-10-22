//===- DynamicAPInt.cpp - DynamicAPInt Implementation -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/DynamicAPInt.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/Hashing.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Debug.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"

using ନାମାକାଶ llvm;

hash_code llvm::hash_ମୂଲ୍ୟ(ଅଚଳ DynamicAPInt &X) {
  ଯଦି (X.isSmall())
    ଫେରନ୍ତୁ llvm::hash_ମୂଲ୍ୟ(X.getSmall());
  ଫେରନ୍ତୁ detail::hash_ମୂଲ୍ୟ(X.getLarge());
}

ଶୂନ୍ୟ DynamicAPInt::static_assert_layout() {
  constexpr size_t ValLargeOffset =
      offsetof(DynamicAPInt, ValLarge.Val.BitWidth);
  constexpr size_t ValSmallOffset = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t ValSmallSize = ଆକାର(ValSmall);
  static_assert(ValLargeOffset >= ValSmallOffset + ValSmallSize);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) ଅଚଳ {
  ଯଦି (isSmall())
    ଫେରନ୍ତୁ OS << ValSmall;
  ଫେରନ୍ତୁ OS << ValLarge;
}

#ଯଦି !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ଶୂନ୍ୟ DynamicAPInt::dump() ଅଚଳ { print(dbgs()); }
#endif
