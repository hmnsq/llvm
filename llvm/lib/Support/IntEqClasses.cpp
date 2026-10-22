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

استعمال نامخلا llvm;

خالی IntEqClasses::grow(غیرعلامتی N) {
  أكد(عدد_Classes == 0 && "grow() called after compress().");
  EC.reserve(N);
  جب (EC.سائز_2() < N)
    EC.ڈالو_عقب(EC.سائز_2());
}

غیرعلامتی IntEqClasses::join(غیرعلامتی a, غیرعلامتی b) {
  أكد(عدد_Classes == 0 && "join() called after compress().");
  غیرعلامتی eca = EC[a];
  غیرعلامتی ecb = EC[b];
  // Update pointers while searching for the leaders, compressing the paths
  // incrementally. The larger leader will eventually be updated, joining the
  // classes.
  جب (eca != ecb)
    اگر (eca < ecb) {
      EC[b] = eca;
      b = ecb;
      ecb = EC[b];
    } ورنہ {
      EC[a] = ecb;
      a = eca;
      eca = EC[a];
    }

  واپس eca;
}

غیرعلامتی IntEqClasses::ڈھونڈو_Leader(غیرعلامتی a) ثابت {
  أكد(عدد_Classes == 0 && "findLeader() called after compress().");
  جب (a != EC[a])
    a = EC[a];
  واپس a;
}

خالی IntEqClasses::compress() {
  اگر (عدد_Classes)
    واپس;
  برائے (غیرعلامتی i = 0, e = EC.سائز_2(); i != e; ++i)
    EC[i] = (EC[i] == i) ? عدد_Classes++ : EC[EC[i]];
}

خالی IntEqClasses::uncompress() {
  اگر (!عدد_Classes)
    واپس;
  Small_سمتیہ<غیرعلامتی, 8> Leader;
  برائے (غیرعلامتی i = 0, e = EC.سائز_2(); i != e; ++i)
    اگر (EC[i] < Leader.سائز_2())
      EC[i] = Leader[EC[i]];
    ورنہ
      Leader.ڈالو_عقب(EC[i] = i);
  عدد_Classes = 0;
}
