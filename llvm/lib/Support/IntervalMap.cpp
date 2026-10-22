//===- lib/Support/IntervalMap.cpp - A sorted interval map ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the few non-templated functions in IntervalMap.
//
//===----------------------------------------------------------------------===//

#شامل "llvm/ADT/IntervalMap.h"
#شامل <cassert>

فضاینام llvm {
فضاینام IntervalMapImpl {

تهی Path::replace_ریشه(تهی *ریشه_2, unsigned Size, IdxPair Offsets) {
  أكد(!path.empty() && "Can't replace missing root");
  path.front() = Entry(ریشه_2, Size, Offsets.first);
  path.درج(path.begin() + 1, Entry(subtree(0), Offsets.second));
}

گره_Ref Path::get_چپ_Sibling(unsigned Level) ثابت {
  // The root has no siblings.
  اگر (Level == 0)
    بازگشت گره_Ref();

  // Go up the tree until we can go left.
  unsigned l = Level - 1;
  وقتی (l && path[l].offset == 0)
    --l;

  // We can't go left.
  اگر (path[l].offset == 0)
    بازگشت گره_Ref();

  // NR is the subtree containing our left sibling.
  گره_Ref NR = path[l].subtree(path[l].offset - 1);

  // Keep right all the way down.
  برای (++l; l != Level; ++l)
    NR = NR.subtree(NR.size() - 1);
  بازگشت NR;
}

تهی Path::move_چپ(unsigned Level) {
  أكد(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  unsigned l = 0;
  اگر (valid()) {
    l = Level - 1;
    وقتی (path[l].offset == 0) {
      أكد(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } وگرنه اگر (height() < Level)
    // end() may have created a height=0 path.
    path.resize(Level + 1, Entry(nullptr, 0, 0));

  // NR is the subtree containing our left sibling.
  --path[l].offset;
  گره_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  برای (++l; l != Level; ++l) {
    path[l] = Entry(NR, NR.size() - 1);
    NR = NR.subtree(NR.size() - 1);
  }
  path[l] = Entry(NR, NR.size() - 1);
}

گره_Ref Path::get_راست_Sibling(unsigned Level) ثابت {
  // The root has no siblings.
  اگر (Level == 0)
    بازگشت گره_Ref();

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  وقتی (l && atLastEntry(l))
    --l;

  // We can't go right.
  اگر (atLastEntry(l))
    بازگشت گره_Ref();

  // NR is the subtree containing our right sibling.
  گره_Ref NR = path[l].subtree(path[l].offset + 1);

  // Keep left all the way down.
  برای (++l; l != Level; ++l)
    NR = NR.subtree(0);
  بازگشت NR;
}

تهی Path::move_راست(unsigned Level) {
  أكد(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  وقتی (l && atLastEntry(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  اگر (++path[l].offset == path[l].size)
    بازگشت;
  گره_Ref NR = subtree(l);

  برای (++l; l != Level; ++l) {
    path[l] = Entry(NR, 0);
    NR = NR.subtree(0);
  }
  path[l] = Entry(NR, 0);
}


IdxPair distribute(unsigned Nodes, unsigned Elements, unsigned Capacity,
                   ثابت unsigned *CurSize, unsigned NewSize[],
                   unsigned Position, منطقی Grow) {
  أكد(Elements + Grow <= Nodes * Capacity && "Not enough room for elements");
  أكد(Position <= Elements && "Invalid position");
  اگر (!Nodes)
    بازگشت IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  ثابت unsigned Per_گره = (Elements + Grow) / Nodes;
  ثابت unsigned Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  unsigned Sum = 0;
  برای (unsigned n = 0; n != Nodes; ++n) {
    Sum += NewSize[n] = Per_گره + (n < Extra);
    اگر (PosPair.first == Nodes && Sum > Position)
      PosPair = IdxPair(n, Position - (Sum - NewSize[n]));
  }
  أكد(Sum == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  اگر (Grow) {
    أكد(PosPair.first < Nodes && "Bad algebra");
    أكد(NewSize[PosPair.first] && "Too few elements to need Grow");
    --NewSize[PosPair.first];
  }

#ifndef NDEBUG
  Sum = 0;
  برای (unsigned n = 0; n != Nodes; ++n) {
    أكد(NewSize[n] <= Capacity && "Overallocated node");
    Sum += NewSize[n];
  }
  أكد(Sum == Elements && "Bad distribution sum");
#endif

  بازگشت PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

