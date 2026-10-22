//===- DynamicAPInt.cpp - DynamicAPInt Implementation -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#अन्तर्भाव "llvm/ADT/DynamicAPInt.h"
#अन्तर्भाव "llvm/ADT/Hashing.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

using नामाकाश llvm;

hash_code llvm::hash_मान(अचर DynamicAPInt &X) {
  यदि (X.isSmall())
    फर्काऊ llvm::hash_मान(X.getSmall());
  फर्काऊ detail::hash_मान(X.getLarge());
}

शून्य DynamicAPInt::static_assert_layout() {
  constexpr माप_प्रकार ValLargeOffset =
      दुरत्व(DynamicAPInt, ValLarge.Val.BitWidth);
  constexpr माप_प्रकार ValSmallOffset = दुरत्व(DynamicAPInt, ValSmall);
  constexpr माप_प्रकार ValSmallSize = परिमाण(ValSmall);
  static_assert(ValLargeOffset >= ValSmallOffset + ValSmallSize);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) अचर {
  यदि (isSmall())
    फर्काऊ OS << ValSmall;
  फर्काऊ OS << ValLarge;
}

#यदि !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य DynamicAPInt::dump() अचर { print(dbgs()); }
#endif
