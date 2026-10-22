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

#அந்தர்பாவ "llvm/ADT/IntervalMap.h"
#அந்தர்பாவ <cassert>

நாமாகாஶ llvm {
நாமாகாஶ IntervalMapImpl {

வெறுமை Path::replace_வேர்(வெறுமை *வேர்_2, unsigned Size, IdxPair Offsets) {
  ப்ரதிஜ்ஞா(!path.empty() && "Can't replace missing root");
  path.front() = Entry(வேர்_2, Size, Offsets.first);
  path.செருகு(path.begin() + 1, Entry(subtree(0), Offsets.second));
}

முனை_Ref Path::get_இடது_Sibling(unsigned Level) மாறிலி {
  // The root has no siblings.
  எனில் (Level == 0)
    திரும்பு முனை_Ref();

  // Go up the tree until we can go left.
  unsigned l = Level - 1;
  போது (l && path[l].offset == 0)
    --l;

  // We can't go left.
  எனில் (path[l].offset == 0)
    திரும்பு முனை_Ref();

  // NR is the subtree containing our left sibling.
  முனை_Ref NR = path[l].subtree(path[l].offset - 1);

  // Keep right all the way down.
  க்கு (++l; l != Level; ++l)
    NR = NR.subtree(NR.size() - 1);
  திரும்பு NR;
}

வெறுமை Path::move_இடது(unsigned Level) {
  ப்ரதிஜ்ஞா(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  unsigned l = 0;
  எனில் (valid()) {
    l = Level - 1;
    போது (path[l].offset == 0) {
      ப்ரதிஜ்ஞா(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } இல்லையெனில் எனில் (height() < Level)
    // end() may have created a height=0 path.
    path.resize(Level + 1, Entry(nullptr, 0, 0));

  // NR is the subtree containing our left sibling.
  --path[l].offset;
  முனை_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  க்கு (++l; l != Level; ++l) {
    path[l] = Entry(NR, NR.size() - 1);
    NR = NR.subtree(NR.size() - 1);
  }
  path[l] = Entry(NR, NR.size() - 1);
}

முனை_Ref Path::get_வலது_Sibling(unsigned Level) மாறிலி {
  // The root has no siblings.
  எனில் (Level == 0)
    திரும்பு முனை_Ref();

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  போது (l && atLastEntry(l))
    --l;

  // We can't go right.
  எனில் (atLastEntry(l))
    திரும்பு முனை_Ref();

  // NR is the subtree containing our right sibling.
  முனை_Ref NR = path[l].subtree(path[l].offset + 1);

  // Keep left all the way down.
  க்கு (++l; l != Level; ++l)
    NR = NR.subtree(0);
  திரும்பு NR;
}

வெறுமை Path::move_வலது(unsigned Level) {
  ப்ரதிஜ்ஞா(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  போது (l && atLastEntry(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  எனில் (++path[l].offset == path[l].size)
    திரும்பு;
  முனை_Ref NR = subtree(l);

  க்கு (++l; l != Level; ++l) {
    path[l] = Entry(NR, 0);
    NR = NR.subtree(0);
  }
  path[l] = Entry(NR, 0);
}


IdxPair distribute(unsigned Nodes, unsigned Elements, unsigned Capacity,
                   மாறிலி unsigned *CurSize, unsigned NewSize[],
                   unsigned Position, தருக்க Grow) {
  ப்ரதிஜ்ஞா(Elements + Grow <= Nodes * Capacity && "Not enough room for elements");
  ப்ரதிஜ்ஞா(Position <= Elements && "Invalid position");
  எனில் (!Nodes)
    திரும்பு IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  மாறிலி unsigned Per_முனை = (Elements + Grow) / Nodes;
  மாறிலி unsigned Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  unsigned Sum = 0;
  க்கு (unsigned n = 0; n != Nodes; ++n) {
    Sum += NewSize[n] = Per_முனை + (n < Extra);
    எனில் (PosPair.first == Nodes && Sum > Position)
      PosPair = IdxPair(n, Position - (Sum - NewSize[n]));
  }
  ப்ரதிஜ்ஞா(Sum == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  எனில் (Grow) {
    ப்ரதிஜ்ஞா(PosPair.first < Nodes && "Bad algebra");
    ப்ரதிஜ்ஞா(NewSize[PosPair.first] && "Too few elements to need Grow");
    --NewSize[PosPair.first];
  }

#ifndef NDEBUG
  Sum = 0;
  க்கு (unsigned n = 0; n != Nodes; ++n) {
    ப்ரதிஜ்ஞா(NewSize[n] <= Capacity && "Overallocated node");
    Sum += NewSize[n];
  }
  ப்ரதிஜ்ஞா(Sum == Elements && "Bad distribution sum");
#endif

  திரும்பு PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

