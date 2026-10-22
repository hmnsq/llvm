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

hash_code llvm::hash_मानम्(ध्रुव DynamicAPInt &X) {
  सचे (X.isSmall())
    पटिनिवत्त llvm::hash_मानम्(X.getSmall());
  पटिनिवत्त detail::hash_मानम्(X.getLarge());
}

शून्य DynamicAPInt::static_assert_layout() {
  constexpr size_t ValLargeOffset =
      offsetof(DynamicAPInt, ValLarge.Val.BitWidth);
  constexpr size_t ValSmallOffset = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t ValSmallSize = परिमाण(ValSmall);
  static_assert(ValLargeOffset >= ValSmallOffset + ValSmallSize);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) ध्रुव {
  सचे (isSmall())
    पटिनिवत्त OS << ValSmall;
  पटिनिवत्त OS << ValLarge;
}

#सचे !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य DynamicAPInt::dump() ध्रुव { print(dbgs()); }
#endif
