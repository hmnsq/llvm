//===- DynamicAPInt.cpp - DynamicAPInt Implementation -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#समावेश "llvm/ADT/DynamicAPInt.h"
#समावेश "llvm/ADT/Hashing.h"
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/raw_ostream.h"

using namespace llvm;

hash_code llvm::hash_मूल्य(स्थिर DynamicAPInt &X) {
  अगर (X.isSmall())
    वापस llvm::hash_मूल्य(X.getSmall());
  वापस detail::hash_मूल्य(X.getLarge());
}

व्योम DynamicAPInt::static_assert_layout() {
  constexpr माप_प्रकार ValLargeOffset =
      दुरत्व(DynamicAPInt, ValLarge.Val.BitWidth);
  constexpr माप_प्रकार ValSmallOffset = दुरत्व(DynamicAPInt, ValSmall);
  constexpr माप_प्रकार Val_Small_आकार = माप(ValSmall);
  static_assert(ValLargeOffset >= ValSmallOffset + Val_Small_आकार);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) स्थिर {
  अगर (isSmall())
    वापस OS << ValSmall;
  वापस OS << ValLarge;
}

#अगर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD व्योम DynamicAPInt::dump() स्थिर { print(dbgs()); }
#पूर्ण_अगर
