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

استفاده نطاق llvm;

hash_کد llvm::hash_مقدار(ثابت DynamicAPInt &X) {
  إذا (X.isSmall())
    أرجع llvm::hash_مقدار(X.getSmall());
  أرجع detail::hash_مقدار(X.getLarge());
}

فراغ DynamicAPInt::static_assert_layout() {
  constexpr size_t Val_Large_جابه_جایی =
      offsetof(DynamicAPInt, ValLarge.Val.بیت_عرض_2);
  constexpr size_t Val_Small_جابه_جایی = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t Val_Small_اندازه = حجم(ValSmall);
  static_assert(Val_Large_جابه_جایی >= Val_Small_جابه_جایی + Val_Small_اندازه);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) ثابت {
  إذا (isSmall())
    أرجع OS << ValSmall;
  أرجع OS << ValLarge;
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_روش فراغ DynamicAPInt::dump() ثابت { print(dbgs()); }
#پایان_اگر
