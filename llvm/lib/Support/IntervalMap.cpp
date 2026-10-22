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

#ضمن "llvm/ADT/IntervalMap.h"
#ضمن <cassert>

نطاق llvm {
نطاق IntervalMapImpl {

فراغ رستہ::replace_جڑ(فراغ *جڑ_2, بنانشانی ناپ, IdxPair Offsets) {
  أكد(!رستہ_2.خالی_تھاواں() && "Can't replace missing root");
  رستہ_2.front() = اندراج(جڑ_2, ناپ, Offsets.پہلا_2);
  رستہ_2.داخل_کرو(رستہ_2.begin() + 1, اندراج(subtree(0), Offsets.دوجا_2));
}

گرہ_Ref رستہ::get_بایاں_Sibling(بنانشانی Level) ثابت {
  // The root has no siblings.
  إذا (Level == 0)
    أرجع گرہ_Ref();

  // Go up the tree until we can go left.
  بنانشانی l = Level - 1;
  طالما (l && رستہ_2[l].وتھ_2 == 0)
    --l;

  // We can't go left.
  إذا (رستہ_2[l].وتھ_2 == 0)
    أرجع گرہ_Ref();

  // NR is the subtree containing our left sibling.
  گرہ_Ref NR = رستہ_2[l].subtree(رستہ_2[l].وتھ_2 - 1);

  // Keep right all the way down.
  لكل (++l; l != Level; ++l)
    NR = NR.subtree(NR.ناپ_2() - 1);
  أرجع NR;
}

فراغ رستہ::move_بایاں(بنانشانی Level) {
  أكد(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  بنانشانی l = 0;
  إذا (valid()) {
    l = Level - 1;
    طالما (رستہ_2[l].وتھ_2 == 0) {
      أكد(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } وإلا إذا (اچائی() < Level)
    // end() may have created a height=0 path.
    رستہ_2.resize(Level + 1, اندراج(خالی_سوچک, 0, 0));

  // NR is the subtree containing our left sibling.
  --رستہ_2[l].وتھ_2;
  گرہ_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  لكل (++l; l != Level; ++l) {
    رستہ_2[l] = اندراج(NR, NR.ناپ_2() - 1);
    NR = NR.subtree(NR.ناپ_2() - 1);
  }
  رستہ_2[l] = اندراج(NR, NR.ناپ_2() - 1);
}

گرہ_Ref رستہ::get_دایاں_Sibling(بنانشانی Level) ثابت {
  // The root has no siblings.
  إذا (Level == 0)
    أرجع گرہ_Ref();

  // Go up the tree until we can go right.
  بنانشانی l = Level - 1;
  طالما (l && at_آخری_اندراج(l))
    --l;

  // We can't go right.
  إذا (at_آخری_اندراج(l))
    أرجع گرہ_Ref();

  // NR is the subtree containing our right sibling.
  گرہ_Ref NR = رستہ_2[l].subtree(رستہ_2[l].وتھ_2 + 1);

  // Keep left all the way down.
  لكل (++l; l != Level; ++l)
    NR = NR.subtree(0);
  أرجع NR;
}

فراغ رستہ::move_دایاں(بنانشانی Level) {
  أكد(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  بنانشانی l = Level - 1;
  طالما (l && at_آخری_اندراج(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  إذا (++رستہ_2[l].وتھ_2 == رستہ_2[l].ناپ_2)
    أرجع;
  گرہ_Ref NR = subtree(l);

  لكل (++l; l != Level; ++l) {
    رستہ_2[l] = اندراج(NR, 0);
    NR = NR.subtree(0);
  }
  رستہ_2[l] = اندراج(NR, 0);
}


IdxPair distribute(بنانشانی Nodes, بنانشانی Elements, بنانشانی سمرتھا,
                   ثابت بنانشانی *Cur_ناپ, بنانشانی New_ناپ[],
                   بنانشانی Position, منطقي Grow) {
  أكد(Elements + Grow <= Nodes * سمرتھا && "Not enough room for elements");
  أكد(Position <= Elements && "Invalid position");
  إذا (!Nodes)
    أرجع IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  ثابت بنانشانی فی_گرہ = (Elements + Grow) / Nodes;
  ثابت بنانشانی Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  بنانشانی جوڑ = 0;
  لكل (بنانشانی n = 0; n != Nodes; ++n) {
    جوڑ += New_ناپ[n] = فی_گرہ + (n < Extra);
    إذا (PosPair.پہلا_2 == Nodes && جوڑ > Position)
      PosPair = IdxPair(n, Position - (جوڑ - New_ناپ[n]));
  }
  أكد(جوڑ == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  إذا (Grow) {
    أكد(PosPair.پہلا_2 < Nodes && "Bad algebra");
    أكد(New_ناپ[PosPair.پہلا_2] && "Too few elements to need Grow");
    --New_ناپ[PosPair.پہلا_2];
  }

#جے_بنا_تعریف NDEBUG
  جوڑ = 0;
  لكل (بنانشانی n = 0; n != Nodes; ++n) {
    أكد(New_ناپ[n] <= سمرتھا && "Overallocated node");
    جوڑ += New_ناپ[n];
  }
  أكد(جوڑ == Elements && "Bad distribution sum");
#جے_انت

  أرجع PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

