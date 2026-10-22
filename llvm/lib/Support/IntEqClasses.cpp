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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/IntEqClasses.h"
#ಅನ್ತರ್ಭಾವ <cassert>

using ನಾಮಾಕಾಶ llvm;

ಶೂನ್ಯ IntEqClasses::grow(unsigned N) {
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_Classes == 0 && "grow() called after compress().");
  EC.reserve(N);
  ಯಾವತ್ (EC.size() < N)
    EC.ತಳ್ಳು_back(EC.size());
}

unsigned IntEqClasses::join(unsigned a, unsigned b) {
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_Classes == 0 && "join() called after compress().");
  unsigned eca = EC[a];
  unsigned ecb = EC[b];
  // Update pointers while searching for the leaders, compressing the paths
  // incrementally. The larger leader will eventually be updated, joining the
  // classes.
  ಯಾವತ್ (eca != ecb)
    ಯದಿ (eca < ecb) {
      EC[b] = eca;
      b = ecb;
      ecb = EC[b];
    } ಅನ್ಯಥಾ {
      EC[a] = ecb;
      a = eca;
      eca = EC[a];
    }

  ಪ್ರತ್ಯಾಗಮ eca;
}

unsigned IntEqClasses::findLeader(unsigned a) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_Classes == 0 && "findLeader() called after compress().");
  ಯಾವತ್ (a != EC[a])
    a = EC[a];
  ಪ್ರತ್ಯಾಗಮ a;
}

ಶೂನ್ಯ IntEqClasses::compress() {
  ಯದಿ (ಅಂಕೆ_Classes)
    ಪ್ರತ್ಯಾಗಮ;
  ಪ್ರತಿ (unsigned i = 0, e = EC.size(); i != e; ++i)
    EC[i] = (EC[i] == i) ? ಅಂಕೆ_Classes++ : EC[EC[i]];
}

ಶೂನ್ಯ IntEqClasses::uncompress() {
  ಯದಿ (!ಅಂಕೆ_Classes)
    ಪ್ರತ್ಯಾಗಮ;
  SmallVector<unsigned, 8> Leader;
  ಪ್ರತಿ (unsigned i = 0, e = EC.size(); i != e; ++i)
    ಯದಿ (EC[i] < Leader.size())
      EC[i] = Leader[EC[i]];
    ಅನ್ಯಥಾ
      Leader.ತಳ್ಳು_back(EC[i] = i);
  ಅಂಕೆ_Classes = 0;
}
