//===- DynamicAPInt.cpp - DynamicAPInt Implementation -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#അന്തര്ഭാവ "llvm/ADT/DynamicAPInt.h"
#അന്തര്ഭാവ "llvm/ADT/Hashing.h"
#അന്തര്ഭാവ "llvm/Support/Debug.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"

using നാമാകാശ llvm;

hash_code llvm::hash_മൂല്യം(സ്ഥിരം DynamicAPInt &X) {
  എങ്കിൽ (X.isSmall())
    തിരികെ llvm::hash_മൂല്യം(X.getSmall());
  തിരികെ detail::hash_മൂല്യം(X.getLarge());
}

ശൂന്യം DynamicAPInt::static_assert_layout() {
  constexpr size_t ValLargeOffset =
      offsetof(DynamicAPInt, ValLarge.Val.BitWidth);
  constexpr size_t ValSmallOffset = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t ValSmallSize = വലുപ്പം(ValSmall);
  static_assert(ValLargeOffset >= ValSmallOffset + ValSmallSize);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) സ്ഥിരം {
  എങ്കിൽ (isSmall())
    തിരികെ OS << ValSmall;
  തിരികെ OS << ValLarge;
}

#എങ്കിൽ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ശൂന്യം DynamicAPInt::dump() സ്ഥിരം { print(dbgs()); }
#endif
