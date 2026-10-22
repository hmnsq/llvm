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

#ضمن "llvm/ADT/IntEqClasses.h"
#ضمن <cassert>

using نطاق llvm;

فراغ IntEqClasses::grow(unsigned N) {
  أكد(NumClasses == 0 && "grow() called after compress().");
  EC.reserve(N);
  ܟܡܐ (EC.size() < N)
    EC.push_back(EC.size());
}

unsigned IntEqClasses::join(unsigned a, unsigned b) {
  أكد(NumClasses == 0 && "join() called after compress().");
  unsigned eca = EC[a];
  unsigned ecb = EC[b];
  // Update pointers while searching for the leaders, compressing the paths
  // incrementally. The larger leader will eventually be updated, joining the
  // classes.
  ܟܡܐ (eca != ecb)
    ܐܢ (eca < ecb) {
      EC[b] = eca;
      b = ecb;
      ecb = EC[b];
    } ܐܚܪܢܐ {
      EC[a] = ecb;
      a = eca;
      eca = EC[a];
    }

  ܦܢܐ eca;
}

unsigned IntEqClasses::findLeader(unsigned a) ثابت {
  أكد(NumClasses == 0 && "findLeader() called after compress().");
  ܟܡܐ (a != EC[a])
    a = EC[a];
  ܦܢܐ a;
}

فراغ IntEqClasses::compress() {
  ܐܢ (NumClasses)
    ܦܢܐ;
  لكل (unsigned i = 0, e = EC.size(); i != e; ++i)
    EC[i] = (EC[i] == i) ? NumClasses++ : EC[EC[i]];
}

فراغ IntEqClasses::uncompress() {
  ܐܢ (!NumClasses)
    ܦܢܐ;
  SmallVector<unsigned, 8> Leader;
  لكل (unsigned i = 0, e = EC.size(); i != e; ++i)
    ܐܢ (EC[i] < Leader.size())
      EC[i] = Leader[EC[i]];
    ܐܚܪܢܐ
      Leader.push_back(EC[i] = i);
  NumClasses = 0;
}
