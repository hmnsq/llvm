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

#అన్తర్భావ "llvm/ADT/IntEqClasses.h"
#అన్తర్భావ <cassert>

ఉపయోగించి నామాకాశ llvm;

శూన్య IntEqClasses::grow(గుర్తులేని N) {
  ప్రతిజ్ఞా(అంకె_Classes == 0 && "grow() called after compress().");
  EC.reserve(N);
  అయితే (EC.కొలత_2() < N)
    EC.నెట్టు_వెనుక(EC.కొలత_2());
}

గుర్తులేని IntEqClasses::join(గుర్తులేని a, గుర్తులేని b) {
  ప్రతిజ్ఞా(అంకె_Classes == 0 && "join() called after compress().");
  గుర్తులేని eca = EC[a];
  గుర్తులేని ecb = EC[b];
  // Update pointers while searching for the leaders, compressing the paths
  // incrementally. The larger leader will eventually be updated, joining the
  // classes.
  అయితే (eca != ecb)
    ఒకవేళ (eca < ecb) {
      EC[b] = eca;
      b = ecb;
      ecb = EC[b];
    } లేకపోతే {
      EC[a] = ecb;
      a = eca;
      eca = EC[a];
    }

  తిరిగి eca;
}

గుర్తులేని IntEqClasses::కనుగొను_Leader(గుర్తులేని a) స్థిరరాశి {
  ప్రతిజ్ఞా(అంకె_Classes == 0 && "findLeader() called after compress().");
  అయితే (a != EC[a])
    a = EC[a];
  తిరిగి a;
}

శూన్య IntEqClasses::compress() {
  ఒకవేళ (అంకె_Classes)
    తిరిగి;
  కొరకు (గుర్తులేని i = 0, e = EC.కొలత_2(); i != e; ++i)
    EC[i] = (EC[i] == i) ? అంకె_Classes++ : EC[EC[i]];
}

శూన్య IntEqClasses::uncompress() {
  ఒకవేళ (!అంకె_Classes)
    తిరిగి;
  Small_సదిశ<గుర్తులేని, 8> Leader;
  కొరకు (గుర్తులేని i = 0, e = EC.కొలత_2(); i != e; ++i)
    ఒకవేళ (EC[i] < Leader.కొలత_2())
      EC[i] = Leader[EC[i]];
    లేకపోతే
      Leader.నెట్టు_వెనుక(EC[i] = i);
  అంకె_Classes = 0;
}
