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

#अन्तर्भाव "llvm/ADT/IntervalMap.h"
#अन्तर्भाव <cassert>

नामाकाश llvm {
नामाकाश IntervalMapImpl {

शून्य Path::replace_मूलम्(शून्य *मूलम्_2, unsigned Size, IdxPair Offsets) {
  प्रतिज्ञा(!path.empty() && "Can't replace missing root");
  path.front() = Entry(मूलम्_2, Size, Offsets.first);
  path.निवेशय(path.begin() + 1, Entry(subtree(0), Offsets.second));
}

ग्रन्थिः_Ref Path::get_वामः_Sibling(unsigned Level) ध्रुव {
  // The root has no siblings.
  सचे (Level == 0)
    पटिनिवत्त ग्रन्थिः_Ref();

  // Go up the tree until we can go left.
  unsigned l = Level - 1;
  याव (l && path[l].offset == 0)
    --l;

  // We can't go left.
  सचे (path[l].offset == 0)
    पटिनिवत्त ग्रन्थिः_Ref();

  // NR is the subtree containing our left sibling.
  ग्रन्थिः_Ref NR = path[l].subtree(path[l].offset - 1);

  // Keep right all the way down.
  अत्थाय (++l; l != Level; ++l)
    NR = NR.subtree(NR.size() - 1);
  पटिनिवत्त NR;
}

शून्य Path::move_वामः(unsigned Level) {
  प्रतिज्ञा(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  unsigned l = 0;
  सचे (valid()) {
    l = Level - 1;
    याव (path[l].offset == 0) {
      प्रतिज्ञा(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } अञ्ञथा सचे (height() < Level)
    // end() may have created a height=0 path.
    path.resize(Level + 1, Entry(nullptr, 0, 0));

  // NR is the subtree containing our left sibling.
  --path[l].offset;
  ग्रन्थिः_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  अत्थाय (++l; l != Level; ++l) {
    path[l] = Entry(NR, NR.size() - 1);
    NR = NR.subtree(NR.size() - 1);
  }
  path[l] = Entry(NR, NR.size() - 1);
}

ग्रन्थिः_Ref Path::get_दक्षिणः_Sibling(unsigned Level) ध्रुव {
  // The root has no siblings.
  सचे (Level == 0)
    पटिनिवत्त ग्रन्थिः_Ref();

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  याव (l && atLastEntry(l))
    --l;

  // We can't go right.
  सचे (atLastEntry(l))
    पटिनिवत्त ग्रन्थिः_Ref();

  // NR is the subtree containing our right sibling.
  ग्रन्थिः_Ref NR = path[l].subtree(path[l].offset + 1);

  // Keep left all the way down.
  अत्थाय (++l; l != Level; ++l)
    NR = NR.subtree(0);
  पटिनिवत्त NR;
}

शून्य Path::move_दक्षिणः(unsigned Level) {
  प्रतिज्ञा(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  unsigned l = Level - 1;
  याव (l && atLastEntry(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  सचे (++path[l].offset == path[l].size)
    पटिनिवत्त;
  ग्रन्थिः_Ref NR = subtree(l);

  अत्थाय (++l; l != Level; ++l) {
    path[l] = Entry(NR, 0);
    NR = NR.subtree(0);
  }
  path[l] = Entry(NR, 0);
}


IdxPair distribute(unsigned Nodes, unsigned Elements, unsigned Capacity,
                   ध्रुव unsigned *CurSize, unsigned NewSize[],
                   unsigned Position, तार्किक Grow) {
  प्रतिज्ञा(Elements + Grow <= Nodes * Capacity && "Not enough room for elements");
  प्रतिज्ञा(Position <= Elements && "Invalid position");
  सचे (!Nodes)
    पटिनिवत्त IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  ध्रुव unsigned Per_ग्रन्थिः = (Elements + Grow) / Nodes;
  ध्रुव unsigned Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  unsigned Sum = 0;
  अत्थाय (unsigned n = 0; n != Nodes; ++n) {
    Sum += NewSize[n] = Per_ग्रन्थिः + (n < Extra);
    सचे (PosPair.first == Nodes && Sum > Position)
      PosPair = IdxPair(n, Position - (Sum - NewSize[n]));
  }
  प्रतिज्ञा(Sum == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  सचे (Grow) {
    प्रतिज्ञा(PosPair.first < Nodes && "Bad algebra");
    प्रतिज्ञा(NewSize[PosPair.first] && "Too few elements to need Grow");
    --NewSize[PosPair.first];
  }

#ifndef NDEBUG
  Sum = 0;
  अत्थाय (unsigned n = 0; n != Nodes; ++n) {
    प्रतिज्ञा(NewSize[n] <= Capacity && "Overallocated node");
    Sum += NewSize[n];
  }
  प्रतिज्ञा(Sum == Elements && "Bad distribution sum");
#endif

  पटिनिवत्त PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

