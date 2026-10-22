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

فراغ رستو::replace_پاڙ(فراغ *پاڙ_2, بنانشان سائيز, IdxPair Offsets) {
  أكد(!رستو_2.خالي_جايون() && "Can't replace missing root");
  رستو_2.front() = داخلا_3(پاڙ_2, سائيز, Offsets.پهريون_2);
  رستو_2.وجهو(رستو_2.begin() + 1, داخلا_3(subtree(0), Offsets.ٻيو_2));
}

ڳنڍ_Ref رستو::get_کاٻو_Sibling(بنانشان Level) ثابت {
  // The root has no siblings.
  إذا (Level == 0)
    أرجع ڳنڍ_Ref();

  // Go up the tree until we can go left.
  بنانشان l = Level - 1;
  طالما (l && رستو_2[l].وٿي_2 == 0)
    --l;

  // We can't go left.
  إذا (رستو_2[l].وٿي_2 == 0)
    أرجع ڳنڍ_Ref();

  // NR is the subtree containing our left sibling.
  ڳنڍ_Ref NR = رستو_2[l].subtree(رستو_2[l].وٿي_2 - 1);

  // Keep right all the way down.
  لكل (++l; l != Level; ++l)
    NR = NR.subtree(NR.سائيز_2() - 1);
  أرجع NR;
}

فراغ رستو::move_کاٻو(بنانشان Level) {
  أكد(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  بنانشان l = 0;
  إذا (valid()) {
    l = Level - 1;
    طالما (رستو_2[l].وٿي_2 == 0) {
      أكد(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } وإلا إذا (اوچائي() < Level)
    // end() may have created a height=0 path.
    رستو_2.resize(Level + 1, داخلا_3(خالي_اشارو, 0, 0));

  // NR is the subtree containing our left sibling.
  --رستو_2[l].وٿي_2;
  ڳنڍ_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  لكل (++l; l != Level; ++l) {
    رستو_2[l] = داخلا_3(NR, NR.سائيز_2() - 1);
    NR = NR.subtree(NR.سائيز_2() - 1);
  }
  رستو_2[l] = داخلا_3(NR, NR.سائيز_2() - 1);
}

ڳنڍ_Ref رستو::get_ساڄو_Sibling(بنانشان Level) ثابت {
  // The root has no siblings.
  إذا (Level == 0)
    أرجع ڳنڍ_Ref();

  // Go up the tree until we can go right.
  بنانشان l = Level - 1;
  طالما (l && at_آخري_داخلا(l))
    --l;

  // We can't go right.
  إذا (at_آخري_داخلا(l))
    أرجع ڳنڍ_Ref();

  // NR is the subtree containing our right sibling.
  ڳنڍ_Ref NR = رستو_2[l].subtree(رستو_2[l].وٿي_2 + 1);

  // Keep left all the way down.
  لكل (++l; l != Level; ++l)
    NR = NR.subtree(0);
  أرجع NR;
}

فراغ رستو::move_ساڄو(بنانشان Level) {
  أكد(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  بنانشان l = Level - 1;
  طالما (l && at_آخري_داخلا(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  إذا (++رستو_2[l].وٿي_2 == رستو_2[l].سائيز_2)
    أرجع;
  ڳنڍ_Ref NR = subtree(l);

  لكل (++l; l != Level; ++l) {
    رستو_2[l] = داخلا_3(NR, 0);
    NR = NR.subtree(0);
  }
  رستو_2[l] = داخلا_3(NR, 0);
}


IdxPair distribute(بنانشان Nodes, بنانشان Elements, بنانشان صلاحيت,
                   ثابت بنانشان *Cur_سائيز, بنانشان New_سائيز[],
                   بنانشان Position, منطقي Grow) {
  أكد(Elements + Grow <= Nodes * صلاحيت && "Not enough room for elements");
  أكد(Position <= Elements && "Invalid position");
  إذا (!Nodes)
    أرجع IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  ثابت بنانشان في_ڳنڍ = (Elements + Grow) / Nodes;
  ثابت بنانشان Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  بنانشان جوڙ = 0;
  لكل (بنانشان n = 0; n != Nodes; ++n) {
    جوڙ += New_سائيز[n] = في_ڳنڍ + (n < Extra);
    إذا (PosPair.پهريون_2 == Nodes && جوڙ > Position)
      PosPair = IdxPair(n, Position - (جوڙ - New_سائيز[n]));
  }
  أكد(جوڙ == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  إذا (Grow) {
    أكد(PosPair.پهريون_2 < Nodes && "Bad algebra");
    أكد(New_سائيز[PosPair.پهريون_2] && "Too few elements to need Grow");
    --New_سائيز[PosPair.پهريون_2];
  }

#جيڪڏهن_وصف_نه_ٿيل NDEBUG
  جوڙ = 0;
  لكل (بنانشان n = 0; n != Nodes; ++n) {
    أكد(New_سائيز[n] <= صلاحيت && "Overallocated node");
    جوڙ += New_سائيز[n];
  }
  أكد(جوڙ == Elements && "Bad distribution sum");
#جيڪڏهن_پورو

  أرجع PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

