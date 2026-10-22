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

ورتوں نطاق llvm;

hash_کوڈ llvm::hash_قدر(ثابت DynamicAPInt &X) {
  إذا (X.isSmall())
    أرجع llvm::hash_قدر(X.getSmall());
  أرجع detail::hash_قدر(X.getLarge());
}

فراغ DynamicAPInt::static_assert_layout() {
  constexpr size_t Val_Large_وتھ =
      offsetof(DynamicAPInt, ValLarge.Val.ثنائی_ہندسہ_چوڑائی_2);
  constexpr size_t Val_Small_وتھ = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t Val_Small_ناپ = حجم(ValSmall);
  static_assert(Val_Large_وتھ >= Val_Small_وتھ + Val_Small_ناپ);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) ثابت {
  إذا (isSmall())
    أرجع OS << ValSmall;
  أرجع OS << ValLarge;
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طریقہ فراغ DynamicAPInt::dump() ثابت { print(dbgs()); }
#جے_انت
