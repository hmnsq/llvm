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

#অন্তর্ভাৱ "llvm/ADT/IntervalMap.h"
#অন্তর্ভাৱ <cassert>

নামাকাশ llvm {
নামাকাশ IntervalMapImpl {

শূন্য Path::replace_মূল(শূন্য *মূল_2, unsigned Size, IdxPair Offsets) {
  প্রতিজ্ঞা(!path.empty() && "Can't replace missing root");
  path.front() = Entry(মূল_2, Size, Offsets.first);
  path.ঢোকাও(path.begin() + 1, Entry(subtree(0), Offsets.second));
}

গ্ৰন্থি_Ref Path::get_বাম_Sibling(unsigned Level) ধ্রুৱ {
  // The root has no siblings.
  যদি (Level == 0)
    ঘূৰাই গ্ৰন্থি_Ref();

  // Go up the tree until we can go left.
  unsigned l = Level - 1;
  যেতিয়ালৈ (l && path[l].offset == 0)
    --l;

  // We can't go left.
  যদি (path[l].offset == 0)
    ঘূৰাই গ্ৰন্থি_Ref();

  // NR is the subtree containing our left sibling.
  গ্ৰন্থি_Ref NR = path[l].subtree(path[l].offset - 1);

  // Keep right all the way down.
  বাবে (++l; l != Level; ++l)
    NR = NR.subtree(NR.size() - 1);
  ঘূৰাই NR;
}

শূন্য Path::move_বাম(unsigned Level) {
  প্রতিজ্ঞা(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  unsigned l = 0;
  যদি (valid()) {
    l = Level - 1;
    যেতিয়ালৈ (path[l].offset == 0) {
      প্রতিজ্ঞা(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } অন্যথা যদি (height() < Level)
    // end() may have created a height=0 path.
    path.resize(Level + 1, Entry(nullptr, 0, 0));

  // NR is the subtree containing our left sibling.
  --path[l].offset;
  গ্ৰন্থি_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  বাবে (++l; l != Level; ++l) {
    path[l] = Entry(NR, NR.size() - 1);
    NR = NR.subtree(NR.size() - 1);
  }
  path[l] = Entry(NR, NR.size() - 1);
}

গ্ৰন্থি_Ref Path::get_ডান_Sibling(unsigned Level) ধ্রুৱ {
  // The root has no siblings.
  যদি (Level == 0)
    ঘূৰাই গ্ৰন্থি_Ref();

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  যেতিয়ালৈ (l && atLastEntry(l))
    --l;

  // We can't go right.
  যদি (atLastEntry(l))
    ঘূৰাই গ্ৰন্থি_Ref();

  // NR is the subtree containing our right sibling.
  গ্ৰন্থি_Ref NR = path[l].subtree(path[l].offset + 1);

  // Keep left all the way down.
  বাবে (++l; l != Level; ++l)
    NR = NR.subtree(0);
  ঘূৰাই NR;
}

শূন্য Path::move_ডান(unsigned Level) {
  প্রতিজ্ঞা(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  যেতিয়ালৈ (l && atLastEntry(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  যদি (++path[l].offset == path[l].size)
    ঘূৰাই;
  গ্ৰন্থি_Ref NR = subtree(l);

  বাবে (++l; l != Level; ++l) {
    path[l] = Entry(NR, 0);
    NR = NR.subtree(0);
  }
  path[l] = Entry(NR, 0);
}


IdxPair distribute(unsigned Nodes, unsigned Elements, unsigned Capacity,
                   ধ্রুৱ unsigned *CurSize, unsigned NewSize[],
                   unsigned Position, তার্কিক Grow) {
  প্রতিজ্ঞা(Elements + Grow <= Nodes * Capacity && "Not enough room for elements");
  প্রতিজ্ঞা(Position <= Elements && "Invalid position");
  যদি (!Nodes)
    ঘূৰাই IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  ধ্রুৱ unsigned Per_গ্ৰন্থি = (Elements + Grow) / Nodes;
  ধ্রুৱ unsigned Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  unsigned Sum = 0;
  বাবে (unsigned n = 0; n != Nodes; ++n) {
    Sum += NewSize[n] = Per_গ্ৰন্থি + (n < Extra);
    যদি (PosPair.first == Nodes && Sum > Position)
      PosPair = IdxPair(n, Position - (Sum - NewSize[n]));
  }
  প্রতিজ্ঞা(Sum == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  যদি (Grow) {
    প্রতিজ্ঞা(PosPair.first < Nodes && "Bad algebra");
    প্রতিজ্ঞা(NewSize[PosPair.first] && "Too few elements to need Grow");
    --NewSize[PosPair.first];
  }

#ifndef NDEBUG
  Sum = 0;
  বাবে (unsigned n = 0; n != Nodes; ++n) {
    প্রতিজ্ঞা(NewSize[n] <= Capacity && "Overallocated node");
    Sum += NewSize[n];
  }
  প্রতিজ্ঞা(Sum == Elements && "Bad distribution sum");
#endif

  ঘূৰাই PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

