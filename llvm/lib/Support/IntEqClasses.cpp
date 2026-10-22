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

#அந்தர்பாவ "llvm/ADT/IntEqClasses.h"
#அந்தர்பாவ <cassert>

using நாமாகாஶ llvm;

வெறுமை IntEqClasses::grow(unsigned N) {
  ப்ரதிஜ்ஞா(இலக்கம்_Classes == 0 && "grow() called after compress().");
  EC.reserve(N);
  போது (EC.size() < N)
    EC.தள்ளு_back(EC.size());
}

unsigned IntEqClasses::join(unsigned a, unsigned b) {
  ப்ரதிஜ்ஞா(இலக்கம்_Classes == 0 && "join() called after compress().");
  unsigned eca = EC[a];
  unsigned ecb = EC[b];
  // Update pointers while searching for the leaders, compressing the paths
  // incrementally. The larger leader will eventually be updated, joining the
  // classes.
  போது (eca != ecb)
    எனில் (eca < ecb) {
      EC[b] = eca;
      b = ecb;
      ecb = EC[b];
    } இல்லையெனில் {
      EC[a] = ecb;
      a = eca;
      eca = EC[a];
    }

  திரும்பு eca;
}

unsigned IntEqClasses::findLeader(unsigned a) மாறிலி {
  ப்ரதிஜ்ஞா(இலக்கம்_Classes == 0 && "findLeader() called after compress().");
  போது (a != EC[a])
    a = EC[a];
  திரும்பு a;
}

வெறுமை IntEqClasses::compress() {
  எனில் (இலக்கம்_Classes)
    திரும்பு;
  க்கு (unsigned i = 0, e = EC.size(); i != e; ++i)
    EC[i] = (EC[i] == i) ? இலக்கம்_Classes++ : EC[EC[i]];
}

வெறுமை IntEqClasses::uncompress() {
  எனில் (!இலக்கம்_Classes)
    திரும்பு;
  SmallVector<unsigned, 8> Leader;
  க்கு (unsigned i = 0, e = EC.size(); i != e; ++i)
    எனில் (EC[i] < Leader.size())
      EC[i] = Leader[EC[i]];
    இல்லையெனில்
      Leader.தள்ளு_back(EC[i] = i);
  இலக்கம்_Classes = 0;
}
