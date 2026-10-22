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

کارول نطاق llvm;

فراغ IntEqClasses::grow(بینښه N) {
  أكد(عدد_Classes == 0 && "grow() called after compress().");
  EC.reserve(N);
  طالما (EC.اندازه_2() < N)
    EC.ټېل_شا(EC.اندازه_2());
}

بینښه IntEqClasses::join(بینښه a, بینښه b) {
  أكد(عدد_Classes == 0 && "join() called after compress().");
  بینښه eca = EC[a];
  بینښه ecb = EC[b];
  // Update pointers while searching for the leaders, compressing the paths
  // incrementally. The larger leader will eventually be updated, joining the
  // classes.
  طالما (eca != ecb)
    إذا (eca < ecb) {
      EC[b] = eca;
      b = ecb;
      ecb = EC[b];
    } وإلا {
      EC[a] = ecb;
      a = eca;
      eca = EC[a];
    }

  أرجع eca;
}

بینښه IntEqClasses::پیدا_کړه_Leader(بینښه a) ثابت {
  أكد(عدد_Classes == 0 && "findLeader() called after compress().");
  طالما (a != EC[a])
    a = EC[a];
  أرجع a;
}

فراغ IntEqClasses::compress() {
  إذا (عدد_Classes)
    أرجع;
  لكل (بینښه i = 0, e = EC.اندازه_2(); i != e; ++i)
    EC[i] = (EC[i] == i) ? عدد_Classes++ : EC[EC[i]];
}

فراغ IntEqClasses::uncompress() {
  إذا (!عدد_Classes)
    أرجع;
  Small_ویکټور<بینښه, 8> Leader;
  لكل (بینښه i = 0, e = EC.اندازه_2(); i != e; ++i)
    إذا (EC[i] < Leader.اندازه_2())
      EC[i] = Leader[EC[i]];
    وإلا
      Leader.ټېل_شا(EC[i] = i);
  عدد_Classes = 0;
}
