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

#അന്തര്ഭാവ "llvm/ADT/IntEqClasses.h"
#അന്തര്ഭാവ <cassert>

using നാമാകാശ llvm;

ശൂന്യം IntEqClasses::grow(unsigned N) {
  പ്രതിജ്ഞാ(അക്കം_Classes == 0 && "grow() called after compress().");
  EC.reserve(N);
  ആകുമ്പോൾ (EC.size() < N)
    EC.തള്ളുക_back(EC.size());
}

unsigned IntEqClasses::join(unsigned a, unsigned b) {
  പ്രതിജ്ഞാ(അക്കം_Classes == 0 && "join() called after compress().");
  unsigned eca = EC[a];
  unsigned ecb = EC[b];
  // Update pointers while searching for the leaders, compressing the paths
  // incrementally. The larger leader will eventually be updated, joining the
  // classes.
  ആകുമ്പോൾ (eca != ecb)
    എങ്കിൽ (eca < ecb) {
      EC[b] = eca;
      b = ecb;
      ecb = EC[b];
    } അല്ലെങ്കിൽ {
      EC[a] = ecb;
      a = eca;
      eca = EC[a];
    }

  തിരികെ eca;
}

unsigned IntEqClasses::findLeader(unsigned a) സ്ഥിരം {
  പ്രതിജ്ഞാ(അക്കം_Classes == 0 && "findLeader() called after compress().");
  ആകുമ്പോൾ (a != EC[a])
    a = EC[a];
  തിരികെ a;
}

ശൂന്യം IntEqClasses::compress() {
  എങ്കിൽ (അക്കം_Classes)
    തിരികെ;
  വേണ്ടി (unsigned i = 0, e = EC.size(); i != e; ++i)
    EC[i] = (EC[i] == i) ? അക്കം_Classes++ : EC[EC[i]];
}

ശൂന്യം IntEqClasses::uncompress() {
  എങ്കിൽ (!അക്കം_Classes)
    തിരികെ;
  SmallVector<unsigned, 8> Leader;
  വേണ്ടി (unsigned i = 0, e = EC.size(); i != e; ++i)
    എങ്കിൽ (EC[i] < Leader.size())
      EC[i] = Leader[EC[i]];
    അല്ലെങ്കിൽ
      Leader.തള്ളുക_back(EC[i] = i);
  അക്കം_Classes = 0;
}
