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

#અન્તર્ભાવ "llvm/ADT/IntEqClasses.h"
#અન્તર્ભાવ <cassert>

using નામાકાશ llvm;

શૂન્ય IntEqClasses::grow(unsigned N) {
  પ્રતિજ્ઞા(અંક_Classes == 0 && "grow() called after compress().");
  EC.reserve(N);
  જ્યારે (EC.size() < N)
    EC.ધકેલો_back(EC.size());
}

unsigned IntEqClasses::join(unsigned a, unsigned b) {
  પ્રતિજ્ઞા(અંક_Classes == 0 && "join() called after compress().");
  unsigned eca = EC[a];
  unsigned ecb = EC[b];
  // Update pointers while searching for the leaders, compressing the paths
  // incrementally. The larger leader will eventually be updated, joining the
  // classes.
  જ્યારે (eca != ecb)
    જો (eca < ecb) {
      EC[b] = eca;
      b = ecb;
      ecb = EC[b];
    } નહીંતર {
      EC[a] = ecb;
      a = eca;
      eca = EC[a];
    }

  પાછા eca;
}

unsigned IntEqClasses::findLeader(unsigned a) અચલ {
  પ્રતિજ્ઞા(અંક_Classes == 0 && "findLeader() called after compress().");
  જ્યારે (a != EC[a])
    a = EC[a];
  પાછા a;
}

શૂન્ય IntEqClasses::compress() {
  જો (અંક_Classes)
    પાછા;
  માટે (unsigned i = 0, e = EC.size(); i != e; ++i)
    EC[i] = (EC[i] == i) ? અંક_Classes++ : EC[EC[i]];
}

શૂન્ય IntEqClasses::uncompress() {
  જો (!અંક_Classes)
    પાછા;
  SmallVector<unsigned, 8> Leader;
  માટે (unsigned i = 0, e = EC.size(); i != e; ++i)
    જો (EC[i] < Leader.size())
      EC[i] = Leader[EC[i]];
    નહીંતર
      Leader.ધકેલો_back(EC[i] = i);
  અંક_Classes = 0;
}
