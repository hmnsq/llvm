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

#include "llvm/ADT/IntervalMap.h"
#include <cassert>

namespace llvm {
namespace IntervalMapImpl {

void Path::replaceRoot(void *Root, unsigned Size, IdxPair Offsets) {
  assert(!path.empty() && "Can't replace missing root");
  path.front() = Entry(Root, Size, Offsets.first);
  path.insert(path.begin() + 1, Entry(subtree(0), Offsets.second));
}

NodeRef Path::getLeftSibling(unsigned Level) const {
  // The root has no siblings.
  ប្រសិនបើ (Level == 0)
    ត្រឡប់ NodeRef();

  // Go up the tree until we can go left.
  unsigned l = Level - 1;
  ខណៈពេល (l && path[l].offset == 0)
    --l;

  // We can't go left.
  ប្រសិនបើ (path[l].offset == 0)
    ត្រឡប់ NodeRef();

  // NR is the subtree containing our left sibling.
  NodeRef NR = path[l].subtree(path[l].offset - 1);

  // Keep right all the way down.
  សម្រាប់ (++l; l != Level; ++l)
    NR = NR.subtree(NR.size() - 1);
  ត្រឡប់ NR;
}

void Path::moveLeft(unsigned Level) {
  assert(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  unsigned l = 0;
  ប្រសិនបើ (valid()) {
    l = Level - 1;
    ខណៈពេល (path[l].offset == 0) {
      assert(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } បើមិនដូច្នេះ ប្រសិនបើ (height() < Level)
    // end() may have created a height=0 path.
    path.resize(Level + 1, Entry(nullptr, 0, 0));

  // NR is the subtree containing our left sibling.
  --path[l].offset;
  NodeRef NR = subtree(l);

  // Get the rightmost node in the subtree.
  សម្រាប់ (++l; l != Level; ++l) {
    path[l] = Entry(NR, NR.size() - 1);
    NR = NR.subtree(NR.size() - 1);
  }
  path[l] = Entry(NR, NR.size() - 1);
}

NodeRef Path::getRightSibling(unsigned Level) const {
  // The root has no siblings.
  ប្រសិនបើ (Level == 0)
    ត្រឡប់ NodeRef();

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  ខណៈពេល (l && atLastEntry(l))
    --l;

  // We can't go right.
  ប្រសិនបើ (atLastEntry(l))
    ត្រឡប់ NodeRef();

  // NR is the subtree containing our right sibling.
  NodeRef NR = path[l].subtree(path[l].offset + 1);

  // Keep left all the way down.
  សម្រាប់ (++l; l != Level; ++l)
    NR = NR.subtree(0);
  ត្រឡប់ NR;
}

void Path::moveRight(unsigned Level) {
  assert(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  ខណៈពេល (l && atLastEntry(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  ប្រសិនបើ (++path[l].offset == path[l].size)
    ត្រឡប់;
  NodeRef NR = subtree(l);

  សម្រាប់ (++l; l != Level; ++l) {
    path[l] = Entry(NR, 0);
    NR = NR.subtree(0);
  }
  path[l] = Entry(NR, 0);
}


IdxPair distribute(unsigned Nodes, unsigned Elements, unsigned Capacity,
                   const unsigned *CurSize, unsigned NewSize[],
                   unsigned Position, bool Grow) {
  assert(Elements + Grow <= Nodes * Capacity && "Not enough room for elements");
  assert(Position <= Elements && "Invalid position");
  ប្រសិនបើ (!Nodes)
    ត្រឡប់ IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  const unsigned PerNode = (Elements + Grow) / Nodes;
  const unsigned Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  unsigned Sum = 0;
  សម្រាប់ (unsigned n = 0; n != Nodes; ++n) {
    Sum += NewSize[n] = PerNode + (n < Extra);
    ប្រសិនបើ (PosPair.first == Nodes && Sum > Position)
      PosPair = IdxPair(n, Position - (Sum - NewSize[n]));
  }
  assert(Sum == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  ប្រសិនបើ (Grow) {
    assert(PosPair.first < Nodes && "Bad algebra");
    assert(NewSize[PosPair.first] && "Too few elements to need Grow");
    --NewSize[PosPair.first];
  }

#ifndef NDEBUG
  Sum = 0;
  សម្រាប់ (unsigned n = 0; n != Nodes; ++n) {
    assert(NewSize[n] <= Capacity && "Overallocated node");
    Sum += NewSize[n];
  }
  assert(Sum == Elements && "Bad distribution sum");
#endif

  ត្រឡប់ PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

