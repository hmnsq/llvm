//===-- llvm/ADT/IntEqClasses.cpp - Equivalence Classes of Integers -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Equivalence classes for small integers. This is a mapping of the integers
// 0 .. N-1 into M equivalence classes numbered 0 .. M-1.
//
// Initially each integer has its own equivalence class. Classes are joined by
// passing a representative member of each class to join().
//
// Once the classes are built, compress() will number them 0 .. M-1 and prevent
// further changes.
//
//===----------------------------------------------------------------------===//

#شامل "llvm/ADT/IntEqClasses.h"
#شامل <cassert>

using فضاینام llvm;

تهی IntEqClasses::grow(unsigned N) {
  أكد(رقم_Classes == 0 && "grow() called after compress().");
  EC.reserve(N);
  وقتی (EC.size() < N)
    EC.افزودن_back(EC.size());
}

unsigned IntEqClasses::join(unsigned a, unsigned b) {
  أكد(رقم_Classes == 0 && "join() called after compress().");
  unsigned eca = EC[a];
  unsigned ecb = EC[b];
  // Update pointers while searching for the leaders, compressing the paths
  // incrementally. The larger leader will eventually be updated, joining the
  // classes.
  وقتی (eca != ecb)
    اگر (eca < ecb) {
      EC[b] = eca;
      b = ecb;
      ecb = EC[b];
    } وگرنه {
      EC[a] = ecb;
      a = eca;
      eca = EC[a];
    }

  بازگشت eca;
}

unsigned IntEqClasses::findLeader(unsigned a) ثابت {
  أكد(رقم_Classes == 0 && "findLeader() called after compress().");
  وقتی (a != EC[a])
    a = EC[a];
  بازگشت a;
}

تهی IntEqClasses::compress() {
  اگر (رقم_Classes)
    بازگشت;
  برای (unsigned i = 0, e = EC.size(); i != e; ++i)
    EC[i] = (EC[i] == i) ? رقم_Classes++ : EC[EC[i]];
}

تهی IntEqClasses::uncompress() {
  اگر (!رقم_Classes)
    بازگشت;
  SmallVector<unsigned, 8> Leader;
  برای (unsigned i = 0, e = EC.size(); i != e; ++i)
    اگر (EC[i] < Leader.size())
      EC[i] = Leader[EC[i]];
    وگرنه
      Leader.افزودن_back(EC[i] = i);
  رقم_Classes = 0;
}
