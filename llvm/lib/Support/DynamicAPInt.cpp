//===- DynamicAPInt.cpp - DynamicAPInt Implementation -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#అన్తర్భావ "llvm/ADT/DynamicAPInt.h"
#అన్తర్భావ "llvm/ADT/Hashing.h"
#అన్తర్భావ "llvm/Support/Debug.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"

ఉపయోగించి నామాకాశ llvm;

hash_సంకేతం llvm::hash_విలువ(స్థిరరాశి DynamicAPInt &X) {
  ఒకవేళ (X.isSmall())
    తిరిగి llvm::hash_విలువ(X.getSmall());
  తిరిగి detail::hash_విలువ(X.getLarge());
}

శూన్య DynamicAPInt::static_assert_layout() {
  constexpr size_t Val_Large_స్థానభ్రంశం =
      offsetof(DynamicAPInt, ValLarge.Val.బిట్టు_వెడల్పు_2);
  constexpr size_t Val_Small_స్థానభ్రంశం = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t Val_Small_కొలత = పరిమాణం(ValSmall);
  static_assert(Val_Large_స్థానభ్రంశం >= Val_Small_స్థానభ్రంశం + Val_Small_కొలత);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) స్థిరరాశి {
  ఒకవేళ (isSmall())
    తిరిగి OS << ValSmall;
  తిరిగి OS << ValLarge;
}

#ఒకవేళ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_పద్ధతి శూన్య DynamicAPInt::dump() స్థిరరాశి { print(dbgs()); }
#అయితే_ముగింపు
