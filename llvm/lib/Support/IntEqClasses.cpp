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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/IntEqClasses.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਖਾਲੀ IntEqClasses::grow(ਬਿਨਾਨਿਸ਼ਾਨੀ N) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਕ_Classes == 0 && "grow() called after compress().");
  EC.reserve(N);
  ਜਦੋਂ (EC.ਮਾਪ_2() < N)
    EC.ਧੱਕੋ_ਪਿੱਠ(EC.ਮਾਪ_2());
}

ਬਿਨਾਨਿਸ਼ਾਨੀ IntEqClasses::join(ਬਿਨਾਨਿਸ਼ਾਨੀ a, ਬਿਨਾਨਿਸ਼ਾਨੀ b) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਕ_Classes == 0 && "join() called after compress().");
  ਬਿਨਾਨਿਸ਼ਾਨੀ eca = EC[a];
  ਬਿਨਾਨਿਸ਼ਾਨੀ ecb = EC[b];
  // Update pointers while searching for the leaders, compressing the paths
  // incrementally. The larger leader will eventually be updated, joining the
  // classes.
  ਜਦੋਂ (eca != ecb)
    ਜੇ (eca < ecb) {
      EC[b] = eca;
      b = ecb;
      ecb = EC[b];
    } ਨਹੀਂਤਾਂ {
      EC[a] = ecb;
      a = eca;
      eca = EC[a];
    }

  ਵਾਪਸ eca;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ IntEqClasses::ਲੱਭੋ_Leader(ਬਿਨਾਨਿਸ਼ਾਨੀ a) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਕ_Classes == 0 && "findLeader() called after compress().");
  ਜਦੋਂ (a != EC[a])
    a = EC[a];
  ਵਾਪਸ a;
}

ਖਾਲੀ IntEqClasses::compress() {
  ਜੇ (ਅੰਕ_Classes)
    ਵਾਪਸ;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = EC.ਮਾਪ_2(); i != e; ++i)
    EC[i] = (EC[i] == i) ? ਅੰਕ_Classes++ : EC[EC[i]];
}

ਖਾਲੀ IntEqClasses::uncompress() {
  ਜੇ (!ਅੰਕ_Classes)
    ਵਾਪਸ;
  Small_ਸਦਿਸ਼<ਬਿਨਾਨਿਸ਼ਾਨੀ, 8> Leader;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ i = 0, e = EC.ਮਾਪ_2(); i != e; ++i)
    ਜੇ (EC[i] < Leader.ਮਾਪ_2())
      EC[i] = Leader[EC[i]];
    ਨਹੀਂਤਾਂ
      Leader.ਧੱਕੋ_ਪਿੱਠ(EC[i] = i);
  ਅੰਕ_Classes = 0;
}
