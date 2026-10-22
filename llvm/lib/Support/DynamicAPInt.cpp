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

استعمال نطاق llvm;

hash_ڪوڊ llvm::hash_قدر(ثابت DynamicAPInt &X) {
  إذا (X.isSmall())
    أرجع llvm::hash_قدر(X.getSmall());
  أرجع detail::hash_قدر(X.getLarge());
}

فراغ DynamicAPInt::static_assert_layout() {
  constexpr size_t Val_Large_وٿي =
      offsetof(DynamicAPInt, ValLarge.Val.ٻٽو_انگ_ويڪر_2);
  constexpr size_t Val_Small_وٿي = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t Val_Small_سائيز = حجم(ValSmall);
  static_assert(Val_Large_وٿي >= Val_Small_وٿي + Val_Small_سائيز);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) ثابت {
  إذا (isSmall())
    أرجع OS << ValSmall;
  أرجع OS << ValLarge;
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طريقو فراغ DynamicAPInt::dump() ثابت { print(dbgs()); }
#جيڪڏهن_پورو
