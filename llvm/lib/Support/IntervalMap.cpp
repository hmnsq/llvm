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

#شامل "llvm/ADT/IntervalMap.h"
#شامل <cassert>

نامخلا llvm {
نامخلا IntervalMapImpl {

خالی راستہ::replace_جڑ(خالی *جڑ_2, غیرعلامتی سائز, IdxPair Offsets) {
  أكد(!راستہ_2.خالی_خانے() && "Can't replace missing root");
  راستہ_2.front() = اندراج_2(جڑ_2, سائز, Offsets.پہلا_2);
  راستہ_2.داخل_کرو(راستہ_2.begin() + 1, اندراج_2(subtree(0), Offsets.دوسرا_2));
}

گرہ_Ref راستہ::get_بایاں_Sibling(غیرعلامتی Level) ثابت {
  // The root has no siblings.
  اگر (Level == 0)
    واپس گرہ_Ref();

  // Go up the tree until we can go left.
  غیرعلامتی l = Level - 1;
  جب (l && راستہ_2[l].انحراف_2 == 0)
    --l;

  // We can't go left.
  اگر (راستہ_2[l].انحراف_2 == 0)
    واپس گرہ_Ref();

  // NR is the subtree containing our left sibling.
  گرہ_Ref NR = راستہ_2[l].subtree(راستہ_2[l].انحراف_2 - 1);

  // Keep right all the way down.
  برائے (++l; l != Level; ++l)
    NR = NR.subtree(NR.سائز_2() - 1);
  واپس NR;
}

خالی راستہ::move_بایاں(غیرعلامتی Level) {
  أكد(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go left.
  غیرعلامتی l = 0;
  اگر (valid()) {
    l = Level - 1;
    جب (راستہ_2[l].انحراف_2 == 0) {
      أكد(l != 0 && "Cannot move beyond begin()");
      --l;
    }
  } ورنہ اگر (اونچائی() < Level)
    // end() may have created a height=0 path.
    راستہ_2.resize(Level + 1, اندراج_2(خالی_اشاریہ, 0, 0));

  // NR is the subtree containing our left sibling.
  --راستہ_2[l].انحراف_2;
  گرہ_Ref NR = subtree(l);

  // Get the rightmost node in the subtree.
  برائے (++l; l != Level; ++l) {
    راستہ_2[l] = اندراج_2(NR, NR.سائز_2() - 1);
    NR = NR.subtree(NR.سائز_2() - 1);
  }
  راستہ_2[l] = اندراج_2(NR, NR.سائز_2() - 1);
}

گرہ_Ref راستہ::get_دایاں_Sibling(غیرعلامتی Level) ثابت {
  // The root has no siblings.
  اگر (Level == 0)
    واپس گرہ_Ref();

  // Go up the tree until we can go right.
  غیرعلامتی l = Level - 1;
  جب (l && at_آخری_اندراج(l))
    --l;

  // We can't go right.
  اگر (at_آخری_اندراج(l))
    واپس گرہ_Ref();

  // NR is the subtree containing our right sibling.
  گرہ_Ref NR = راستہ_2[l].subtree(راستہ_2[l].انحراف_2 + 1);

  // Keep left all the way down.
  برائے (++l; l != Level; ++l)
    NR = NR.subtree(0);
  واپس NR;
}

خالی راستہ::move_دایاں(غیرعلامتی Level) {
  أكد(Level != 0 && "Cannot move the root node");

  // Go up the tree until we can go right.
  غیرعلامتی l = Level - 1;
  جب (l && at_آخری_اندراج(l))
    --l;

  // NR is the subtree containing our right sibling. If we hit end(), we have
  // offset(0) == node(0).size().
  اگر (++راستہ_2[l].انحراف_2 == راستہ_2[l].سائز_2)
    واپس;
  گرہ_Ref NR = subtree(l);

  برائے (++l; l != Level; ++l) {
    راستہ_2[l] = اندراج_2(NR, 0);
    NR = NR.subtree(0);
  }
  راستہ_2[l] = اندراج_2(NR, 0);
}


IdxPair distribute(غیرعلامتی Nodes, غیرعلامتی Elements, غیرعلامتی استعداد,
                   ثابت غیرعلامتی *Cur_سائز, غیرعلامتی New_سائز[],
                   غیرعلامتی Position, منطقی Grow) {
  أكد(Elements + Grow <= Nodes * استعداد && "Not enough room for elements");
  أكد(Position <= Elements && "Invalid position");
  اگر (!Nodes)
    واپس IdxPair();

  // Trivial algorithm: left-leaning even distribution.
  ثابت غیرعلامتی فی_گرہ = (Elements + Grow) / Nodes;
  ثابت غیرعلامتی Extra = (Elements + Grow) % Nodes;
  IdxPair PosPair = IdxPair(Nodes, 0);
  غیرعلامتی مجموع = 0;
  برائے (غیرعلامتی n = 0; n != Nodes; ++n) {
    مجموع += New_سائز[n] = فی_گرہ + (n < Extra);
    اگر (PosPair.پہلا_2 == Nodes && مجموع > Position)
      PosPair = IdxPair(n, Position - (مجموع - New_سائز[n]));
  }
  أكد(مجموع == Elements + Grow && "Bad distribution sum");

  // Subtract the Grow element that was added.
  اگر (Grow) {
    أكد(PosPair.پہلا_2 < Nodes && "Bad algebra");
    أكد(New_سائز[PosPair.پہلا_2] && "Too few elements to need Grow");
    --New_سائز[PosPair.پہلا_2];
  }

#اگر_غیر_تعریف_شدہ NDEBUG
  مجموع = 0;
  برائے (غیرعلامتی n = 0; n != Nodes; ++n) {
    أكد(New_سائز[n] <= استعداد && "Overallocated node");
    مجموع += New_سائز[n];
  }
  أكد(مجموع == Elements && "Bad distribution sum");
#اگر_ختم

  واپس PosPair;
}

} // namespace IntervalMapImpl
} // namespace llvm

