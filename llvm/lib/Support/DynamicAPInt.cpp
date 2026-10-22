//===- DynamicAPInt.cpp - DynamicAPInt Implementation -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/DynamicAPInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Hashing.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

hash_ਕੋਡ llvm::hash_ਮੁੱਲ(ਅਚਲ DynamicAPInt &X) {
  ਜੇ (X.isSmall())
    ਵਾਪਸ llvm::hash_ਮੁੱਲ(X.getSmall());
  ਵਾਪਸ detail::hash_ਮੁੱਲ(X.getLarge());
}

ਖਾਲੀ DynamicAPInt::static_assert_layout() {
  constexpr size_t Val_Large_ਵਿੱਥ =
      offsetof(DynamicAPInt, ValLarge.Val.ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  constexpr size_t Val_Small_ਵਿੱਥ = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t Val_Small_ਮਾਪ = ਆਕਾਰ(ValSmall);
  static_assert(Val_Large_ਵਿੱਥ >= Val_Small_ਵਿੱਥ + Val_Small_ਮਾਪ);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) ਅਚਲ {
  ਜੇ (isSmall())
    ਵਾਪਸ OS << ValSmall;
  ਵਾਪਸ OS << ValLarge;
}

#ਜੇ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ਵਿਧੀ ਖਾਲੀ DynamicAPInt::dump() ਅਚਲ { print(dbgs()); }
#ਜੇ_ਅੰਤ
