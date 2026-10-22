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

فراغ مسیر::replace_ریشه(فراغ *ریشه_2, بیعلامت اندازه, IdxPair Offsets) {
  أكد(!مسیر_2.خالی() && "Can't replace missing root");
  مسیر_2.front() = مدخل(ریشه_2, اندازه, Offsets.اولی_2);
  مسیر_2.درج(مسیر_2.begin() + 1, مدخل(subtree(0), Offsets.دومی_2));
}

گره_Ref مسیر::get_چپ_Sibling(بیعلامت Level) ثابت {
  // The root has no siblings.
  إذا (Level == 0)
    أرجع گره_Ref();

  // Go up the tree until we can go left.
  بیعلامت l = Level - 1;
  طالما (l && مسیر_2[l].جابه_جایی_2 == 0)
    --l;

  // We can't go left.
  إذا (مسیر_2[l].جابه_جایی_2 == 0)
    أرجع گره_Ref();

  // NR is the subtree containing our left sibling.
  گره_Ref NR = مسیر_2[l].subtree(مسیر_2[l].جابه_جایی_2 - 1);

  // Keep right all the way down.
  لكل (++l; l != Level; ++l)
    NR = NR.subtree(NR.اندازه_2() - 1);
  أرجع NR;
}

فراغ مسیر::move_چپ(بیعلامت Level) {
  أكد(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  بیعلامت l = 0;
  إذا (valid()) {
    l = Level - 1;
    طالما (مسیر_2[l].جابه_جایی_2 == 0) {
      أكد(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } وإلا إذا (ارتفاع() < Level)
    // end() may have created a height=0 path.
    مسیر_2.resize(Level + 1, مدخل(اشاره_تهی, 0, 0));

  // NR is the subtree containing our left sibling.
  --مسیر_2[l].جابه_جایی_2;
  گره_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  لكل (++l; l != Level; ++l) {
    مسیر_2[l] = مدخل(NR, NR.اندازه_2() - 1);
    NR = NR.subtree(NR.اندازه_2() - 1);
  }
  مسیر_2[l] = مدخل(NR, NR.اندازه_2() - 1);
}

گره_Ref مسیر::get_راست_Sibling(بیعلامت Level) ثابت {
  // The root has no siblings.
  إذا (Level == 0)
    أرجع گره_Ref();

  // Go up the tree until we can go right.
  بیعلامت l = Level - 1;
  طالما (l && at_آخرین_مدخل(l))
    --l;

  // We can't go right.
  إذا (at_آخرین_مدخل(l))
    أرجع گره_Ref();

  // NR is the subtree containing our right sibling.
  گره_Ref NR = مسیر_2[l].subtree(مسیر_2[l].جابه_جایی_2 + 1);

  // Keep left all the way down.
  لكل (++l; l != Level; ++l)
    NR = NR.subtree(0);
  أرجع NR;
}

فراغ مسیر::move_راست(بیعلامت Level) {
  أكد(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  بیعلامت l = Level - 1;
  طالما (l && at_آخرین_مدخل(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  إذا (++مسیر_2[l].جابه_جایی_2 == مسیر_2[l].اندازه_2)
    أرجع;
  گره_Ref NR = subtree(l);

  لكل (++l; l != Level; ++l) {
    مسیر_2[l] = مدخل(NR, 0);
    NR = NR.subtree(0);
  }
  مسیر_2[l] = مدخل(NR, 0);
}


IdxPair distribute(بیعلامت Nodes, بیعلامت Elements, بیعلامت ظرفیت,
                   ثابت بیعلامت *Cur_اندازه, بیعلامت New_اندازه[],
                   بیعلامت Position, منطقي Grow) {
  أكد(Elements + Grow <= Nodes * ظرفیت && "Not enough room for elements");
  أكد(Position <= Elements && "Invalid position");
  إذا (!Nodes)
    أرجع IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  ثابت بیعلامت در_هر_گره = (Elements + Grow) / Nodes;
  ثابت بیعلامت Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  بیعلامت جمع = 0;
  لكل (بیعلامت n = 0; n != Nodes; ++n) {
    جمع += New_اندازه[n] = در_هر_گره + (n < Extra);
    إذا (PosPair.اولی_2 == Nodes && جمع > Position)
      PosPair = IdxPair(n, Position - (جمع - New_اندازه[n]));
  }
  أكد(جمع == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  إذا (Grow) {
    أكد(PosPair.اولی_2 < Nodes && "Bad algebra");
    أكد(New_اندازه[PosPair.اولی_2] && "Too few elements to need Grow");
    --New_اندازه[PosPair.اولی_2];
  }

#اگر_تعریف_نشده NDEBUG
  جمع = 0;
  لكل (بیعلامت n = 0; n != Nodes; ++n) {
    أكد(New_اندازه[n] <= ظرفیت && "Overallocated node");
    جمع += New_اندازه[n];
  }
  أكد(جمع == Elements && "Bad distribution sum");
#پایان_اگر

  أرجع PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

