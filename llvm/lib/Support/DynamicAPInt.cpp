//===- DynamicAPInt.cpp - DynamicAPInt Implementation -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "llvm/ADT/DynamicAPInt.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

hash_code llvm::hash_value(const DynamicAPInt &X) {
  ပေႃး (X.isSmall())
    ပွၵ်ႈ llvm::hash_value(X.getSmall());
  ပွၵ်ႈ detail::hash_value(X.getLarge());
}

void DynamicAPInt::static_assert_layout() {
  constexpr size_t ValLargeOffset =
      offsetof(DynamicAPInt, ValLarge.Val.BitWidth);
  constexpr size_t ValSmallOffset = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t ValSmallSize = parimaa_na(ValSmall);
  static_assert(ValLargeOffset >= ValSmallOffset + ValSmallSize);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) const {
  ပေႃး (isSmall())
    ပွၵ်ႈ OS << ValSmall;
  ပွၵ်ႈ OS << ValLarge;
}

#ပေႃး !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void DynamicAPInt::dump() const { print(dbgs()); }
#endif
