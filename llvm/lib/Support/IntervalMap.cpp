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

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/IntervalMap.h"
#ଅନ୍ତର୍ଭାଵ <cassert>

ନାମାକାଶ llvm {
ନାମାକାଶ IntervalMapImpl {

ଶୂନ୍ୟ Path::replace_ମୂଳ(ଶୂନ୍ୟ *ମୂଳ_2, unsigned Size, IdxPair Offsets) {
  ପ୍ରତିଜ୍ଞା(!path.empty() && "Can't replace missing root");
  path.front() = Entry(ମୂଳ_2, Size, Offsets.first);
  path.ଭର୍ତ୍ତି(path.begin() + 1, Entry(subtree(0), Offsets.second));
}

ଗଣ୍ଠି_Ref Path::get_ବାମ_Sibling(unsigned Level) ଅଚଳ {
  // The root has no siblings.
  ଯଦି (Level == 0)
    ଫେରନ୍ତୁ ଗଣ୍ଠି_Ref();

  // Go up the tree until we can go left.
  unsigned l = Level - 1;
  ଯେତେବେଳେ (l && path[l].offset == 0)
    --l;

  // We can't go left.
  ଯଦି (path[l].offset == 0)
    ଫେରନ୍ତୁ ଗଣ୍ଠି_Ref();

  // NR is the subtree containing our left sibling.
  ଗଣ୍ଠି_Ref NR = path[l].subtree(path[l].offset - 1);

  // Keep right all the way down.
  ପାଇଁ (++l; l != Level; ++l)
    NR = NR.subtree(NR.size() - 1);
  ଫେରନ୍ତୁ NR;
}

ଶୂନ୍ୟ Path::move_ବାମ(unsigned Level) {
  ପ୍ରତିଜ୍ଞା(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  unsigned l = 0;
  ଯଦି (valid()) {
    l = Level - 1;
    ଯେତେବେଳେ (path[l].offset == 0) {
      ପ୍ରତିଜ୍ଞା(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } ଅନ୍ୟଥା ଯଦି (height() < Level)
    // end() may have created a height=0 path.
    path.resize(Level + 1, Entry(nullptr, 0, 0));

  // NR is the subtree containing our left sibling.
  --path[l].offset;
  ଗଣ୍ଠି_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  ପାଇଁ (++l; l != Level; ++l) {
    path[l] = Entry(NR, NR.size() - 1);
    NR = NR.subtree(NR.size() - 1);
  }
  path[l] = Entry(NR, NR.size() - 1);
}

ଗଣ୍ଠି_Ref Path::get_ଡାହାଣ_Sibling(unsigned Level) ଅଚଳ {
  // The root has no siblings.
  ଯଦି (Level == 0)
    ଫେରନ୍ତୁ ଗଣ୍ଠି_Ref();

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  ଯେତେବେଳେ (l && atLastEntry(l))
    --l;

  // We can't go right.
  ଯଦି (atLastEntry(l))
    ଫେରନ୍ତୁ ଗଣ୍ଠି_Ref();

  // NR is the subtree containing our right sibling.
  ଗଣ୍ଠି_Ref NR = path[l].subtree(path[l].offset + 1);

  // Keep left all the way down.
  ପାଇଁ (++l; l != Level; ++l)
    NR = NR.subtree(0);
  ଫେରନ୍ତୁ NR;
}

ଶୂନ୍ୟ Path::move_ଡାହାଣ(unsigned Level) {
  ପ୍ରତିଜ୍ଞା(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  ଯେତେବେଳେ (l && atLastEntry(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  ଯଦି (++path[l].offset == path[l].size)
    ଫେରନ୍ତୁ;
  ଗଣ୍ଠି_Ref NR = subtree(l);

  ପାଇଁ (++l; l != Level; ++l) {
    path[l] = Entry(NR, 0);
    NR = NR.subtree(0);
  }
  path[l] = Entry(NR, 0);
}


IdxPair distribute(unsigned Nodes, unsigned Elements, unsigned Capacity,
                   ଅଚଳ unsigned *CurSize, unsigned NewSize[],
                   unsigned Position, ତାର୍କିକ Grow) {
  ପ୍ରତିଜ୍ଞା(Elements + Grow <= Nodes * Capacity && "Not enough room for elements");
  ପ୍ରତିଜ୍ଞା(Position <= Elements && "Invalid position");
  ଯଦି (!Nodes)
    ଫେରନ୍ତୁ IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  ଅଚଳ unsigned Per_ଗଣ୍ଠି = (Elements + Grow) / Nodes;
  ଅଚଳ unsigned Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  unsigned Sum = 0;
  ପାଇଁ (unsigned n = 0; n != Nodes; ++n) {
    Sum += NewSize[n] = Per_ଗଣ୍ଠି + (n < Extra);
    ଯଦି (PosPair.first == Nodes && Sum > Position)
      PosPair = IdxPair(n, Position - (Sum - NewSize[n]));
  }
  ପ୍ରତିଜ୍ଞା(Sum == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  ଯଦି (Grow) {
    ପ୍ରତିଜ୍ଞା(PosPair.first < Nodes && "Bad algebra");
    ପ୍ରତିଜ୍ଞା(NewSize[PosPair.first] && "Too few elements to need Grow");
    --NewSize[PosPair.first];
  }

#ifndef NDEBUG
  Sum = 0;
  ପାଇଁ (unsigned n = 0; n != Nodes; ++n) {
    ପ୍ରତିଜ୍ଞା(NewSize[n] <= Capacity && "Overallocated node");
    Sum += NewSize[n];
  }
  ପ୍ରତିଜ୍ଞା(Sum == Elements && "Bad distribution sum");
#endif

  ଫେରନ୍ତୁ PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

