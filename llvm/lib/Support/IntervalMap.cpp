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

#समावेश "llvm/ADT/IntervalMap.h"
#समावेश <cassert>

namespace llvm {
namespace IntervalMapImpl {

व्योम Path::replace_मूल(व्योम *मूल_2, अचिन्हित आकार, IdxPair Offsets) {
  निश्चित(!path.empty() && "Can't replace missing root");
  path.front() = Entry(मूल_2, आकार, Offsets.first);
  path.जोड़ो(path.begin() + 1, Entry(subtree(0), Offsets.second));
}

गाँठ_Ref Path::get_बायाँ_Sibling(अचिन्हित Level) स्थिर {
  // The root has no siblings.
  अगर (Level == 0)
    वापस गाँठ_Ref();

  // Go up the tree until we can go left.
  अचिन्हित l = Level - 1;
  जबतक (l && path[l].offset == 0)
    --l;

  // We can't go left.
  अगर (path[l].offset == 0)
    वापस गाँठ_Ref();

  // NR is the subtree containing our left sibling.
  गाँठ_Ref NR = path[l].subtree(path[l].offset - 1);

  // Keep right all the way down.
  क्रम (++l; l != Level; ++l)
    NR = NR.subtree(NR.आकार_2() - 1);
  वापस NR;
}

व्योम Path::move_बायाँ(अचिन्हित Level) {
  निश्चित(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  अचिन्हित l = 0;
  अगर (valid()) {
    l = Level - 1;
    जबतक (path[l].offset == 0) {
      निश्चित(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } अन्यथा अगर (height() < Level)
    // end() may have created a height=0 path.
    path.resize(Level + 1, Entry(nullptr, 0, 0));

  // NR is the subtree containing our left sibling.
  --path[l].offset;
  गाँठ_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  क्रम (++l; l != Level; ++l) {
    path[l] = Entry(NR, NR.आकार_2() - 1);
    NR = NR.subtree(NR.आकार_2() - 1);
  }
  path[l] = Entry(NR, NR.आकार_2() - 1);
}

गाँठ_Ref Path::get_दायाँ_Sibling(अचिन्हित Level) स्थिर {
  // The root has no siblings.
  अगर (Level == 0)
    वापस गाँठ_Ref();

  // Go up the tree until we can go right.
  अचिन्हित l = Level - 1;
  जबतक (l && atLastEntry(l))
    --l;

  // We can't go right.
  अगर (atLastEntry(l))
    वापस गाँठ_Ref();

  // NR is the subtree containing our right sibling.
  गाँठ_Ref NR = path[l].subtree(path[l].offset + 1);

  // Keep left all the way down.
  क्रम (++l; l != Level; ++l)
    NR = NR.subtree(0);
  वापस NR;
}

व्योम Path::move_दायाँ(अचिन्हित Level) {
  निश्चित(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  अचिन्हित l = Level - 1;
  जबतक (l && atLastEntry(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  अगर (++path[l].offset == path[l].आकार_2)
    वापस;
  गाँठ_Ref NR = subtree(l);

  क्रम (++l; l != Level; ++l) {
    path[l] = Entry(NR, 0);
    NR = NR.subtree(0);
  }
  path[l] = Entry(NR, 0);
}


IdxPair distribute(अचिन्हित Nodes, अचिन्हित Elements, अचिन्हित Capacity,
                   स्थिर अचिन्हित *Cur_आकार, अचिन्हित New_आकार[],
                   अचिन्हित Position, bool Grow) {
  निश्चित(Elements + Grow <= Nodes * Capacity && "Not enough room for elements");
  निश्चित(Position <= Elements && "Invalid position");
  अगर (!Nodes)
    वापस IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  स्थिर अचिन्हित Per_गाँठ = (Elements + Grow) / Nodes;
  स्थिर अचिन्हित Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  अचिन्हित योग = 0;
  क्रम (अचिन्हित n = 0; n != Nodes; ++n) {
    योग += New_आकार[n] = Per_गाँठ + (n < Extra);
    अगर (PosPair.first == Nodes && योग > Position)
      PosPair = IdxPair(n, Position - (योग - New_आकार[n]));
  }
  निश्चित(योग == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  अगर (Grow) {
    निश्चित(PosPair.first < Nodes && "Bad algebra");
    निश्चित(New_आकार[PosPair.first] && "Too few elements to need Grow");
    --New_आकार[PosPair.first];
  }

#अगर_अघोषित न_संशोधन
  योग = 0;
  क्रम (अचिन्हित n = 0; n != Nodes; ++n) {
    निश्चित(New_आकार[n] <= Capacity && "Overallocated node");
    योग += New_आकार[n];
  }
  निश्चित(योग == Elements && "Bad distribution sum");
#पूर्ण_अगर

  वापस PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

