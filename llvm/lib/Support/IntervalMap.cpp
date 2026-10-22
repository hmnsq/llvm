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

शून्य Path::replace_जरा(शून्य *जरा_2, अचिन्हित Size, IdxPair Offsets) {
  प्रतिज्ञा(!path.empty() && "Can't replace missing root");
  path.front() = Entry(जरा_2, Size, Offsets.first);
  path.घुसाउ(path.begin() + 1, Entry(subtree(0), Offsets.second));
}

गाँठो_Ref Path::get_बायाँ_Sibling(अचिन्हित Level) अचर {
  // The root has no siblings.
  यदि (Level == 0)
    फर्काऊ गाँठो_Ref();

  // Go up the tree until we can go left.
  अचिन्हित l = Level - 1;
  जबसम्म (l && path[l].offset == 0)
    --l;

  // We can't go left.
  यदि (path[l].offset == 0)
    फर्काऊ गाँठो_Ref();

  // NR is the subtree containing our left sibling.
  गाँठो_Ref NR = path[l].subtree(path[l].offset - 1);

  // Keep right all the way down.
  लागि (++l; l != Level; ++l)
    NR = NR.subtree(NR.size() - 1);
  फर्काऊ NR;
}

शून्य Path::move_बायाँ(अचिन्हित Level) {
  प्रतिज्ञा(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  अचिन्हित l = 0;
  यदि (valid()) {
    l = Level - 1;
    जबसम्म (path[l].offset == 0) {
      प्रतिज्ञा(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } अन्यथा यदि (height() < Level)
    // end() may have created a height=0 path.
    path.resize(Level + 1, Entry(nullptr, 0, 0));

  // NR is the subtree containing our left sibling.
  --path[l].offset;
  गाँठो_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  लागि (++l; l != Level; ++l) {
    path[l] = Entry(NR, NR.size() - 1);
    NR = NR.subtree(NR.size() - 1);
  }
  path[l] = Entry(NR, NR.size() - 1);
}

गाँठो_Ref Path::get_दायाँ_Sibling(अचिन्हित Level) अचर {
  // The root has no siblings.
  यदि (Level == 0)
    फर्काऊ गाँठो_Ref();

  // Go up the tree until we can go right.
  अचिन्हित l = Level - 1;
  जबसम्म (l && atLastEntry(l))
    --l;

  // We can't go right.
  यदि (atLastEntry(l))
    फर्काऊ गाँठो_Ref();

  // NR is the subtree containing our right sibling.
  गाँठो_Ref NR = path[l].subtree(path[l].offset + 1);

  // Keep left all the way down.
  लागि (++l; l != Level; ++l)
    NR = NR.subtree(0);
  फर्काऊ NR;
}

शून्य Path::move_दायाँ(अचिन्हित Level) {
  प्रतिज्ञा(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  अचिन्हित l = Level - 1;
  जबसम्म (l && atLastEntry(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  यदि (++path[l].offset == path[l].size)
    फर्काऊ;
  गाँठो_Ref NR = subtree(l);

  लागि (++l; l != Level; ++l) {
    path[l] = Entry(NR, 0);
    NR = NR.subtree(0);
  }
  path[l] = Entry(NR, 0);
}


IdxPair distribute(अचिन्हित Nodes, अचिन्हित Elements, अचिन्हित Capacity,
                   अचर अचिन्हित *CurSize, अचिन्हित NewSize[],
                   अचिन्हित Position, तार्किक Grow) {
  प्रतिज्ञा(Elements + Grow <= Nodes * Capacity && "Not enough room for elements");
  प्रतिज्ञा(Position <= Elements && "Invalid position");
  यदि (!Nodes)
    फर्काऊ IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  अचर अचिन्हित Per_गाँठो = (Elements + Grow) / Nodes;
  अचर अचिन्हित Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  अचिन्हित Sum = 0;
  लागि (अचिन्हित n = 0; n != Nodes; ++n) {
    Sum += NewSize[n] = Per_गाँठो + (n < Extra);
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
  लागि (अचिन्हित n = 0; n != Nodes; ++n) {
    प्रतिज्ञा(NewSize[n] <= Capacity && "Overallocated node");
    Sum += NewSize[n];
  }
  प्रतिज्ञा(Sum == Elements && "Bad distribution sum");
#endif

  फर्काऊ PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

