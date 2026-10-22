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

#అన్తర్భావ "llvm/ADT/IntervalMap.h"
#అన్తర్భావ <cassert>

నామాకాశ llvm {
నామాకాశ IntervalMapImpl {

శూన్య మార్గం::replace_మూలం(శూన్య *మూలం_2, గుర్తులేని కొలత, IdxPair Offsets) {
  ప్రతిజ్ఞా(!మార్గం_2.ఖాళీలు() && "Can't replace missing root");
  మార్గం_2.front() = నమోదు_2(మూలం_2, కొలత, Offsets.మొదటి_2);
  మార్గం_2.చొప్పించు(మార్గం_2.begin() + 1, నమోదు_2(subtree(0), Offsets.రెండవ_2));
}

కణుపు_Ref మార్గం::get_ఎడమ_Sibling(గుర్తులేని Level) స్థిరరాశి {
  // The root has no siblings.
  ఒకవేళ (Level == 0)
    తిరిగి కణుపు_Ref();

  // Go up the tree until we can go left.
  గుర్తులేని l = Level - 1;
  అయితే (l && మార్గం_2[l].స్థానభ్రంశం_2 == 0)
    --l;

  // We can't go left.
  ఒకవేళ (మార్గం_2[l].స్థానభ్రంశం_2 == 0)
    తిరిగి కణుపు_Ref();

  // NR is the subtree containing our left sibling.
  కణుపు_Ref NR = మార్గం_2[l].subtree(మార్గం_2[l].స్థానభ్రంశం_2 - 1);

  // Keep right all the way down.
  కొరకు (++l; l != Level; ++l)
    NR = NR.subtree(NR.కొలత_2() - 1);
  తిరిగి NR;
}

శూన్య మార్గం::move_ఎడమ(గుర్తులేని Level) {
  ప్రతిజ్ఞా(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  గుర్తులేని l = 0;
  ఒకవేళ (valid()) {
    l = Level - 1;
    అయితే (మార్గం_2[l].స్థానభ్రంశం_2 == 0) {
      ప్రతిజ్ఞా(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } లేకపోతే ఒకవేళ (ఎత్తు() < Level)
    // end() may have created a height=0 path.
    మార్గం_2.resize(Level + 1, నమోదు_2(శూన్యసూచిక, 0, 0));

  // NR is the subtree containing our left sibling.
  --మార్గం_2[l].స్థానభ్రంశం_2;
  కణుపు_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  కొరకు (++l; l != Level; ++l) {
    మార్గం_2[l] = నమోదు_2(NR, NR.కొలత_2() - 1);
    NR = NR.subtree(NR.కొలత_2() - 1);
  }
  మార్గం_2[l] = నమోదు_2(NR, NR.కొలత_2() - 1);
}

కణుపు_Ref మార్గం::get_కుడి_Sibling(గుర్తులేని Level) స్థిరరాశి {
  // The root has no siblings.
  ఒకవేళ (Level == 0)
    తిరిగి కణుపు_Ref();

  // Go up the tree until we can go right.
  గుర్తులేని l = Level - 1;
  అయితే (l && at_చివరి_నమోదు(l))
    --l;

  // We can't go right.
  ఒకవేళ (at_చివరి_నమోదు(l))
    తిరిగి కణుపు_Ref();

  // NR is the subtree containing our right sibling.
  కణుపు_Ref NR = మార్గం_2[l].subtree(మార్గం_2[l].స్థానభ్రంశం_2 + 1);

  // Keep left all the way down.
  కొరకు (++l; l != Level; ++l)
    NR = NR.subtree(0);
  తిరిగి NR;
}

శూన్య మార్గం::move_కుడి(గుర్తులేని Level) {
  ప్రతిజ్ఞా(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  గుర్తులేని l = Level - 1;
  అయితే (l && at_చివరి_నమోదు(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  ఒకవేళ (++మార్గం_2[l].స్థానభ్రంశం_2 == మార్గం_2[l].కొలత_2)
    తిరిగి;
  కణుపు_Ref NR = subtree(l);

  కొరకు (++l; l != Level; ++l) {
    మార్గం_2[l] = నమోదు_2(NR, 0);
    NR = NR.subtree(0);
  }
  మార్గం_2[l] = నమోదు_2(NR, 0);
}


IdxPair distribute(గుర్తులేని Nodes, గుర్తులేని Elements, గుర్తులేని సామర్థ్యం,
                   స్థిరరాశి గుర్తులేని *Cur_కొలత, గుర్తులేని New_కొలత[],
                   గుర్తులేని Position, తార్కిక Grow) {
  ప్రతిజ్ఞా(Elements + Grow <= Nodes * సామర్థ్యం && "Not enough room for elements");
  ప్రతిజ్ఞా(Position <= Elements && "Invalid position");
  ఒకవేళ (!Nodes)
    తిరిగి IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  స్థిరరాశి గుర్తులేని చొప్పున_కణుపు = (Elements + Grow) / Nodes;
  స్థిరరాశి గుర్తులేని Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  గుర్తులేని కూడిక = 0;
  కొరకు (గుర్తులేని n = 0; n != Nodes; ++n) {
    కూడిక += New_కొలత[n] = చొప్పున_కణుపు + (n < Extra);
    ఒకవేళ (PosPair.మొదటి_2 == Nodes && కూడిక > Position)
      PosPair = IdxPair(n, Position - (కూడిక - New_కొలత[n]));
  }
  ప్రతిజ్ఞా(కూడిక == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  ఒకవేళ (Grow) {
    ప్రతిజ్ఞా(PosPair.మొదటి_2 < Nodes && "Bad algebra");
    ప్రతిజ్ఞా(New_కొలత[PosPair.మొదటి_2] && "Too few elements to need Grow");
    --New_కొలత[PosPair.మొదటి_2];
  }

#నిర్వచితం_కాకపోతే NDEBUG
  కూడిక = 0;
  కొరకు (గుర్తులేని n = 0; n != Nodes; ++n) {
    ప్రతిజ్ఞా(New_కొలత[n] <= సామర్థ్యం && "Overallocated node");
    కూడిక += New_కొలత[n];
  }
  ప్రతిజ్ఞా(కూడిక == Elements && "Bad distribution sum");
#అయితే_ముగింపు

  తిరిగి PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

