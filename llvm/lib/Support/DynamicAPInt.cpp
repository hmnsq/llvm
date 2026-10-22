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

استعمال نامخلا llvm;

hash_رمز llvm::hash_قدر(ثابت DynamicAPInt &X) {
  اگر (X.isSmall())
    واپس llvm::hash_قدر(X.getSmall());
  واپس detail::hash_قدر(X.getLarge());
}

خالی DynamicAPInt::static_assert_layout() {
  constexpr size_t Val_Large_انحراف =
      offsetof(DynamicAPInt, ValLarge.Val.ثنائی_ہندسہ_چوڑائی_2);
  constexpr size_t Val_Small_انحراف = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t Val_Small_سائز = حجم(ValSmall);
  static_assert(Val_Large_انحراف >= Val_Small_انحراف + Val_Small_سائز);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) ثابت {
  اگر (isSmall())
    واپس OS << ValSmall;
  واپس OS << ValLarge;
}

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طریقہ خالی DynamicAPInt::dump() ثابت { print(dbgs()); }
#اگر_ختم
