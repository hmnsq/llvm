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

उपयुज्य नामाकाश llvm;

hash_संहिता llvm::hash_मानम्(ध्रुव DynamicAPInt &X) {
  यदि (X.isSmall())
    प्रत्यागम llvm::hash_मानम्(X.getSmall());
  प्रत्यागम detail::hash_मानम्(X.getLarge());
}

शून्य DynamicAPInt::static_assert_layout() {
  constexpr size_t Val_Large_विस्थापनम् =
      offsetof(DynamicAPInt, ValLarge.Val.Bit_विस्तारः);
  constexpr size_t Val_Small_विस्थापनम् = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t Val_Small_आकार = परिमाण(ValSmall);
  static_assert(Val_Large_विस्थापनम् >= Val_Small_विस्थापनम् + Val_Small_आकार);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) ध्रुव {
  यदि (isSmall())
    प्रत्यागम OS << ValSmall;
  प्रत्यागम OS << ValLarge;
}

#यदि !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_विधानम् शून्य DynamicAPInt::dump() ध्रुव { print(dbgs()); }
#यदि_अन्तः
