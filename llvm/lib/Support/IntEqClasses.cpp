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

#অন্তর্ভাব "llvm/ADT/IntEqClasses.h"
#অন্তর্ভাব <cassert>

using নামাকাশ llvm;

শূন্য IntEqClasses::grow(unsigned N) {
  প্রতিজ্ঞা(অঙ্ক_Classes == 0 && "grow() called after compress().");
  EC.reserve(N);
  যখন (EC.আকার_3() < N)
    EC.ঠেলো_back(EC.আকার_3());
}

unsigned IntEqClasses::join(unsigned a, unsigned b) {
  প্রতিজ্ঞা(অঙ্ক_Classes == 0 && "join() called after compress().");
  unsigned eca = EC[a];
  unsigned ecb = EC[b];
  // Update pointers while searching for the leaders, compressing the paths
  // incrementally. The larger leader will eventually be updated, joining the
  // classes.
  যখন (eca != ecb)
    যদি (eca < ecb) {
      EC[b] = eca;
      b = ecb;
      ecb = EC[b];
    } অন্যথা {
      EC[a] = ecb;
      a = eca;
      eca = EC[a];
    }

  ফেরত eca;
}

unsigned IntEqClasses::findLeader(unsigned a) ধ্রুবক {
  প্রতিজ্ঞা(অঙ্ক_Classes == 0 && "findLeader() called after compress().");
  যখন (a != EC[a])
    a = EC[a];
  ফেরত a;
}

শূন্য IntEqClasses::compress() {
  যদি (অঙ্ক_Classes)
    ফেরত;
  জন্য (unsigned i = 0, e = EC.আকার_3(); i != e; ++i)
    EC[i] = (EC[i] == i) ? অঙ্ক_Classes++ : EC[EC[i]];
}

শূন্য IntEqClasses::uncompress() {
  যদি (!অঙ্ক_Classes)
    ফেরত;
  SmallVector<unsigned, 8> Leader;
  জন্য (unsigned i = 0, e = EC.আকার_3(); i != e; ++i)
    যদি (EC[i] < Leader.আকার_3())
      EC[i] = Leader[EC[i]];
    অন্যথা
      Leader.ঠেলো_back(EC[i] = i);
  অঙ্ক_Classes = 0;
}
