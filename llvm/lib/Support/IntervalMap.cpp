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

#അന്തര്ഭാവ "llvm/ADT/IntervalMap.h"
#അന്തര്ഭാവ <cassert>

നാമാകാശ llvm {
നാമാകാശ IntervalMapImpl {

ശൂന്യം Path::replace_വേര്(ശൂന്യം *വേര്_2, unsigned Size, IdxPair Offsets) {
  പ്രതിജ്ഞാ(!path.empty() && "Can't replace missing root");
  path.front() = Entry(വേര്_2, Size, Offsets.first);
  path.തിരുകുക(path.begin() + 1, Entry(subtree(0), Offsets.second));
}

കണ്ണി_Ref Path::get_ഇടത്_Sibling(unsigned Level) സ്ഥിരം {
  // The root has no siblings.
  എങ്കിൽ (Level == 0)
    തിരികെ കണ്ണി_Ref();

  // Go up the tree until we can go left.
  unsigned l = Level - 1;
  ആകുമ്പോൾ (l && path[l].offset == 0)
    --l;

  // We can't go left.
  എങ്കിൽ (path[l].offset == 0)
    തിരികെ കണ്ണി_Ref();

  // NR is the subtree containing our left sibling.
  കണ്ണി_Ref NR = path[l].subtree(path[l].offset - 1);

  // Keep right all the way down.
  വേണ്ടി (++l; l != Level; ++l)
    NR = NR.subtree(NR.size() - 1);
  തിരികെ NR;
}

ശൂന്യം Path::move_ഇടത്(unsigned Level) {
  പ്രതിജ്ഞാ(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  unsigned l = 0;
  എങ്കിൽ (valid()) {
    l = Level - 1;
    ആകുമ്പോൾ (path[l].offset == 0) {
      പ്രതിജ്ഞാ(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } അല്ലെങ്കിൽ എങ്കിൽ (height() < Level)
    // end() may have created a height=0 path.
    path.resize(Level + 1, Entry(nullptr, 0, 0));

  // NR is the subtree containing our left sibling.
  --path[l].offset;
  കണ്ണി_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  വേണ്ടി (++l; l != Level; ++l) {
    path[l] = Entry(NR, NR.size() - 1);
    NR = NR.subtree(NR.size() - 1);
  }
  path[l] = Entry(NR, NR.size() - 1);
}

കണ്ണി_Ref Path::get_വലത്_Sibling(unsigned Level) സ്ഥിരം {
  // The root has no siblings.
  എങ്കിൽ (Level == 0)
    തിരികെ കണ്ണി_Ref();

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  ആകുമ്പോൾ (l && atLastEntry(l))
    --l;

  // We can't go right.
  എങ്കിൽ (atLastEntry(l))
    തിരികെ കണ്ണി_Ref();

  // NR is the subtree containing our right sibling.
  കണ്ണി_Ref NR = path[l].subtree(path[l].offset + 1);

  // Keep left all the way down.
  വേണ്ടി (++l; l != Level; ++l)
    NR = NR.subtree(0);
  തിരികെ NR;
}

ശൂന്യം Path::move_വലത്(unsigned Level) {
  പ്രതിജ്ഞാ(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  ആകുമ്പോൾ (l && atLastEntry(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  എങ്കിൽ (++path[l].offset == path[l].size)
    തിരികെ;
  കണ്ണി_Ref NR = subtree(l);

  വേണ്ടി (++l; l != Level; ++l) {
    path[l] = Entry(NR, 0);
    NR = NR.subtree(0);
  }
  path[l] = Entry(NR, 0);
}


IdxPair distribute(unsigned Nodes, unsigned Elements, unsigned Capacity,
                   സ്ഥിരം unsigned *CurSize, unsigned NewSize[],
                   unsigned Position, തർക്കം Grow) {
  പ്രതിജ്ഞാ(Elements + Grow <= Nodes * Capacity && "Not enough room for elements");
  പ്രതിജ്ഞാ(Position <= Elements && "Invalid position");
  എങ്കിൽ (!Nodes)
    തിരികെ IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  സ്ഥിരം unsigned Per_കണ്ണി = (Elements + Grow) / Nodes;
  സ്ഥിരം unsigned Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  unsigned Sum = 0;
  വേണ്ടി (unsigned n = 0; n != Nodes; ++n) {
    Sum += NewSize[n] = Per_കണ്ണി + (n < Extra);
    എങ്കിൽ (PosPair.first == Nodes && Sum > Position)
      PosPair = IdxPair(n, Position - (Sum - NewSize[n]));
  }
  പ്രതിജ്ഞാ(Sum == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  എങ്കിൽ (Grow) {
    പ്രതിജ്ഞാ(PosPair.first < Nodes && "Bad algebra");
    പ്രതിജ്ഞാ(NewSize[PosPair.first] && "Too few elements to need Grow");
    --NewSize[PosPair.first];
  }

#ifndef NDEBUG
  Sum = 0;
  വേണ്ടി (unsigned n = 0; n != Nodes; ++n) {
    പ്രതിജ്ഞാ(NewSize[n] <= Capacity && "Overallocated node");
    Sum += NewSize[n];
  }
  പ്രതിജ്ഞാ(Sum == Elements && "Bad distribution sum");
#endif

  തിരികെ PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

