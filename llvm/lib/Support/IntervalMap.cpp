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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/IntervalMap.h"
#ಅನ್ತರ್ಭಾವ <cassert>

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ IntervalMapImpl {

ಶೂನ್ಯ ಮಾರ್ಗ::replace_ಬೇರು(ಶೂನ್ಯ *ಬೇರು_2, ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2, IdxPair Offsets) {
  ಪ್ರತಿಜ್ಞಾ(!ಮಾರ್ಗ_2.ಖಾಲಿಗಳು() && "Can't replace missing root");
  ಮಾರ್ಗ_2.front() = ನಮೂದು(ಬೇರು_2, ಗಾತ್ರ_2, Offsets.ಮೊದಲ_2);
  ಮಾರ್ಗ_2.ಸೇರಿಸು(ಮಾರ್ಗ_2.begin() + 1, ನಮೂದು(subtree(0), Offsets.ಎರಡನೇ_2));
}

ಗಂಟು_Ref ಮಾರ್ಗ::get_ಎಡ_Sibling(ಚಿಹ್ನೆರಹಿತ Level) ಸ್ಥಿರಾಂಕ {
  // The root has no siblings.
  ಆದರೆ (Level == 0)
    ಹಿಂತಿರುಗಿ ಗಂಟು_Ref();

  // Go up the tree until we can go left.
  ಚಿಹ್ನೆರಹಿತ l = Level - 1;
  ಆಗ (l && ಮಾರ್ಗ_2[l].ಸ್ಥಾನಾಂತರ_2 == 0)
    --l;

  // We can't go left.
  ಆದರೆ (ಮಾರ್ಗ_2[l].ಸ್ಥಾನಾಂತರ_2 == 0)
    ಹಿಂತಿರುಗಿ ಗಂಟು_Ref();

  // NR is the subtree containing our left sibling.
  ಗಂಟು_Ref NR = ಮಾರ್ಗ_2[l].subtree(ಮಾರ್ಗ_2[l].ಸ್ಥಾನಾಂತರ_2 - 1);

  // Keep right all the way down.
  ಗಾಗಿ (++l; l != Level; ++l)
    NR = NR.subtree(NR.ಗಾತ್ರ_3() - 1);
  ಹಿಂತಿರುಗಿ NR;
}

ಶೂನ್ಯ ಮಾರ್ಗ::move_ಎಡ(ಚಿಹ್ನೆರಹಿತ Level) {
  ಪ್ರತಿಜ್ಞಾ(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  ಚಿಹ್ನೆರಹಿತ l = 0;
  ಆದರೆ (valid()) {
    l = Level - 1;
    ಆಗ (ಮಾರ್ಗ_2[l].ಸ್ಥಾನಾಂತರ_2 == 0) {
      ಪ್ರತಿಜ್ಞಾ(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಎತ್ತರ() < Level)
    // end() may have created a height=0 path.
    ಮಾರ್ಗ_2.resize(Level + 1, ನಮೂದು(ಶೂನ್ಯಸೂಚಕ, 0, 0));

  // NR is the subtree containing our left sibling.
  --ಮಾರ್ಗ_2[l].ಸ್ಥಾನಾಂತರ_2;
  ಗಂಟು_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  ಗಾಗಿ (++l; l != Level; ++l) {
    ಮಾರ್ಗ_2[l] = ನಮೂದು(NR, NR.ಗಾತ್ರ_3() - 1);
    NR = NR.subtree(NR.ಗಾತ್ರ_3() - 1);
  }
  ಮಾರ್ಗ_2[l] = ನಮೂದು(NR, NR.ಗಾತ್ರ_3() - 1);
}

ಗಂಟು_Ref ಮಾರ್ಗ::get_ಬಲ_Sibling(ಚಿಹ್ನೆರಹಿತ Level) ಸ್ಥಿರಾಂಕ {
  // The root has no siblings.
  ಆದರೆ (Level == 0)
    ಹಿಂತಿರುಗಿ ಗಂಟು_Ref();

  // Go up the tree until we can go right.
  ಚಿಹ್ನೆರಹಿತ l = Level - 1;
  ಆಗ (l && at_ಕೊನೆಯ_ನಮೂದು(l))
    --l;

  // We can't go right.
  ಆದರೆ (at_ಕೊನೆಯ_ನಮೂದು(l))
    ಹಿಂತಿರುಗಿ ಗಂಟು_Ref();

  // NR is the subtree containing our right sibling.
  ಗಂಟು_Ref NR = ಮಾರ್ಗ_2[l].subtree(ಮಾರ್ಗ_2[l].ಸ್ಥಾನಾಂತರ_2 + 1);

  // Keep left all the way down.
  ಗಾಗಿ (++l; l != Level; ++l)
    NR = NR.subtree(0);
  ಹಿಂತಿರುಗಿ NR;
}

ಶೂನ್ಯ ಮಾರ್ಗ::move_ಬಲ(ಚಿಹ್ನೆರಹಿತ Level) {
  ಪ್ರತಿಜ್ಞಾ(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  ಚಿಹ್ನೆರಹಿತ l = Level - 1;
  ಆಗ (l && at_ಕೊನೆಯ_ನಮೂದು(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  ಆದರೆ (++ಮಾರ್ಗ_2[l].ಸ್ಥಾನಾಂತರ_2 == ಮಾರ್ಗ_2[l].ಗಾತ್ರ_3)
    ಹಿಂತಿರುಗಿ;
  ಗಂಟು_Ref NR = subtree(l);

  ಗಾಗಿ (++l; l != Level; ++l) {
    ಮಾರ್ಗ_2[l] = ನಮೂದು(NR, 0);
    NR = NR.subtree(0);
  }
  ಮಾರ್ಗ_2[l] = ನಮೂದು(NR, 0);
}


IdxPair distribute(ಚಿಹ್ನೆರಹಿತ Nodes, ಚಿಹ್ನೆರಹಿತ Elements, ಚಿಹ್ನೆರಹಿತ ಸಾಮರ್ಥ್ಯ,
                   ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ *Cur_ಗಾತ್ರ, ಚಿಹ್ನೆರಹಿತ New_ಗಾತ್ರ[],
                   ಚಿಹ್ನೆರಹಿತ Position, ತಾರ್ಕಿಕ Grow) {
  ಪ್ರತಿಜ್ಞಾ(Elements + Grow <= Nodes * ಸಾಮರ್ಥ್ಯ && "Not enough room for elements");
  ಪ್ರತಿಜ್ಞಾ(Position <= Elements && "Invalid position");
  ಆದರೆ (!Nodes)
    ಹಿಂತಿರುಗಿ IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಪ್ರತಿ_ಗಂಟು = (Elements + Grow) / Nodes;
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  ಚಿಹ್ನೆರಹಿತ ಮೊತ್ತ = 0;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ n = 0; n != Nodes; ++n) {
    ಮೊತ್ತ += New_ಗಾತ್ರ[n] = ಪ್ರತಿ_ಗಂಟು + (n < Extra);
    ಆದರೆ (PosPair.ಮೊದಲ_2 == Nodes && ಮೊತ್ತ > Position)
      PosPair = IdxPair(n, Position - (ಮೊತ್ತ - New_ಗಾತ್ರ[n]));
  }
  ಪ್ರತಿಜ್ಞಾ(ಮೊತ್ತ == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  ಆದರೆ (Grow) {
    ಪ್ರತಿಜ್ಞಾ(PosPair.ಮೊದಲ_2 < Nodes && "Bad algebra");
    ಪ್ರತಿಜ್ಞಾ(New_ಗಾತ್ರ[PosPair.ಮೊದಲ_2] && "Too few elements to need Grow");
    --New_ಗಾತ್ರ[PosPair.ಮೊದಲ_2];
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
  ಮೊತ್ತ = 0;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ n = 0; n != Nodes; ++n) {
    ಪ್ರತಿಜ್ಞಾ(New_ಗಾತ್ರ[n] <= ಸಾಮರ್ಥ್ಯ && "Overallocated node");
    ಮೊತ್ತ += New_ಗಾತ್ರ[n];
  }
  ಪ್ರತಿಜ್ಞಾ(ಮೊತ್ತ == Elements && "Bad distribution sum");
#ಆದರೆ_ಅಂತ್ಯ

  ಹಿಂತಿರುಗಿ PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

