//===- DynamicAPInt.cpp - DynamicAPInt Implementation -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#شامل "llvm/ADT/DynamicAPInt.h"
#شامل "llvm/ADT/Hashing.h"
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/raw_ostream.h"

using فضاینام llvm;

hash_code llvm::hash_مقدار(ثابت DynamicAPInt &X) {
  اگر (X.isSmall())
    بازگشت llvm::hash_مقدار(X.getSmall());
  بازگشت detail::hash_مقدار(X.getLarge());
}

تهی DynamicAPInt::static_assert_layout() {
  constexpr size_t ValLargeOffset =
      offsetof(DynamicAPInt, ValLarge.Val.BitWidth);
  constexpr size_t ValSmallOffset = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t ValSmallSize = اندازه(ValSmall);
  static_assert(ValLargeOffset >= ValSmallOffset + ValSmallSize);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) ثابت {
  اگر (isSmall())
    بازگشت OS << ValSmall;
  بازگشت OS << ValLarge;
}

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD تهی DynamicAPInt::dump() ثابت { print(dbgs()); }
#endif
