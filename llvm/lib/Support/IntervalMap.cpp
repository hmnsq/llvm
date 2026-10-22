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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/IntervalMap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>

ਨਾਮਾਕਾਸ਼ llvm {
ਨਾਮਾਕਾਸ਼ IntervalMapImpl {

ਖਾਲੀ ਰਸਤਾ::replace_ਜੜ੍ਹ(ਖਾਲੀ *ਜੜ੍ਹ_2, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ, IdxPair Offsets) {
  ਪ੍ਰਤਿਜ੍ਞਾ(!ਰਸਤਾ_2.ਖਾਲੀ_ਥਾਵਾਂ() && "Can't replace missing root");
  ਰਸਤਾ_2.front() = ਇੰਦਰਾਜ(ਜੜ੍ਹ_2, ਮਾਪ, Offsets.ਪਹਿਲਾ_2);
  ਰਸਤਾ_2.ਪਾਓ(ਰਸਤਾ_2.begin() + 1, ਇੰਦਰਾਜ(subtree(0), Offsets.ਦੂਜਾ_2));
}

ਗੰਢ_Ref ਰਸਤਾ::get_ਖੱਬਾ_Sibling(ਬਿਨਾਨਿਸ਼ਾਨੀ Level) ਅਚਲ {
  // The root has no siblings.
  ਜੇ (Level == 0)
    ਵਾਪਸ ਗੰਢ_Ref();

  // Go up the tree until we can go left.
  ਬਿਨਾਨਿਸ਼ਾਨੀ l = Level - 1;
  ਜਦੋਂ (l && ਰਸਤਾ_2[l].ਵਿੱਥ_2 == 0)
    --l;

  // We can't go left.
  ਜੇ (ਰਸਤਾ_2[l].ਵਿੱਥ_2 == 0)
    ਵਾਪਸ ਗੰਢ_Ref();

  // NR is the subtree containing our left sibling.
  ਗੰਢ_Ref NR = ਰਸਤਾ_2[l].subtree(ਰਸਤਾ_2[l].ਵਿੱਥ_2 - 1);

  // Keep right all the way down.
  ਲਈ (++l; l != Level; ++l)
    NR = NR.subtree(NR.ਮਾਪ_2() - 1);
  ਵਾਪਸ NR;
}

ਖਾਲੀ ਰਸਤਾ::move_ਖੱਬਾ(ਬਿਨਾਨਿਸ਼ਾਨੀ Level) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  ਬਿਨਾਨਿਸ਼ਾਨੀ l = 0;
  ਜੇ (valid()) {
    l = Level - 1;
    ਜਦੋਂ (ਰਸਤਾ_2[l].ਵਿੱਥ_2 == 0) {
      ਪ੍ਰਤਿਜ੍ਞਾ(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } ਨਹੀਂਤਾਂ ਜੇ (ਉਚਾਈ() < Level)
    // end() may have created a height=0 path.
    ਰਸਤਾ_2.resize(Level + 1, ਇੰਦਰਾਜ(ਖਾਲੀ_ਸੂਚਕ, 0, 0));

  // NR is the subtree containing our left sibling.
  --ਰਸਤਾ_2[l].ਵਿੱਥ_2;
  ਗੰਢ_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  ਲਈ (++l; l != Level; ++l) {
    ਰਸਤਾ_2[l] = ਇੰਦਰਾਜ(NR, NR.ਮਾਪ_2() - 1);
    NR = NR.subtree(NR.ਮਾਪ_2() - 1);
  }
  ਰਸਤਾ_2[l] = ਇੰਦਰਾਜ(NR, NR.ਮਾਪ_2() - 1);
}

ਗੰਢ_Ref ਰਸਤਾ::get_ਸੱਜਾ_Sibling(ਬਿਨਾਨਿਸ਼ਾਨੀ Level) ਅਚਲ {
  // The root has no siblings.
  ਜੇ (Level == 0)
    ਵਾਪਸ ਗੰਢ_Ref();

  // Go up the tree until we can go right.
  ਬਿਨਾਨਿਸ਼ਾਨੀ l = Level - 1;
  ਜਦੋਂ (l && at_ਆਖਰੀ_ਇੰਦਰਾਜ(l))
    --l;

  // We can't go right.
  ਜੇ (at_ਆਖਰੀ_ਇੰਦਰਾਜ(l))
    ਵਾਪਸ ਗੰਢ_Ref();

  // NR is the subtree containing our right sibling.
  ਗੰਢ_Ref NR = ਰਸਤਾ_2[l].subtree(ਰਸਤਾ_2[l].ਵਿੱਥ_2 + 1);

  // Keep left all the way down.
  ਲਈ (++l; l != Level; ++l)
    NR = NR.subtree(0);
  ਵਾਪਸ NR;
}

ਖਾਲੀ ਰਸਤਾ::move_ਸੱਜਾ(ਬਿਨਾਨਿਸ਼ਾਨੀ Level) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  ਬਿਨਾਨਿਸ਼ਾਨੀ l = Level - 1;
  ਜਦੋਂ (l && at_ਆਖਰੀ_ਇੰਦਰਾਜ(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  ਜੇ (++ਰਸਤਾ_2[l].ਵਿੱਥ_2 == ਰਸਤਾ_2[l].ਮਾਪ_2)
    ਵਾਪਸ;
  ਗੰਢ_Ref NR = subtree(l);

  ਲਈ (++l; l != Level; ++l) {
    ਰਸਤਾ_2[l] = ਇੰਦਰਾਜ(NR, 0);
    NR = NR.subtree(0);
  }
  ਰਸਤਾ_2[l] = ਇੰਦਰਾਜ(NR, 0);
}


IdxPair distribute(ਬਿਨਾਨਿਸ਼ਾਨੀ Nodes, ਬਿਨਾਨਿਸ਼ਾਨੀ Elements, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਮਰੱਥਾ,
                   ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ *Cur_ਮਾਪ, ਬਿਨਾਨਿਸ਼ਾਨੀ New_ਮਾਪ[],
                   ਬਿਨਾਨਿਸ਼ਾਨੀ Position, ਤਾਰਕਿਕ Grow) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Elements + Grow <= Nodes * ਸਮਰੱਥਾ && "Not enough room for elements");
  ਪ੍ਰਤਿਜ੍ਞਾ(Position <= Elements && "Invalid position");
  ਜੇ (!Nodes)
    ਵਾਪਸ IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪ੍ਰਤੀ_ਗੰਢ = (Elements + Grow) / Nodes;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਜੋੜ = 0;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ n = 0; n != Nodes; ++n) {
    ਜੋੜ += New_ਮਾਪ[n] = ਪ੍ਰਤੀ_ਗੰਢ + (n < Extra);
    ਜੇ (PosPair.ਪਹਿਲਾ_2 == Nodes && ਜੋੜ > Position)
      PosPair = IdxPair(n, Position - (ਜੋੜ - New_ਮਾਪ[n]));
  }
  ਪ੍ਰਤਿਜ੍ਞਾ(ਜੋੜ == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  ਜੇ (Grow) {
    ਪ੍ਰਤਿਜ੍ਞਾ(PosPair.ਪਹਿਲਾ_2 < Nodes && "Bad algebra");
    ਪ੍ਰਤਿਜ੍ਞਾ(New_ਮਾਪ[PosPair.ਪਹਿਲਾ_2] && "Too few elements to need Grow");
    --New_ਮਾਪ[PosPair.ਪਹਿਲਾ_2];
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
  ਜੋੜ = 0;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ n = 0; n != Nodes; ++n) {
    ਪ੍ਰਤਿਜ੍ਞਾ(New_ਮਾਪ[n] <= ਸਮਰੱਥਾ && "Overallocated node");
    ਜੋੜ += New_ਮਾਪ[n];
  }
  ਪ੍ਰਤਿਜ੍ਞਾ(ਜੋੜ == Elements && "Bad distribution sum");
#ਜੇ_ਅੰਤ

  ਵਾਪਸ PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

