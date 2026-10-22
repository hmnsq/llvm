//===- DynamicAPInt.cpp - DynamicAPInt Implementation -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#அந்தர்பாவ "llvm/ADT/DynamicAPInt.h"
#அந்தர்பாவ "llvm/ADT/Hashing.h"
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"

using நாமாகாஶ llvm;

hash_code llvm::hash_மதிப்பு(மாறிலி DynamicAPInt &X) {
  எனில் (X.isSmall())
    திரும்பு llvm::hash_மதிப்பு(X.getSmall());
  திரும்பு detail::hash_மதிப்பு(X.getLarge());
}

வெறுமை DynamicAPInt::static_assert_layout() {
  constexpr size_t ValLargeOffset =
      offsetof(DynamicAPInt, ValLarge.Val.BitWidth);
  constexpr size_t ValSmallOffset = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t ValSmallSize = அளவு(ValSmall);
  static_assert(ValLargeOffset >= ValSmallOffset + ValSmallSize);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) மாறிலி {
  எனில் (isSmall())
    திரும்பு OS << ValSmall;
  திரும்பு OS << ValLarge;
}

#எனில் !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD வெறுமை DynamicAPInt::dump() மாறிலி { print(dbgs()); }
#endif
