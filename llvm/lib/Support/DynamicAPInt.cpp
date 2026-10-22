//===- DynamicAPInt.cpp - DynamicAPInt Implementation -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#অন্তর্ভাব "llvm/ADT/DynamicAPInt.h"
#অন্তর্ভাব "llvm/ADT/Hashing.h"
#অন্তর্ভাব "llvm/Support/Debug.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"

using নামাকাশ llvm;

hash_code llvm::hash_মান(ধ্রুব DynamicAPInt &X) {
  যদি (X.isSmall())
    প্রত্যাগম llvm::hash_মান(X.getSmall());
  প্রত্যাগম detail::hash_মান(X.getLarge());
}

শূন্য DynamicAPInt::static_assert_layout() {
  constexpr size_t ValLargeOffset =
      offsetof(DynamicAPInt, ValLarge.Val.BitWidth);
  constexpr size_t ValSmallOffset = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t ValSmallSize = পরিমাণ(ValSmall);
  static_assert(ValLargeOffset >= ValSmallOffset + ValSmallSize);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) ধ্রুব {
  যদি (isSmall())
    প্রত্যাগম OS << ValSmall;
  প্রত্যাগম OS << ValLarge;
}

#যদি !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD শূন্য DynamicAPInt::dump() ধ্রুব { print(dbgs()); }
#endif
