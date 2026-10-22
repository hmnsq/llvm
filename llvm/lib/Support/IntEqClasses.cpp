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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಶೂನ್ಯ IntEqClasses::grow(ಚಿಹ್ನೆರಹಿತ N) {
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_Classes == 0 && "grow() called after compress().");
  EC.reserve(N);
  ಆಗ (EC.ಗಾತ್ರ_3() < N)
    EC.ತಳ್ಳು_ಹಿಂಭಾಗ(EC.ಗಾತ್ರ_3());
}

ಚಿಹ್ನೆರಹಿತ IntEqClasses::join(ಚಿಹ್ನೆರಹಿತ a, ಚಿಹ್ನೆರಹಿತ b) {
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_Classes == 0 && "join() called after compress().");
  ಚಿಹ್ನೆರಹಿತ eca = EC[a];
  ಚಿಹ್ನೆರಹಿತ ecb = EC[b];
  // Update pointers while searching for the leaders, compressing the paths
  // incrementally. The larger leader will eventually be updated, joining the
  // classes.
  ಆಗ (eca != ecb)
    ಆದರೆ (eca < ecb) {
      EC[b] = eca;
      b = ecb;
      ecb = EC[b];
    } ಇಲ್ಲದಿದ್ದರೆ {
      EC[a] = ecb;
      a = eca;
      eca = EC[a];
    }

  ಹಿಂತಿರುಗಿ eca;
}

ಚಿಹ್ನೆರಹಿತ IntEqClasses::ಕಂಡುಹಿಡಿ_Leader(ಚಿಹ್ನೆರಹಿತ a) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_Classes == 0 && "findLeader() called after compress().");
  ಆಗ (a != EC[a])
    a = EC[a];
  ಹಿಂತಿರುಗಿ a;
}

ಶೂನ್ಯ IntEqClasses::compress() {
  ಆದರೆ (ಅಂಕೆ_Classes)
    ಹಿಂತಿರುಗಿ;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = EC.ಗಾತ್ರ_3(); i != e; ++i)
    EC[i] = (EC[i] == i) ? ಅಂಕೆ_Classes++ : EC[EC[i]];
}

ಶೂನ್ಯ IntEqClasses::uncompress() {
  ಆದರೆ (!ಅಂಕೆ_Classes)
    ಹಿಂತಿರುಗಿ;
  Small_ಸದಿಶ<ಚಿಹ್ನೆರಹಿತ, 8> Leader;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ i = 0, e = EC.ಗಾತ್ರ_3(); i != e; ++i)
    ಆದರೆ (EC[i] < Leader.ಗಾತ್ರ_3())
      EC[i] = Leader[EC[i]];
    ಇಲ್ಲದಿದ್ದರೆ
      Leader.ತಳ್ಳು_ಹಿಂಭಾಗ(EC[i] = i);
  ಅಂಕೆ_Classes = 0;
}
