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

#include "llvm/ADT/IntEqClasses.h"
#include <cassert>

using namespace llvm;

void IntEqClasses::grow(unsigned N) {
  assert(NumClasses == 0 && "grow() called after compress().");
  EC.reserve(N);
  ᮞᮨᮓᮨᮜ (EC.size() < N)
    EC.push_back(EC.size());
}

unsigned IntEqClasses::join(unsigned a, unsigned b) {
  assert(NumClasses == 0 && "join() called after compress().");
  unsigned eca = EC[a];
  unsigned ecb = EC[b];
  // Update pointers while searching for the leaders, compressing the paths
  // incrementally. The larger leader will eventually be updated, joining the
  // classes.
  ᮞᮨᮓᮨᮜ (eca != ecb)
    ᮚᮦᮔ᮪ (eca < ecb) {
      EC[b] = eca;
      b = ecb;
      ecb = EC[b];
    } ᮜᮤᮚ {
      EC[a] = ecb;
      a = eca;
      eca = EC[a];
    }

  ᮝᮜᮤ eca;
}

unsigned IntEqClasses::findLeader(unsigned a) const {
  assert(NumClasses == 0 && "findLeader() called after compress().");
  ᮞᮨᮓᮨᮜ (a != EC[a])
    a = EC[a];
  ᮝᮜᮤ a;
}

void IntEqClasses::compress() {
  ᮚᮦᮔ᮪ (NumClasses)
    ᮝᮜᮤ;
  for (unsigned i = 0, e = EC.size(); i != e; ++i)
    EC[i] = (EC[i] == i) ? NumClasses++ : EC[EC[i]];
}

void IntEqClasses::uncompress() {
  ᮚᮦᮔ᮪ (!NumClasses)
    ᮝᮜᮤ;
  SmallVector<unsigned, 8> Leader;
  for (unsigned i = 0, e = EC.size(); i != e; ++i)
    ᮚᮦᮔ᮪ (EC[i] < Leader.size())
      EC[i] = Leader[EC[i]];
    ᮜᮤᮚ
      Leader.push_back(EC[i] = i);
  NumClasses = 0;
}
