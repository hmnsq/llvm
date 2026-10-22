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

#समावेश "llvm/ADT/IntEqClasses.h"
#समावेश <cassert>

using namespace llvm;

व्योम IntEqClasses::grow(अचिन्हित N) {
  निश्चित(अंक_Classes == 0 && "grow() called after compress().");
  EC.reserve(N);
  जबतक (EC.आकार_2() < N)
    EC.धकेलो_back(EC.आकार_2());
}

अचिन्हित IntEqClasses::join(अचिन्हित a, अचिन्हित b) {
  निश्चित(अंक_Classes == 0 && "join() called after compress().");
  अचिन्हित eca = EC[a];
  अचिन्हित ecb = EC[b];
  // Update pointers while searching for the leaders, compressing the paths
  // incrementally. The larger leader will eventually be updated, joining the
  // classes.
  जबतक (eca != ecb)
    अगर (eca < ecb) {
      EC[b] = eca;
      b = ecb;
      ecb = EC[b];
    } अन्यथा {
      EC[a] = ecb;
      a = eca;
      eca = EC[a];
    }

  वापस eca;
}

अचिन्हित IntEqClasses::findLeader(अचिन्हित a) स्थिर {
  निश्चित(अंक_Classes == 0 && "findLeader() called after compress().");
  जबतक (a != EC[a])
    a = EC[a];
  वापस a;
}

व्योम IntEqClasses::compress() {
  अगर (अंक_Classes)
    वापस;
  क्रम (अचिन्हित i = 0, e = EC.आकार_2(); i != e; ++i)
    EC[i] = (EC[i] == i) ? अंक_Classes++ : EC[EC[i]];
}

व्योम IntEqClasses::uncompress() {
  अगर (!अंक_Classes)
    वापस;
  SmallVector<अचिन्हित, 8> Leader;
  क्रम (अचिन्हित i = 0, e = EC.आकार_2(); i != e; ++i)
    अगर (EC[i] < Leader.आकार_2())
      EC[i] = Leader[EC[i]];
    अन्यथा
      Leader.धकेलो_back(EC[i] = i);
  अंक_Classes = 0;
}
