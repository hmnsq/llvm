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

शून्य मार्गः::replace_मूलम्(शून्य *मूलम्_2, अचिह्नित आकार, IdxPair Offsets) {
  प्रतिज्ञा(!मार्गः_2.रिक्तानि() && "Can't replace missing root");
  मार्गः_2.front() = प्रविष्टिः(मूलम्_2, आकार, Offsets.first);
  मार्गः_2.निवेशय(मार्गः_2.begin() + 1, प्रविष्टिः(subtree(0), Offsets.second));
}

ग्रन्थिः_Ref मार्गः::get_वामः_Sibling(अचिह्नित Level) ध्रुव {
  // The root has no siblings.
  यदि (Level == 0)
    प्रत्यागम ग्रन्थिः_Ref();

  // Go up the tree until we can go left.
  अचिह्नित l = Level - 1;
  यावत् (l && मार्गः_2[l].विस्थापनम्_2 == 0)
    --l;

  // We can't go left.
  यदि (मार्गः_2[l].विस्थापनम्_2 == 0)
    प्रत्यागम ग्रन्थिः_Ref();

  // NR is the subtree containing our left sibling.
  ग्रन्थिः_Ref NR = मार्गः_2[l].subtree(मार्गः_2[l].विस्थापनम्_2 - 1);

  // Keep right all the way down.
  प्रति (++l; l != Level; ++l)
    NR = NR.subtree(NR.आकार_2() - 1);
  प्रत्यागम NR;
}

शून्य मार्गः::move_वामः(अचिह्नित Level) {
  प्रतिज्ञा(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  अचिह्नित l = 0;
  यदि (valid()) {
    l = Level - 1;
    यावत् (मार्गः_2[l].विस्थापनम्_2 == 0) {
      प्रतिज्ञा(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } अन्यथा यदि (height() < Level)
    // end() may have created a height=0 path.
    मार्गः_2.resize(Level + 1, प्रविष्टिः(शून्यसूचकः, 0, 0));

  // NR is the subtree containing our left sibling.
  --मार्गः_2[l].विस्थापनम्_2;
  ग्रन्थिः_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  प्रति (++l; l != Level; ++l) {
    मार्गः_2[l] = प्रविष्टिः(NR, NR.आकार_2() - 1);
    NR = NR.subtree(NR.आकार_2() - 1);
  }
  मार्गः_2[l] = प्रविष्टिः(NR, NR.आकार_2() - 1);
}

ग्रन्थिः_Ref मार्गः::get_दक्षिणः_Sibling(अचिह्नित Level) ध्रुव {
  // The root has no siblings.
  यदि (Level == 0)
    प्रत्यागम ग्रन्थिः_Ref();

  // Go up the tree until we can go right.
  अचिह्नित l = Level - 1;
  यावत् (l && at_चरमः_प्रविष्टिः(l))
    --l;

  // We can't go right.
  यदि (at_चरमः_प्रविष्टिः(l))
    प्रत्यागम ग्रन्थिः_Ref();

  // NR is the subtree containing our right sibling.
  ग्रन्थिः_Ref NR = मार्गः_2[l].subtree(मार्गः_2[l].विस्थापनम्_2 + 1);

  // Keep left all the way down.
  प्रति (++l; l != Level; ++l)
    NR = NR.subtree(0);
  प्रत्यागम NR;
}

शून्य मार्गः::move_दक्षिणः(अचिह्नित Level) {
  प्रतिज्ञा(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  अचिह्नित l = Level - 1;
  यावत् (l && at_चरमः_प्रविष्टिः(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  यदि (++मार्गः_2[l].विस्थापनम्_2 == मार्गः_2[l].आकार_2)
    प्रत्यागम;
  ग्रन्थिः_Ref NR = subtree(l);

  प्रति (++l; l != Level; ++l) {
    मार्गः_2[l] = प्रविष्टिः(NR, 0);
    NR = NR.subtree(0);
  }
  मार्गः_2[l] = प्रविष्टिः(NR, 0);
}


IdxPair distribute(अचिह्नित Nodes, अचिह्नित Elements, अचिह्नित क्षमता,
                   ध्रुव अचिह्नित *Cur_आकार, अचिह्नित New_आकार[],
                   अचिह्नित Position, तार्किक Grow) {
  प्रतिज्ञा(Elements + Grow <= Nodes * क्षमता && "Not enough room for elements");
  प्रतिज्ञा(Position <= Elements && "Invalid position");
  यदि (!Nodes)
    प्रत्यागम IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  ध्रुव अचिह्नित प्रत्येकम्_ग्रन्थिः = (Elements + Grow) / Nodes;
  ध्रुव अचिह्नित Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  अचिह्नित योग = 0;
  प्रति (अचिह्नित n = 0; n != Nodes; ++n) {
    योग += New_आकार[n] = प्रत्येकम्_ग्रन्थिः + (n < Extra);
    यदि (PosPair.first == Nodes && योग > Position)
      PosPair = IdxPair(n, Position - (योग - New_आकार[n]));
  }
  प्रतिज्ञा(योग == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  यदि (Grow) {
    प्रतिज्ञा(PosPair.first < Nodes && "Bad algebra");
    प्रतिज्ञा(New_आकार[PosPair.first] && "Too few elements to need Grow");
    --New_आकार[PosPair.first];
  }

#यदि_अपरिभाषितम् NDEBUG
  योग = 0;
  प्रति (अचिह्नित n = 0; n != Nodes; ++n) {
    प्रतिज्ञा(New_आकार[n] <= क्षमता && "Overallocated node");
    योग += New_आकार[n];
  }
  प्रतिज्ञा(योग == Elements && "Bad distribution sum");
#यदि_अन्तः

  प्रत्यागम PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

