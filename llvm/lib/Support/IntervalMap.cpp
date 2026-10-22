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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/IntervalMap.h"
#ಅನ್ತರ್ಭಾವ <cassert>

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ IntervalMapImpl {

ಶೂನ್ಯ Path::replace_ಬೇರು(ಶೂನ್ಯ *ಬೇರು_2, unsigned Size, IdxPair Offsets) {
  ಪ್ರತಿಜ್ಞಾ(!path.empty() && "Can't replace missing root");
  path.front() = Entry(ಬೇರು_2, Size, Offsets.first);
  path.ಸೇರಿಸು(path.begin() + 1, Entry(subtree(0), Offsets.second));
}

ಗಂಟು_Ref Path::get_ಎಡ_Sibling(unsigned Level) ಧ್ರುವ {
  // The root has no siblings.
  ಯದಿ (Level == 0)
    ಪ್ರತ್ಯಾಗಮ ಗಂಟು_Ref();

  // Go up the tree until we can go left.
  unsigned l = Level - 1;
  ಯಾವತ್ (l && path[l].offset == 0)
    --l;

  // We can't go left.
  ಯದಿ (path[l].offset == 0)
    ಪ್ರತ್ಯಾಗಮ ಗಂಟು_Ref();

  // NR is the subtree containing our left sibling.
  ಗಂಟು_Ref NR = path[l].subtree(path[l].offset - 1);

  // Keep right all the way down.
  ಪ್ರತಿ (++l; l != Level; ++l)
    NR = NR.subtree(NR.size() - 1);
  ಪ್ರತ್ಯಾಗಮ NR;
}

ಶೂನ್ಯ Path::move_ಎಡ(unsigned Level) {
  ಪ್ರತಿಜ್ಞಾ(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  unsigned l = 0;
  ಯದಿ (valid()) {
    l = Level - 1;
    ಯಾವತ್ (path[l].offset == 0) {
      ಪ್ರತಿಜ್ಞಾ(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } ಅನ್ಯಥಾ ಯದಿ (height() < Level)
    // end() may have created a height=0 path.
    path.resize(Level + 1, Entry(nullptr, 0, 0));

  // NR is the subtree containing our left sibling.
  --path[l].offset;
  ಗಂಟು_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  ಪ್ರತಿ (++l; l != Level; ++l) {
    path[l] = Entry(NR, NR.size() - 1);
    NR = NR.subtree(NR.size() - 1);
  }
  path[l] = Entry(NR, NR.size() - 1);
}

ಗಂಟು_Ref Path::get_ಬಲ_Sibling(unsigned Level) ಧ್ರುವ {
  // The root has no siblings.
  ಯದಿ (Level == 0)
    ಪ್ರತ್ಯಾಗಮ ಗಂಟು_Ref();

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  ಯಾವತ್ (l && atLastEntry(l))
    --l;

  // We can't go right.
  ಯದಿ (atLastEntry(l))
    ಪ್ರತ್ಯಾಗಮ ಗಂಟು_Ref();

  // NR is the subtree containing our right sibling.
  ಗಂಟು_Ref NR = path[l].subtree(path[l].offset + 1);

  // Keep left all the way down.
  ಪ್ರತಿ (++l; l != Level; ++l)
    NR = NR.subtree(0);
  ಪ್ರತ್ಯಾಗಮ NR;
}

ಶೂನ್ಯ Path::move_ಬಲ(unsigned Level) {
  ಪ್ರತಿಜ್ಞಾ(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  ಯಾವತ್ (l && atLastEntry(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  ಯದಿ (++path[l].offset == path[l].size)
    ಪ್ರತ್ಯಾಗಮ;
  ಗಂಟು_Ref NR = subtree(l);

  ಪ್ರತಿ (++l; l != Level; ++l) {
    path[l] = Entry(NR, 0);
    NR = NR.subtree(0);
  }
  path[l] = Entry(NR, 0);
}


IdxPair distribute(unsigned Nodes, unsigned Elements, unsigned Capacity,
                   ಧ್ರುವ unsigned *CurSize, unsigned NewSize[],
                   unsigned Position, ತಾರ್ಕಿಕ Grow) {
  ಪ್ರತಿಜ್ಞಾ(Elements + Grow <= Nodes * Capacity && "Not enough room for elements");
  ಪ್ರತಿಜ್ಞಾ(Position <= Elements && "Invalid position");
  ಯದಿ (!Nodes)
    ಪ್ರತ್ಯಾಗಮ IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  ಧ್ರುವ unsigned Per_ಗಂಟು = (Elements + Grow) / Nodes;
  ಧ್ರುವ unsigned Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  unsigned Sum = 0;
  ಪ್ರತಿ (unsigned n = 0; n != Nodes; ++n) {
    Sum += NewSize[n] = Per_ಗಂಟು + (n < Extra);
    ಯದಿ (PosPair.first == Nodes && Sum > Position)
      PosPair = IdxPair(n, Position - (Sum - NewSize[n]));
  }
  ಪ್ರತಿಜ್ಞಾ(Sum == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  ಯದಿ (Grow) {
    ಪ್ರತಿಜ್ಞಾ(PosPair.first < Nodes && "Bad algebra");
    ಪ್ರತಿಜ್ಞಾ(NewSize[PosPair.first] && "Too few elements to need Grow");
    --NewSize[PosPair.first];
  }

#ifndef NDEBUG
  Sum = 0;
  ಪ್ರತಿ (unsigned n = 0; n != Nodes; ++n) {
    ಪ್ರತಿಜ್ಞಾ(NewSize[n] <= Capacity && "Overallocated node");
    Sum += NewSize[n];
  }
  ಪ್ರತಿಜ್ಞಾ(Sum == Elements && "Bad distribution sum");
#endif

  ಪ್ರತ್ಯಾಗಮ PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

