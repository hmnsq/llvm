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

hash_code llvm::hash_मूल्य(स्थिरांक DynamicAPInt &X) {
  जर (X.isSmall())
    परत llvm::hash_मूल्य(X.getSmall());
  परत detail::hash_मूल्य(X.getLarge());
}

शून्य DynamicAPInt::static_assert_layout() {
  constexpr माप_प्रकार ValLargeOffset =
      दुरत्व(DynamicAPInt, ValLarge.Val.BitWidth);
  constexpr माप_प्रकार ValSmallOffset = दुरत्व(DynamicAPInt, ValSmall);
  constexpr माप_प्रकार ValSmallSize = आकार(ValSmall);
  static_assert(ValLargeOffset >= ValSmallOffset + ValSmallSize);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) स्थिरांक {
  जर (isSmall())
    परत OS << ValSmall;
  परत OS << ValLarge;
}

#जर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य DynamicAPInt::dump() स्थिरांक { print(dbgs()); }
#endif
