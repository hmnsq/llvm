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

#अन्तर्भाव "llvm/ADT/IntEqClasses.h"
#अन्तर्भाव <cassert>

using नामाकाश llvm;

शून्य IntEqClasses::grow(unsigned N) {
  प्रतिज्ञा(अङ्कः_Classes == 0 && "grow() called after compress().");
  EC.reserve(N);
  जाव (EC.size() < N)
    EC.प्रक्षिप_back(EC.size());
}

unsigned IntEqClasses::join(unsigned a, unsigned b) {
  प्रतिज्ञा(अङ्कः_Classes == 0 && "join() called after compress().");
  unsigned eca = EC[a];
  unsigned ecb = EC[b];
  // Update pointers while searching for the leaders, compressing the paths
  // incrementally. The larger leader will eventually be updated, joining the
  // classes.
  जाव (eca != ecb)
    जइ (eca < ecb) {
      EC[b] = eca;
      b = ecb;
      ecb = EC[b];
    } अण्णहा {
      EC[a] = ecb;
      a = eca;
      eca = EC[a];
    }

  प्रत्यागम eca;
}

unsigned IntEqClasses::findLeader(unsigned a) ध्रुव {
  प्रतिज्ञा(अङ्कः_Classes == 0 && "findLeader() called after compress().");
  जाव (a != EC[a])
    a = EC[a];
  प्रत्यागम a;
}

शून्य IntEqClasses::compress() {
  जइ (अङ्कः_Classes)
    प्रत्यागम;
  प्रति (unsigned i = 0, e = EC.size(); i != e; ++i)
    EC[i] = (EC[i] == i) ? अङ्कः_Classes++ : EC[EC[i]];
}

शून्य IntEqClasses::uncompress() {
  जइ (!अङ्कः_Classes)
    प्रत्यागम;
  SmallVector<unsigned, 8> Leader;
  प्रति (unsigned i = 0, e = EC.size(); i != e; ++i)
    जइ (EC[i] < Leader.size())
      EC[i] = Leader[EC[i]];
    अण्णहा
      Leader.प्रक्षिप_back(EC[i] = i);
  अङ्कः_Classes = 0;
}
