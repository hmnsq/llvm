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

शून्य IntEqClasses::grow(अचिन्हित N) {
  प्रतिज्ञा(अङ्क_Classes == 0 && "grow() called after compress().");
  EC.reserve(N);
  जबसम्म (EC.size() < N)
    EC.थप_back(EC.size());
}

अचिन्हित IntEqClasses::join(अचिन्हित a, अचिन्हित b) {
  प्रतिज्ञा(अङ्क_Classes == 0 && "join() called after compress().");
  अचिन्हित eca = EC[a];
  अचिन्हित ecb = EC[b];
  // Update pointers while searching for the leaders, compressing the paths
  // incrementally. The larger leader will eventually be updated, joining the
  // classes.
  जबसम्म (eca != ecb)
    यदि (eca < ecb) {
      EC[b] = eca;
      b = ecb;
      ecb = EC[b];
    } अन्यथा {
      EC[a] = ecb;
      a = eca;
      eca = EC[a];
    }

  फर्काऊ eca;
}

अचिन्हित IntEqClasses::findLeader(अचिन्हित a) अचर {
  प्रतिज्ञा(अङ्क_Classes == 0 && "findLeader() called after compress().");
  जबसम्म (a != EC[a])
    a = EC[a];
  फर्काऊ a;
}

शून्य IntEqClasses::compress() {
  यदि (अङ्क_Classes)
    फर्काऊ;
  लागि (अचिन्हित i = 0, e = EC.size(); i != e; ++i)
    EC[i] = (EC[i] == i) ? अङ्क_Classes++ : EC[EC[i]];
}

शून्य IntEqClasses::uncompress() {
  यदि (!अङ्क_Classes)
    फर्काऊ;
  SmallVector<अचिन्हित, 8> Leader;
  लागि (अचिन्हित i = 0, e = EC.size(); i != e; ++i)
    यदि (EC[i] < Leader.size())
      EC[i] = Leader[EC[i]];
    अन्यथा
      Leader.थप_back(EC[i] = i);
  अङ्क_Classes = 0;
}
