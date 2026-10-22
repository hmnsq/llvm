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

#અન્તર્ભાવ "llvm/ADT/IntervalMap.h"
#અન્તર્ભાવ <cassert>

નામાકાશ llvm {
નામાકાશ IntervalMapImpl {

શૂન્ય Path::replace_મૂળ(શૂન્ય *મૂળ_2, unsigned Size, IdxPair Offsets) {
  પ્રતિજ્ઞા(!path.empty() && "Can't replace missing root");
  path.front() = Entry(મૂળ_2, Size, Offsets.first);
  path.ઉમેરો(path.begin() + 1, Entry(subtree(0), Offsets.second));
}

ગાંઠ_Ref Path::get_ડાબું_Sibling(unsigned Level) અચલ {
  // The root has no siblings.
  જો (Level == 0)
    પાછા ગાંઠ_Ref();

  // Go up the tree until we can go left.
  unsigned l = Level - 1;
  જ્યારે (l && path[l].offset == 0)
    --l;

  // We can't go left.
  જો (path[l].offset == 0)
    પાછા ગાંઠ_Ref();

  // NR is the subtree containing our left sibling.
  ગાંઠ_Ref NR = path[l].subtree(path[l].offset - 1);

  // Keep right all the way down.
  માટે (++l; l != Level; ++l)
    NR = NR.subtree(NR.size() - 1);
  પાછા NR;
}

શૂન્ય Path::move_ડાબું(unsigned Level) {
  પ્રતિજ્ઞા(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  unsigned l = 0;
  જો (valid()) {
    l = Level - 1;
    જ્યારે (path[l].offset == 0) {
      પ્રતિજ્ઞા(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } નહીંતર જો (height() < Level)
    // end() may have created a height=0 path.
    path.resize(Level + 1, Entry(nullptr, 0, 0));

  // NR is the subtree containing our left sibling.
  --path[l].offset;
  ગાંઠ_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  માટે (++l; l != Level; ++l) {
    path[l] = Entry(NR, NR.size() - 1);
    NR = NR.subtree(NR.size() - 1);
  }
  path[l] = Entry(NR, NR.size() - 1);
}

ગાંઠ_Ref Path::get_જમણું_Sibling(unsigned Level) અચલ {
  // The root has no siblings.
  જો (Level == 0)
    પાછા ગાંઠ_Ref();

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  જ્યારે (l && atLastEntry(l))
    --l;

  // We can't go right.
  જો (atLastEntry(l))
    પાછા ગાંઠ_Ref();

  // NR is the subtree containing our right sibling.
  ગાંઠ_Ref NR = path[l].subtree(path[l].offset + 1);

  // Keep left all the way down.
  માટે (++l; l != Level; ++l)
    NR = NR.subtree(0);
  પાછા NR;
}

શૂન્ય Path::move_જમણું(unsigned Level) {
  પ્રતિજ્ઞા(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  જ્યારે (l && atLastEntry(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  જો (++path[l].offset == path[l].size)
    પાછા;
  ગાંઠ_Ref NR = subtree(l);

  માટે (++l; l != Level; ++l) {
    path[l] = Entry(NR, 0);
    NR = NR.subtree(0);
  }
  path[l] = Entry(NR, 0);
}


IdxPair distribute(unsigned Nodes, unsigned Elements, unsigned Capacity,
                   અચલ unsigned *CurSize, unsigned NewSize[],
                   unsigned Position, તાર્કિક Grow) {
  પ્રતિજ્ઞા(Elements + Grow <= Nodes * Capacity && "Not enough room for elements");
  પ્રતિજ્ઞા(Position <= Elements && "Invalid position");
  જો (!Nodes)
    પાછા IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  અચલ unsigned Per_ગાંઠ = (Elements + Grow) / Nodes;
  અચલ unsigned Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  unsigned Sum = 0;
  માટે (unsigned n = 0; n != Nodes; ++n) {
    Sum += NewSize[n] = Per_ગાંઠ + (n < Extra);
    જો (PosPair.first == Nodes && Sum > Position)
      PosPair = IdxPair(n, Position - (Sum - NewSize[n]));
  }
  પ્રતિજ્ઞા(Sum == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  જો (Grow) {
    પ્રતિજ્ઞા(PosPair.first < Nodes && "Bad algebra");
    પ્રતિજ્ઞા(NewSize[PosPair.first] && "Too few elements to need Grow");
    --NewSize[PosPair.first];
  }

#ifndef NDEBUG
  Sum = 0;
  માટે (unsigned n = 0; n != Nodes; ++n) {
    પ્રતિજ્ઞા(NewSize[n] <= Capacity && "Overallocated node");
    Sum += NewSize[n];
  }
  પ્રતિજ્ઞા(Sum == Elements && "Bad distribution sum");
#endif

  પાછા PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

