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

فراغ لار::replace_ریښه(فراغ *ریښه_2, بینښه اندازه, IdxPair Offsets) {
  أكد(!لار_2.تشه() && "Can't replace missing root");
  لار_2.front() = ثبت(ریښه_2, اندازه, Offsets.لومړی_2);
  لار_2.ورننباسه(لار_2.begin() + 1, ثبت(subtree(0), Offsets.دوهم_2));
}

غوټه_Ref لار::get_کیڼ_Sibling(بینښه Level) ثابت {
  // The root has no siblings.
  إذا (Level == 0)
    أرجع غوټه_Ref();

  // Go up the tree until we can go left.
  بینښه l = Level - 1;
  طالما (l && لار_2[l].دننه_واټن_2 == 0)
    --l;

  // We can't go left.
  إذا (لار_2[l].دننه_واټن_2 == 0)
    أرجع غوټه_Ref();

  // NR is the subtree containing our left sibling.
  غوټه_Ref NR = لار_2[l].subtree(لار_2[l].دننه_واټن_2 - 1);

  // Keep right all the way down.
  لكل (++l; l != Level; ++l)
    NR = NR.subtree(NR.اندازه_2() - 1);
  أرجع NR;
}

فراغ لار::move_کیڼ(بینښه Level) {
  أكد(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  بینښه l = 0;
  إذا (valid()) {
    l = Level - 1;
    طالما (لار_2[l].دننه_واټن_2 == 0) {
      أكد(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } وإلا إذا (لوړوالی() < Level)
    // end() may have created a height=0 path.
    لار_2.resize(Level + 1, ثبت(تش_اشاره, 0, 0));

  // NR is the subtree containing our left sibling.
  --لار_2[l].دننه_واټن_2;
  غوټه_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  لكل (++l; l != Level; ++l) {
    لار_2[l] = ثبت(NR, NR.اندازه_2() - 1);
    NR = NR.subtree(NR.اندازه_2() - 1);
  }
  لار_2[l] = ثبت(NR, NR.اندازه_2() - 1);
}

غوټه_Ref لار::get_ښی_Sibling(بینښه Level) ثابت {
  // The root has no siblings.
  إذا (Level == 0)
    أرجع غوټه_Ref();

  // Go up the tree until we can go right.
  بینښه l = Level - 1;
  طالما (l && at_وروستی_ثبت(l))
    --l;

  // We can't go right.
  إذا (at_وروستی_ثبت(l))
    أرجع غوټه_Ref();

  // NR is the subtree containing our right sibling.
  غوټه_Ref NR = لار_2[l].subtree(لار_2[l].دننه_واټن_2 + 1);

  // Keep left all the way down.
  لكل (++l; l != Level; ++l)
    NR = NR.subtree(0);
  أرجع NR;
}

فراغ لار::move_ښی(بینښه Level) {
  أكد(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  بینښه l = Level - 1;
  طالما (l && at_وروستی_ثبت(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  إذا (++لار_2[l].دننه_واټن_2 == لار_2[l].اندازه_2)
    أرجع;
  غوټه_Ref NR = subtree(l);

  لكل (++l; l != Level; ++l) {
    لار_2[l] = ثبت(NR, 0);
    NR = NR.subtree(0);
  }
  لار_2[l] = ثبت(NR, 0);
}


IdxPair distribute(بینښه Nodes, بینښه Elements, بینښه ظرفیت,
                   ثابت بینښه *Cur_اندازه, بینښه New_اندازه[],
                   بینښه Position, منطقي Grow) {
  أكد(Elements + Grow <= Nodes * ظرفیت && "Not enough room for elements");
  أكد(Position <= Elements && "Invalid position");
  إذا (!Nodes)
    أرجع IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  ثابت بینښه فی_غوټه = (Elements + Grow) / Nodes;
  ثابت بینښه Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  بینښه مجموعه = 0;
  لكل (بینښه n = 0; n != Nodes; ++n) {
    مجموعه += New_اندازه[n] = فی_غوټه + (n < Extra);
    إذا (PosPair.لومړی_2 == Nodes && مجموعه > Position)
      PosPair = IdxPair(n, Position - (مجموعه - New_اندازه[n]));
  }
  أكد(مجموعه == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  إذا (Grow) {
    أكد(PosPair.لومړی_2 < Nodes && "Bad algebra");
    أكد(New_اندازه[PosPair.لومړی_2] && "Too few elements to need Grow");
    --New_اندازه[PosPair.لومړی_2];
  }

#که_نه_تعریف_شوی NDEBUG
  مجموعه = 0;
  لكل (بینښه n = 0; n != Nodes; ++n) {
    أكد(New_اندازه[n] <= ظرفیت && "Overallocated node");
    مجموعه += New_اندازه[n];
  }
  أكد(مجموعه == Elements && "Bad distribution sum");
#د_که_پای

  أرجع PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

