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

استعمال نطاق llvm;

فراغ IntEqClasses::grow(بنانشان N) {
  أكد(انگ_Classes == 0 && "grow() called after compress().");
  EC.reserve(N);
  طالما (EC.سائيز_2() < N)
    EC.ڌڪو_پٺ(EC.سائيز_2());
}

بنانشان IntEqClasses::join(بنانشان a, بنانشان b) {
  أكد(انگ_Classes == 0 && "join() called after compress().");
  بنانشان eca = EC[a];
  بنانشان ecb = EC[b];
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

بنانشان IntEqClasses::ڳوليو_Leader(بنانشان a) ثابت {
  أكد(انگ_Classes == 0 && "findLeader() called after compress().");
  طالما (a != EC[a])
    a = EC[a];
  أرجع a;
}

فراغ IntEqClasses::compress() {
  إذا (انگ_Classes)
    أرجع;
  لكل (بنانشان i = 0, e = EC.سائيز_2(); i != e; ++i)
    EC[i] = (EC[i] == i) ? انگ_Classes++ : EC[EC[i]];
}

فراغ IntEqClasses::uncompress() {
  إذا (!انگ_Classes)
    أرجع;
  Small_ويڪٽر<بنانشان, 8> Leader;
  لكل (بنانشان i = 0, e = EC.سائيز_2(); i != e; ++i)
    إذا (EC[i] < Leader.سائيز_2())
      EC[i] = Leader[EC[i]];
    وإلا
      Leader.ڌڪو_پٺ(EC[i] = i);
  انگ_Classes = 0;
}
