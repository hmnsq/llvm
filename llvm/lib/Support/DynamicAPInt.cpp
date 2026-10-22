//===- DynamicAPInt.cpp - DynamicAPInt Implementation -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#અન્તર્ભાવ "llvm/ADT/DynamicAPInt.h"
#અન્તર્ભાવ "llvm/ADT/Hashing.h"
#અન્તર્ભાવ "llvm/Support/Debug.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"

using નામાકાશ llvm;

hash_code llvm::hash_કિંમત(અચલ DynamicAPInt &X) {
  જો (X.isSmall())
    પાછા llvm::hash_કિંમત(X.getSmall());
  પાછા detail::hash_કિંમત(X.getLarge());
}

શૂન્ય DynamicAPInt::static_assert_layout() {
  constexpr size_t ValLargeOffset =
      offsetof(DynamicAPInt, ValLarge.Val.BitWidth);
  constexpr size_t ValSmallOffset = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t ValSmallSize = કદ(ValSmall);
  static_assert(ValLargeOffset >= ValSmallOffset + ValSmallSize);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) અચલ {
  જો (isSmall())
    પાછા OS << ValSmall;
  પાછા OS << ValLarge;
}

#જો !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD શૂન્ય DynamicAPInt::dump() અચલ { print(dbgs()); }
#endif
