//===- DynamicAPInt.cpp - DynamicAPInt Implementation -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ضمن "llvm/ADT/DynamicAPInt.h"
#ضمن "llvm/ADT/Hashing.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/raw_ostream.h"

using نطاق llvm;

hash_code llvm::hash_قيمة(ثابت DynamicAPInt &X) {
  إذا (X.isSmall())
    أرجع llvm::hash_قيمة(X.getSmall());
  أرجع detail::hash_قيمة(X.getLarge());
}

فراغ DynamicAPInt::static_assert_layout() {
  constexpr size_t ValLargeOffset =
      offsetof(DynamicAPInt, ValLarge.Val.BitWidth);
  constexpr size_t ValSmallOffset = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t ValSmallSize = حجم(ValSmall);
  static_assert(ValLargeOffset >= ValSmallOffset + ValSmallSize);
}

raw_ostream &DynamicAPInt::اطبع(raw_ostream &OS) ثابت {
  إذا (isSmall())
    أرجع OS << ValSmall;
  أرجع OS << ValLarge;
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD فراغ DynamicAPInt::dump() ثابت { اطبع(dbgs()); }
#endif
