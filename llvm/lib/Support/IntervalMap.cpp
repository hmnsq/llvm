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

शून्य Path::replace_मूल(शून्य *मूल_2, अचिन्हित Size, IdxPair Offsets) {
  प्रतिज्ञा(!path.empty() && "Can't replace missing root");
  path.front() = Entry(मूल_2, Size, Offsets.first);
  path.जोड़ो(path.begin() + 1, Entry(subtree(0), Offsets.second));
}

गाँठ_Ref Path::get_बायाँ_Sibling(अचिन्हित Level) ध्रुव {
  // The root has no siblings.
  यदि (Level == 0)
    प्रत्यागम गाँठ_Ref();

  // Go up the tree until we can go left.
  अचिन्हित l = Level - 1;
  यावत् (l && path[l].offset == 0)
    --l;

  // We can't go left.
  यदि (path[l].offset == 0)
    प्रत्यागम गाँठ_Ref();

  // NR is the subtree containing our left sibling.
  गाँठ_Ref NR = path[l].subtree(path[l].offset - 1);

  // Keep right all the way down.
  प्रति (++l; l != Level; ++l)
    NR = NR.subtree(NR.size() - 1);
  प्रत्यागम NR;
}

शून्य Path::move_बायाँ(अचिन्हित Level) {
  प्रतिज्ञा(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  अचिन्हित l = 0;
  यदि (valid()) {
    l = Level - 1;
    यावत् (path[l].offset == 0) {
      प्रतिज्ञा(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } अन्यथा यदि (height() < Level)
    // end() may have created a height=0 path.
    path.resize(Level + 1, Entry(nullptr, 0, 0));

  // NR is the subtree containing our left sibling.
  --path[l].offset;
  गाँठ_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  प्रति (++l; l != Level; ++l) {
    path[l] = Entry(NR, NR.size() - 1);
    NR = NR.subtree(NR.size() - 1);
  }
  path[l] = Entry(NR, NR.size() - 1);
}

गाँठ_Ref Path::get_दायाँ_Sibling(अचिन्हित Level) ध्रुव {
  // The root has no siblings.
  यदि (Level == 0)
    प्रत्यागम गाँठ_Ref();

  // Go up the tree until we can go right.
  अचिन्हित l = Level - 1;
  यावत् (l && atLastEntry(l))
    --l;

  // We can't go right.
  यदि (atLastEntry(l))
    प्रत्यागम गाँठ_Ref();

  // NR is the subtree containing our right sibling.
  गाँठ_Ref NR = path[l].subtree(path[l].offset + 1);

  // Keep left all the way down.
  प्रति (++l; l != Level; ++l)
    NR = NR.subtree(0);
  प्रत्यागम NR;
}

शून्य Path::move_दायाँ(अचिन्हित Level) {
  प्रतिज्ञा(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  अचिन्हित l = Level - 1;
  यावत् (l && atLastEntry(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  यदि (++path[l].offset == path[l].size)
    प्रत्यागम;
  गाँठ_Ref NR = subtree(l);

  प्रति (++l; l != Level; ++l) {
    path[l] = Entry(NR, 0);
    NR = NR.subtree(0);
  }
  path[l] = Entry(NR, 0);
}


IdxPair distribute(अचिन्हित Nodes, अचिन्हित Elements, अचिन्हित Capacity,
                   ध्रुव अचिन्हित *CurSize, अचिन्हित NewSize[],
                   अचिन्हित Position, तार्किक Grow) {
  प्रतिज्ञा(Elements + Grow <= Nodes * Capacity && "Not enough room for elements");
  प्रतिज्ञा(Position <= Elements && "Invalid position");
  यदि (!Nodes)
    प्रत्यागम IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  ध्रुव अचिन्हित Per_गाँठ = (Elements + Grow) / Nodes;
  ध्रुव अचिन्हित Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  अचिन्हित Sum = 0;
  प्रति (अचिन्हित n = 0; n != Nodes; ++n) {
    Sum += NewSize[n] = Per_गाँठ + (n < Extra);
    यदि (PosPair.first == Nodes && Sum > Position)
      PosPair = IdxPair(n, Position - (Sum - NewSize[n]));
  }
  प्रतिज्ञा(Sum == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  यदि (Grow) {
    प्रतिज्ञा(PosPair.first < Nodes && "Bad algebra");
    प्रतिज्ञा(NewSize[PosPair.first] && "Too few elements to need Grow");
    --NewSize[PosPair.first];
  }

#ifndef न_संशोधन
  Sum = 0;
  प्रति (अचिन्हित n = 0; n != Nodes; ++n) {
    प्रतिज्ञा(NewSize[n] <= Capacity && "Overallocated node");
    Sum += NewSize[n];
  }
  प्रतिज्ञा(Sum == Elements && "Bad distribution sum");
#endif

  प्रत्यागम PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

