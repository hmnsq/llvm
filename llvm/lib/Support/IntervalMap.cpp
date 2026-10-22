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

#ضمن "llvm/ADT/IntervalMap.h"
#ضمن <cassert>

نطاق llvm {
نطاق IntervalMapImpl {

فراغ Path::replace_ڕەگ(فراغ *ڕەگ_2, unsigned Size, IdxPair Offsets) {
  أكد(!path.empty() && "Can't replace missing root");
  path.front() = Entry(ڕەگ_2, Size, Offsets.first);
  path.زیادکردن(path.begin() + 1, Entry(subtree(0), Offsets.second));
}

گرێ_Ref Path::get_چەپ_Sibling(unsigned Level) ثابت {
  // The root has no siblings.
  إذا (Level == 0)
    أرجع گرێ_Ref();

  // Go up the tree until we can go left.
  unsigned l = Level - 1;
  طالما (l && path[l].offset == 0)
    --l;

  // We can't go left.
  إذا (path[l].offset == 0)
    أرجع گرێ_Ref();

  // NR is the subtree containing our left sibling.
  گرێ_Ref NR = path[l].subtree(path[l].offset - 1);

  // Keep right all the way down.
  لكل (++l; l != Level; ++l)
    NR = NR.subtree(NR.size() - 1);
  أرجع NR;
}

فراغ Path::move_چەپ(unsigned Level) {
  أكد(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  unsigned l = 0;
  إذا (valid()) {
    l = Level - 1;
    طالما (path[l].offset == 0) {
      أكد(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } وإلا إذا (height() < Level)
    // end() may have created a height=0 path.
    path.resize(Level + 1, Entry(nullptr, 0, 0));

  // NR is the subtree containing our left sibling.
  --path[l].offset;
  گرێ_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  لكل (++l; l != Level; ++l) {
    path[l] = Entry(NR, NR.size() - 1);
    NR = NR.subtree(NR.size() - 1);
  }
  path[l] = Entry(NR, NR.size() - 1);
}

گرێ_Ref Path::get_ڕاست_Sibling(unsigned Level) ثابت {
  // The root has no siblings.
  إذا (Level == 0)
    أرجع گرێ_Ref();

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  طالما (l && atLastEntry(l))
    --l;

  // We can't go right.
  إذا (atLastEntry(l))
    أرجع گرێ_Ref();

  // NR is the subtree containing our right sibling.
  گرێ_Ref NR = path[l].subtree(path[l].offset + 1);

  // Keep left all the way down.
  لكل (++l; l != Level; ++l)
    NR = NR.subtree(0);
  أرجع NR;
}

فراغ Path::move_ڕاست(unsigned Level) {
  أكد(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  طالما (l && atLastEntry(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  إذا (++path[l].offset == path[l].size)
    أرجع;
  گرێ_Ref NR = subtree(l);

  لكل (++l; l != Level; ++l) {
    path[l] = Entry(NR, 0);
    NR = NR.subtree(0);
  }
  path[l] = Entry(NR, 0);
}


IdxPair distribute(unsigned Nodes, unsigned Elements, unsigned Capacity,
                   ثابت unsigned *CurSize, unsigned NewSize[],
                   unsigned Position, منطقي Grow) {
  أكد(Elements + Grow <= Nodes * Capacity && "Not enough room for elements");
  أكد(Position <= Elements && "Invalid position");
  إذا (!Nodes)
    أرجع IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  ثابت unsigned Per_گرێ = (Elements + Grow) / Nodes;
  ثابت unsigned Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  unsigned Sum = 0;
  لكل (unsigned n = 0; n != Nodes; ++n) {
    Sum += NewSize[n] = Per_گرێ + (n < Extra);
    إذا (PosPair.first == Nodes && Sum > Position)
      PosPair = IdxPair(n, Position - (Sum - NewSize[n]));
  }
  أكد(Sum == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  إذا (Grow) {
    أكد(PosPair.first < Nodes && "Bad algebra");
    أكد(NewSize[PosPair.first] && "Too few elements to need Grow");
    --NewSize[PosPair.first];
  }

#ifndef NDEBUG
  Sum = 0;
  لكل (unsigned n = 0; n != Nodes; ++n) {
    أكد(NewSize[n] <= Capacity && "Overallocated node");
    Sum += NewSize[n];
  }
  أكد(Sum == Elements && "Bad distribution sum");
#endif

  أرجع PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

